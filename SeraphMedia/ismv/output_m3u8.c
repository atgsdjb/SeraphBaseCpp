/*******************************************************************************
 output_m3u8.c - A library for writing M3U8 playlists.

 Copyright (C) 2009 CodeShop B.V.
 http://www.code-shop.com

 For licensing see the LICENSE file
******************************************************************************/ 

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef __cplusplus
#define __STDC_FORMAT_MACROS // C++ should define this for PRIu64
#endif

#include "output_m3u8.h"
#include "mp4_io.h"
#include "mp4_reader.h"
#include "output_bucket.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#define snprintf _snprintf
#endif

static char* uri_escape(char const* first, char const* last, char* dst)
{
  // unreserved character as per IETF RFC3986 section 2.3
  static const char unreserved_chars[] =
    "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-._~";

  while(first != last)
  {
    if(strchr(unreserved_chars, *first))
    {
      *dst++ = *first;
    }
    else
    {
      *dst++ = '%';
      *dst++ = unreserved_chars[(*first >> 4) & 15];
      *dst++ = unreserved_chars[(*first >> 0) & 15];
    }
    ++first;
  }

  return dst;
}

static char const* path_leaf(char const* path)
{
  char const* p = strrchr(path, '/');
  if(p && *p != '\0')
  {
    return p + 1;
  }

  return path;
}

struct av_track_t
{
  unsigned int bitrate;
  trak_t const* trak;
};
typedef struct av_track_t av_track_t;

static int av_track_compare(void const* p1, void const* p2)
{
  av_track_t const* t1 = (av_track_t const*)p1;
  av_track_t const* t2 = (av_track_t const*)p2;

  return !(t1->bitrate < t2->bitrate);
}

static void write_stream_playlist(char const* filename,
                                  char const* basename,
                                  unsigned int audio_bitrate,
                                  trak_t const* audio_trak,
                                  unsigned int video_bitrate,
                                  trak_t const* video_trak)
{
  samples_t const* audio_first = audio_trak->samples_;
  samples_t const* audio_last = audio_trak->samples_ + audio_trak->samples_size_ + 1;
  samples_t const* video_first = video_trak->samples_;
  samples_t const* video_last = video_trak->samples_ + video_trak->samples_size_ + 1;

  unsigned int chunk = 0;
  uint64_t audio_pts = (uint64_t)-1;
  uint64_t video_pts = (uint64_t)-1;

  uint64_t iphone_pts;

  char* buffer = (char*)malloc(1024 * 256);
  char* p = buffer;

  // MUST start with EXTM3U tag
  p += sprintf(p, "#EXTM3U\n");

  // version info
  p += sprintf(p, "## Created with mod_smooth_streaming(%s)\n",
               X_MOD_SMOOTH_STREAMING_VERSION);

  while(video_first != video_last)
  {
    uint64_t first_pts;

    while(audio_first != audio_last)
    {
      if(audio_first->is_smooth_ss_)
        break;
      ++audio_first;
    }

    while(video_first != video_last)
    {
      if(video_first->is_smooth_ss_)
        break;
      ++video_first;
    }

    // SmoothStreaming uses a fixed 10000000 timescale
    first_pts = trak_time_to_moov_time(video_first->pts_,
      10000000, video_trak->mdia_->mdhd_->timescale_);

    if(video_pts != (uint64_t)(-1))
    {
      char const* leaf;
      unsigned int duration = (unsigned int)
        ((first_pts - iphone_pts + 5000000) / 10000000);

      // fix for Snow-Leopard Quicktime player. Increment duration of last
      // chunk so we won't have a rouding error < 0 at the end of the
      // playout
      if(video_first == video_last - 1)
      {
        ++duration;
      }

      if(chunk == 0)
      {
        // Indicate the approximate duration of the next media file that will
        // be added to the main presentation.
        p += sprintf(p, "#EXT-X-TARGETDURATION:%u\n", duration);

        // Indicate the unique sequence number of the first URI that appears
        // in the Playlist file.
        p += sprintf(p, "#EXT-X-MEDIA-SEQUENCE:0\n");
      }

      // Record marker that describes the following media file
      p += sprintf(p, "#EXTINF:%u, %s\n", duration, "no desc");

      // URI
      leaf = path_leaf(basename);
      p = uri_escape(leaf, leaf + strlen(leaf), p);
      p += sprintf(p, ".ism?format=ts");
      p += sprintf(p, "&video=%"PRIu64"&bitrate=%u",
                   video_pts, video_bitrate);
      if(audio_first != audio_last)
      {
        p += sprintf(p, "&audio=%"PRIu64"&bitrate=%u",
                     audio_pts, audio_bitrate);
      }
      p += sprintf(p, "\n");

      ++chunk;

      iphone_pts += (uint64_t)(duration) * 10000000;
    }
    else
    {
      iphone_pts = (first_pts + 5000000) / 10000000;
    }
    audio_pts = trak_time_to_moov_time(audio_first->pts_,
      10000000, audio_trak->mdia_->mdhd_->timescale_);
    video_pts = first_pts;

    if(video_first == video_last)
      break;
    ++video_first;

    if(audio_first != audio_last)
    {
      ++audio_first;
    }
  }

//  printf("m3u8: duration=%u\n", (unsigned int)(iphone_pts / 10000000));

  p += sprintf(p, "#EXT-X-ENDLIST\n");

  {
    size_t filesize = p - buffer;
    mem_range_t* mem_range = mem_range_init_write(filename, 0, filesize);
    memcpy(mem_range_map(mem_range, 0, filesize), buffer, filesize);
    mem_range_exit(mem_range);
  }

  free(buffer);
}

extern int mp4_create_m3u8(char const* basename,
                           struct mp4_context_t** mp4_context,
                           unsigned int mp4_contexts,
                           struct bucket_t** buckets)
{
  unsigned int file;
  int result = 1;

  for(file = 0; file != mp4_contexts; ++file)
  {
    mp4_context_t* context = mp4_context[file];

    if(!moov_build_index(context, context->moov))
    {
      return 0;
    }
  }

  {
    av_track_t audio_tracks[MAX_TRACKS];
    av_track_t video_tracks[MAX_TRACKS];
    unsigned int audio_track = 0;
    unsigned int video_track = 0;
    unsigned int track;

    // collect all audio/video tracks
    for(file = 0; file != mp4_contexts; ++file)
    {
      mp4_context_t* context = mp4_context[file];
      moov_t const* moov = context->moov;
      for(track = 0; track != moov->tracks_; ++track)
      {
        trak_t const* trak = moov->traks_[track];
        unsigned int bitrate = ((trak_bitrate(trak) + 999) / 1000) * 1000;

        if(trak->mdia_->hdlr_->handler_type_ == FOURCC('v', 'i', 'd', 'e'))
        {
          video_tracks[video_track].bitrate = bitrate;
          video_tracks[video_track].trak = trak;
          ++video_track;
        } else
        if(trak->mdia_->hdlr_->handler_type_ == FOURCC('s', 'o', 'u', 'n'))
        {
          audio_tracks[audio_track].bitrate = bitrate;
          audio_tracks[audio_track].trak = trak;
          ++audio_track;
        }
      }
    }

    printf("found %u audio and %d video track(s)\n", audio_track, video_track);
    if(audio_track == 0 || video_track == 0)
    {
      printf("m3u8 playlist needs at least 1 audio and 1 video track\n");
      return 0;
    }

    // sort on bitrate
    qsort(&audio_tracks[0], audio_track, sizeof(av_track_t), av_track_compare);
    qsort(&video_tracks[0], video_track, sizeof(av_track_t), av_track_compare);

    {
      av_track_t* audio_first = audio_tracks;
      av_track_t* audio_last = audio_tracks + audio_track;
      av_track_t* video_first = video_tracks;
      av_track_t* video_last = video_tracks + video_track;

      char* buffer = (char*)malloc(1024 * 256);
      char* p = buffer;

      // MUST start with EXTM3U tag
      p += sprintf(p, "#EXTM3U\n");

      // version info
      p += sprintf(p, "## Created with mod_smooth_streaming(%s)\n",
                   X_MOD_SMOOTH_STREAMING_VERSION);

      // create playlist of playlists
      while(video_first != video_last)
      {
        unsigned int bitrate = video_first->bitrate + audio_first->bitrate;

        char filename[256];
        char const* leaf;

        p += sprintf(p, "#EXT-X-STREAM-INF:PROGRAM-ID=1,BANDWIDTH=%u\n",
          bitrate);

        snprintf(filename, sizeof(filename), "%s-%uk%s", basename,
          bitrate / 1000, ".m3u8");

        leaf = path_leaf(filename);
        p = uri_escape(leaf, leaf + strlen(leaf), p);
        p += sprintf(p, "\n");

        write_stream_playlist(filename, basename,
                              audio_first->bitrate, audio_first->trak,
                              video_first->bitrate, video_first->trak);

        if(audio_first + 1 != audio_last)
        {
          ++audio_first;
        }
        ++video_first;
      }

      bucket_insert_tail(buckets, bucket_init_memory(buffer, p - buffer));
      free(buffer);
    }
  }
  return result;
}

// End Of File

