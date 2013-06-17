/*******************************************************************************
 output_ismc.c - A library for writing Smooth Streaming Client Manifests.

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

#include "output_ismc.h"
#include "mp4_io.h"
#include "mp4_reader.h"
#include "output_bucket.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#else
//wu add for iphone
#define bswap32(x)   ((((x) & 0x000000FF) << 24) | \
(((x) & 0x0000FF00) << 8 ) | \
(((x) & 0x00FF0000) >> 8 ) | \
(((x) & 0xFF000000) >> 24))

#define bswap16(x)   ((((x) & 0x00FF) << 8) | \
(((x) & 0xFF00) >> 8))
//wu end
# if defined(__FreeBSD__) || defined(__NetBSD__)
#  include <endian.h>
#  define bswap_16(x) bswap16(x)
#  define bswap_32(x) bswap32(x)
# elif defined(__OpenBSD__)
#  include <endian.h>
#  define bswap_16(x) swap16(x)
#  define bswap_32(x) swap32(x)
# else
#  include <byteswap.h>
# endif
#endif 

static const uint32_t aac_samplerates[] =
{
  96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050,
  16000, 12000, 11025,  8000,  7350,     0,     0,     0
};

static const uint32_t aac_channels[] =
{
  0, 1, 2, 3, 4, 5, 6, 8,
  0, 0, 0, 0, 0, 0, 0, 0
};

static char const version_data[] =
{
  0x0, 0x0, 0x0,  44, 'f', 'r', 'e', 'e',
  'v', 'i', 'd', 'e', 'o', ' ', 's', 'e',
  'r', 'v', 'e', 'd', ' ', 'b', 'y', ' ',
  'm', 'o', 'd', '_', 's', 'm', 'o', 'o',
  't', 'h', '_', 's', 't', 'r', 'e', 'a',
  'm', 'i', 'n', 'g'
};

static uint16_t byteswap16(uint16_t val)
{
#ifdef WIN32
  return _byteswap_ushort(val);
#else
  return bswap_16(val);
#endif
}

static uint32_t byteswap32(uint32_t val)
{
#ifdef WIN32
  return _byteswap_ulong(val);
#else
  return bswap_32(val);
#endif
}

////////////////////////////////////////////////////////////////////////////////
// Manifest

static char* hex64(unsigned char const* first, unsigned char const* last, char* out)
{
  static const char* hex = "0123456789ABCDEF";
  while(first != last)
  {
    int a = (*first >> 4) & 15;
    int b = (*first >> 0) & 15;
    *out++ = hex[a];
    *out++ = hex[b];
    ++first;
  }
  *out = '\0';

  return out;
}

struct quality_level_t
{
  uint32_t bitrate_;
  uint32_t fourcc_;
  uint32_t width_;
  uint32_t height_;
  char codec_private_data_[256];
};
typedef struct quality_level_t quality_level_t;

static quality_level_t* quality_level_init()
{
  quality_level_t* that = (quality_level_t*)malloc(sizeof(quality_level_t));

  that->width_ = 0;
  that->height_ = 0;
  that->codec_private_data_[0] = '\0';

  return that;
}

static quality_level_t* quality_level_copy(quality_level_t* rhs)
{
  quality_level_t* that = (quality_level_t*) malloc(sizeof(quality_level_t));

  that->bitrate_ = rhs->bitrate_;
  that->fourcc_ = rhs->fourcc_;
  that->width_ = rhs->width_;
  that->height_ = rhs->height_;
  memcpy(that->codec_private_data_, rhs->codec_private_data_,
    sizeof(that->codec_private_data_) / sizeof(that->codec_private_data_[0]));

  return that;
}

static void quality_level_exit(quality_level_t* that)
{
  free(that);
}

static char*
quality_level_write(quality_level_t const* that, char* buffer)
{
  char* p = buffer;

  p += sprintf(p, "<QualityLevel"
                  " Bitrate=\"%u\""
                  " FourCC=\"%c%c%c%c\"",
               that->bitrate_,
               (that->fourcc_ >> 24) & 0xff,
               (that->fourcc_ >> 16) & 0xff,
               (that->fourcc_ >> 8) & 0xff,
               that->fourcc_ & 0xff);

  if(that->width_ && that->height_)
  {
    p += sprintf(p, " Width=\"%u\""
                    " Height=\"%u\""
                    " CodecPrivateData=\"%s\"",
                 that->width_,
                 that->height_,
                 that->codec_private_data_);

  }
  else
  {
    p += sprintf(p, " WaveFormatEx=\"%s\"",
                 that->codec_private_data_);
  }
  p += sprintf(p, " />\n");

  return p;
}

#define MAX_QUALITY_LEVELS 8  // TODO: enforce
#define MAX_STREAMS 4         // TODO: enforce

struct stream_t
{
  // handler_type: 'soun' or 'vide'
  uint32_t type_;

  // H264/WmaPro/
  char subtype_[32];

  // vod: number of chunks, live: 0
  uint32_t chunks_;

  // 0=vod, 1=live
  int is_live_;

  // QualityLevels({bitrate})/Fragments(audio={start_time}
  char url_[256];

  // number of streams
  size_t quality_levels_;

  quality_level_t* quality_level_[MAX_QUALITY_LEVELS];

  // the starting pts (for live)
  uint64_t begin_pts_;

  // each chunk's duration
  uint64_t* durations_;
};
typedef struct stream_t stream_t;

static stream_t* stream_init(uint32_t type, uint32_t chunks, int is_live)
{
  stream_t* that = (stream_t*)malloc(sizeof(stream_t));

  that->type_ = type;
  that->subtype_[0] = '\0';
  that->chunks_ = chunks;
  that->is_live_ = is_live;
  that->url_[0] = '\0';
  that->quality_levels_ = 0;
  that->begin_pts_ = 0;
  that->durations_ = (uint64_t*)malloc(chunks * sizeof(uint64_t));

  return that;
}

static stream_t* stream_copy(stream_t* rhs)
{
  stream_t* that = (stream_t*)malloc(sizeof(stream_t));
  size_t i;

  that->type_ = rhs->type_;
  strcpy(that->subtype_, rhs->subtype_);
  that->chunks_ = rhs->chunks_;
  that->is_live_ = rhs->is_live_;
  strcpy(that->url_, rhs->url_);
  that->quality_levels_ = rhs->quality_levels_;
  for(i = 0; i != rhs->quality_levels_; ++i)
  {
    that->quality_level_[i] = quality_level_copy(rhs->quality_level_[i]);
  }
  that->begin_pts_ = rhs->begin_pts_;
  that->durations_ = (uint64_t*)malloc(that->chunks_ * sizeof(uint64_t));
  memcpy(that->durations_, rhs->durations_, that->chunks_ * sizeof(uint64_t));

  return that;
}

static void stream_exit(struct stream_t* that)
{
  quality_level_t** first = that->quality_level_;
  quality_level_t** last = that->quality_level_ + that->quality_levels_;
  while(first != last)
  {
    quality_level_exit(*first);
    ++first;
  }

  free(that->durations_);
  free(that);
}

static void stream_add_quality_level(struct stream_t* that,
                                     quality_level_t* child)
{
  that->quality_level_[that->quality_levels_] = child;
  ++that->quality_levels_;
}

struct smooth_streaming_media_t
{
  uint64_t duration_;
  size_t streams_;
  struct stream_t* stream_[MAX_STREAMS]; // audio and video
};
typedef struct smooth_streaming_media_t smooth_streaming_media_t;

static struct smooth_streaming_media_t* smooth_streaming_media_init()
{
  smooth_streaming_media_t* that = (smooth_streaming_media_t*)
    malloc(sizeof(smooth_streaming_media_t));

  that->duration_ = 0;
  that->streams_ = 0;

  return that;
}

static void smooth_streaming_media_exit(smooth_streaming_media_t* that)
{
  stream_t** first = that->stream_;
  stream_t** last = that->stream_ + that->streams_;
  while(first != last)
  {
    stream_exit(*first);
    ++first;
  }
  free(that);
}

static stream_t* smooth_streaming_media_find_stream(
  smooth_streaming_media_t* that, uint32_t type)
{
  stream_t** first = that->stream_;
  stream_t** last = that->stream_ + that->streams_;
  while(first != last)
  {
    if((*first)->type_ == type)
    {
      return *first;
    }
    ++first;
  }

  return NULL;
}

static stream_t* smooth_streaming_media_new_stream(
  smooth_streaming_media_t* that, uint32_t type, uint32_t chunks, int is_live)
{
  stream_t* stream = smooth_streaming_media_find_stream(that, type);
  if(!stream)
  {
    stream = stream_init(type, chunks, is_live);
    that->stream_[that->streams_] = stream;
    ++that->streams_;
  }

  return stream;
}

static char* stream_write(stream_t const* stream, char* buffer)
{
  char* p = buffer;

  char const* type = NULL;

  switch(stream->type_)
  {
  case FOURCC('s', 'o', 'u', 'n'):
    type = "audio";
    break;
  case FOURCC('v', 'i', 'd', 'e'):
    type = "video";
    break;
  default:
    break;
  }

  p += sprintf(p, "<StreamIndex"
                  " Type=\"%s\""
                  " Subtype=\"%s\""
                  " Chunks=\"%u\""
                  " Url=\"%sFragments(%s={start time})\">\n",
               type, stream->subtype_,
               stream->is_live_ ? 0 : stream->chunks_,
               stream->url_, type);

  {
    quality_level_t* const* first = stream->quality_level_;
    quality_level_t* const* last = stream->quality_level_ + stream->quality_levels_;
    while(first != last)
    {
      p = quality_level_write(*first, p);
      ++first;
    }
  }

  {
    uint64_t const* first = stream->durations_;
    uint64_t const* last = stream->durations_ + stream->chunks_;
    if(stream->is_live_)
    {
      uint64_t t = stream->begin_pts_;
      while(first != last)
      {
        p += sprintf(p, "<c"
                        " t=\"%"PRIu64"\""
                        " d=\"%"PRIu64"\""
                        " />\n",
                     t, *first);
        t += *first;
        ++first;
      }
    }
    else
    {
      uint32_t chunk = 0;
      while(first != last)
      {
        p += sprintf(p, "<c"
                        " n=\"%u\""
                        " d=\"%"PRIu64"\""
                        " />\n",
                     chunk, *first);
        ++chunk;
        ++first;
      }
    }
  }

  p += sprintf(p, "</StreamIndex>\n");

  return p;
}

static char*
smooth_streaming_media_write(smooth_streaming_media_t* that, char* buffer)
{
  int is_live = that->duration_ == 0 ? 1 : 0;
  char* p = buffer;
  stream_t** first = that->stream_;
  stream_t** last = that->stream_ + that->streams_;

  p += sprintf(p, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
  p += sprintf(p, "<!--Created with mod_smooth_streaming(%s)-->\n",
               X_MOD_SMOOTH_STREAMING_VERSION);

  // write SmoothStreamingMedia
  {
    p += sprintf(p, "<SmoothStreamingMedia"
                    " MajorVersion=\"1\""
                    " MinorVersion=\"0\""
                    " Duration=\"%"PRIu64"\"",
                 that->duration_);

    if(is_live)
    {
      p += sprintf(p, " LookAheadFragmentCount=\"2\""
                      " IsLive=\"TRUE\"");
    }

    p += sprintf(p, ">\n");
  }

  while(first != last)
  {
    p = stream_write(*first, p);
    ++first;
  }

  p += sprintf(p, "</SmoothStreamingMedia>\n");

  return p;
}

static smooth_streaming_media_t*
create_manifest(mp4_context_t const* mp4_context, moov_t* moov)
{
  smooth_streaming_media_t* smooth_streaming_media = NULL;
  unsigned int track;
  int is_live = 0;  // moov->mvhd_->duration_ == 0 ? 1 : 0;

  if(!moov_build_index(mp4_context, moov))
  {
    return smooth_streaming_media;
  }

  smooth_streaming_media = smooth_streaming_media_init();

  for(track = 0; track != moov->tracks_; ++track)
  {
    trak_t* trak = moov->traks_[track];
    unsigned int bitrate = ((trak_bitrate(trak) + 999) / 1000) * 1000;
    uint32_t chunks = 0;
    stream_t* stream;

    // count the number of smooth streaming chunks
    {
      samples_t* first = trak->samples_;
      samples_t* last = trak->samples_ + trak->samples_size_;
      uint64_t trak_duration;
      while(first != last)
      {
        if(first->is_smooth_ss_)
          ++chunks;
        ++first;
      }
      trak_duration = trak_time_to_moov_time(first->pts_,
          10000000, trak->mdia_->mdhd_->timescale_);
      if(trak_duration > smooth_streaming_media->duration_)
      {
        smooth_streaming_media->duration_ = trak_duration;
      }
    }

    stream = smooth_streaming_media_new_stream(smooth_streaming_media,
      trak->mdia_->hdlr_->handler_type_, chunks, is_live);

    {
      strcpy(stream->url_, "QualityLevels({bitrate})/");
    }

    // write StreamIndex
    {
      stsd_t const* stsd = trak->mdia_->minf_->stbl_->stsd_;
      sample_entry_t const* sample_entry = &stsd->sample_entries_[0];
      int is_avc = 0;

      switch(trak->mdia_->hdlr_->handler_type_)
      {
      case FOURCC('v', 'i', 'd', 'e'):
        is_avc = sample_entry->fourcc_ == FOURCC('a', 'v', 'c', '1');
        // H264 or WVC1
        sprintf(stream->subtype_, "%s", is_avc ? "H264" : "WVC1");
        break;
      case FOURCC('s', 'o', 'u', 'n'):
        // WmaPro or 
        if(sample_entry->fourcc_ == FOURCC('o', 'w', 'm', 'a'))
          sprintf(stream->subtype_, "WmaPro");
        else
          sprintf(stream->subtype_, "%c%c%c%c",
                  (sample_entry->fourcc_ >> 24),
                  (sample_entry->fourcc_ >> 16),
                  (sample_entry->fourcc_ >>  8),
                  (sample_entry->fourcc_ >>  0));
        break;
      default:
        break;
      }

      // write QualityLevel
      {
        quality_level_t* quality_level = quality_level_init();
        stream_add_quality_level(stream, quality_level);
        quality_level->fourcc_ = sample_entry->fourcc_;
        quality_level->bitrate_ = bitrate;
        switch(trak->mdia_->hdlr_->handler_type_)
        {
        case FOURCC('v', 'i', 'd', 'e'):
          quality_level->width_ = trak->tkhd_->width_ / 65536;
          quality_level->height_ = trak->tkhd_->height_ / 65536;

          quality_level->fourcc_ = is_avc ? FOURCC('H', '2', '6', '4')
                                          : FOURCC('W', 'V', 'C', '1');

          if(!sample_entry->codec_private_data_length_)
          {
            MP4_WARNING("%s", "[Warning]: No codec private data found\n");
          }

          if(is_avc)
          {
            static const unsigned char nal[4] = { 0, 0, 0, 1 };
            char* out = quality_level->codec_private_data_;

            out = hex64(nal, nal + 4, out);
            out = hex64(sample_entry->sps_,
                        sample_entry->sps_ + sample_entry->sps_length_,
                        out);
            out = hex64(nal, nal + 4, out);
            out = hex64(sample_entry->pps_,
                        sample_entry->pps_ + sample_entry->pps_length_,
                        out);
          }
          else
          {
            hex64(sample_entry->codec_private_data_,
                  sample_entry->codec_private_data_ +
                  sample_entry->codec_private_data_length_,
                  quality_level->codec_private_data_);
          }
          break;
        case FOURCC('s', 'o', 'u', 'n'):
        {
          char* out = &quality_level->codec_private_data_[0];

          // owma already includes the WAVEFORMATEX structure
          if(sample_entry->fourcc_ != FOURCC('o', 'w', 'm', 'a'))
          {
            // create WAVEFORMATEX
            //
            // WAVE_FORMAT_MPEG_ADTS_AAC (0x1600)
            // Advanced Audio Coding (AAC) audio in Audio Data Transport Stream
            // (ADTS) format.
            // No additional data is requried after the WAVEFORMATEX structure.
            //
            // WAVE_FORMAT_RAW_AAC1 (0x00ff)
            // Raw AAC.
            // The WAVEFORMATEX structure is followed by additional bytes that
            // contain the AudioSpecificConfig() data. The length of the
            // AudioSpecificConfig() data is 2 bytes for AAC-LC or HE-AAC with
            // implicit signaling of SBR/PS. It is more than 2 bytes for HE-AAC
            // with explicit signaling of SBR/PS.

  //          uint16_t wFormatTag = 0xa106; // WAVE_FORMAT_AAC
  //          uint16_t wFormatTag = 0x1600; // WAVE_FORMAT_MPEG_ADTS_AAC
  //          uint16_t wFormatTag = 0x00ff; // WAVE_FORMAT_RAW_AAC1
  //          uint16_t wFormatTag = 0x0055; // mp3
            uint16_t wFormatTag = sample_entry->wFormatTag;
            uint16_t nChannels = sample_entry->nChannels;
            uint32_t nSamplesPerSec = sample_entry->nSamplesPerSec;
            uint32_t nAvgBytesPerSec = sample_entry->nAvgBytesPerSec;
            uint16_t nBlockAlign = sample_entry->nBlockAlign != 0 ?
                                   sample_entry->nBlockAlign : 1;
            uint16_t wBitsPerSample = sample_entry->wBitsPerSample;
            uint16_t cbSize = (uint16_t)sample_entry->codec_private_data_length_;
            unsigned char waveformatex[18];
            unsigned char* wfx = waveformatex;

            if(cbSize >= 2)
            {
              // object_type
              // 0. Null
              // 1. AAC Main
              // 2. AAC LC
              // 3. AAC SSR
              // 4. AAC LTP
              // 6. AAC Scalable

              unsigned int object_type =
                sample_entry->codec_private_data_[0] >> 3;
              unsigned int frequency_index =
                ((sample_entry->codec_private_data_[0] & 7) << 1) |
                 (sample_entry->codec_private_data_[1] >> 7);
              unsigned int channels =
                (sample_entry->codec_private_data_[1] >> 3) & 15;

              MP4_INFO("AAC object_type/profile=%u frequency=%u channels=%u\n",
                      object_type - 1, aac_samplerates[frequency_index],
                      aac_channels[channels]);

              if(channels != nChannels)
              {
                MP4_WARNING("[Warning] settings channels in WAVEFORMATEX to %u\n",
                       channels);
                nChannels = (uint16_t)channels;
              }
            }

            wfx = write_16(wfx, byteswap16(wFormatTag));
            wfx = write_16(wfx, byteswap16(nChannels));
            wfx = write_32(wfx, byteswap32(nSamplesPerSec));
            wfx = write_32(wfx, byteswap32(nAvgBytesPerSec));
            wfx = write_16(wfx, byteswap16(nBlockAlign));
            wfx = write_16(wfx, byteswap16(wBitsPerSample));
            wfx = write_16(wfx, byteswap16(cbSize));
            out = hex64(waveformatex, wfx, out);

            MP4_INFO("%s", "WAVEFORMATEX:\n");
            MP4_INFO("  wFormatTag=0x%04x\n", wFormatTag);
            MP4_INFO("  wChannels=%u\n", nChannels);
            MP4_INFO("  nSamplesPerSec=%u\n", nSamplesPerSec);
            MP4_INFO("  nAvgBytesPerSec=%u\n", nAvgBytesPerSec);
            MP4_INFO("  nBlockAlign=%u\n", nBlockAlign);
            MP4_INFO("  wBitsPerSample=%u\n", wBitsPerSample);
            MP4_INFO("  cbSize=%u\n", cbSize);
          }

          if(!sample_entry->codec_private_data_length_)
          {
            MP4_WARNING("%s", "[Warning]: No codec private data found\n");
          }

          out = hex64(sample_entry->codec_private_data_,
                      sample_entry->codec_private_data_ +
                      sample_entry->codec_private_data_length_,
                      out);
        }
          break;
        default:
          break;
        }
      }

      // the chunks
      if(trak->samples_)
      {
        samples_t* first = trak->samples_;
        samples_t* last = trak->samples_ + trak->samples_size_ + 1;
        unsigned int chunk = 0;
        uint64_t begin_pts = (uint64_t)-1;
        while(first != last)
        {
          uint64_t first_pts;
          while(first != last)
          {
            if(first->is_smooth_ss_)
              break;
            ++first;
          }

          // SmoothStreaming uses a fixed 10000000 timescale
          first_pts = trak_time_to_moov_time(first->pts_,
            10000000, trak->mdia_->mdhd_->timescale_);

          if(begin_pts != (uint64_t)(-1))
          {
            stream->durations_[chunk] = first_pts - begin_pts;
            ++chunk;
          }
          else
          {
            stream->begin_pts_ = first_pts;
          }
          begin_pts = first_pts;
          if(first == last)
            break;
          ++first;
        }
      }
    }
  }

  return smooth_streaming_media;
}

static void manifest_merge(struct mp4_context_t* mp4_context,
                           struct smooth_streaming_media_t* manifest,
                           struct smooth_streaming_media_t* smooth_streaming_media)
{
  struct stream_t** first = smooth_streaming_media->stream_;
  struct stream_t** last =
    smooth_streaming_media->stream_ + smooth_streaming_media->streams_;
  while(first != last)
  {
    // merge with stream of same type or add new stream
    struct stream_t* stream =
      smooth_streaming_media_find_stream(manifest, (*first)->type_);
    if(!stream)
    {
      stream = stream_copy(*first);
      manifest->stream_[manifest->streams_] = stream;
      ++manifest->streams_;
    }
    else
    {
      size_t i;
      size_t j;
      if(stream->chunks_ != (*first)->chunks_)
      {
        MP4_ERROR("Incompatible number of chunks (%u) in %s\n", (*first)->chunks_, remove_path(mp4_context->filename_));
        return;
      }
      // add quality levels
      for(i = 0; i != (*first)->quality_levels_; ++i)
      {
        unsigned int bitrate = (*first)->quality_level_[i]->bitrate_;
        // skip quality levels with the same bitrate
        for(j = 0; j != stream->quality_levels_; ++j)
        {
          if(bitrate == stream->quality_level_[j]->bitrate_)
          {
            MP4_INFO("Stream with duplicate bitrate skipped (%u)\n", bitrate);
            bitrate = 0;
            break;
          }
        }
        if(bitrate)
        {
          stream_add_quality_level(stream,
            quality_level_copy((*first)->quality_level_[i]));
        }
      }
    }
    ++first;
  }
}

extern int mp4_create_manifest_client(struct mp4_context_t** mp4_context,
                                      unsigned int mp4_contexts,
                                      struct bucket_t** buckets)
{
  unsigned int file;
  struct smooth_streaming_media_t* manifest = NULL;
  int result = 1;
  for(file = 0; file != mp4_contexts; ++file)
  {
    mp4_context_t* context = mp4_context[file];
    moov_t* moov = context->moov;
    smooth_streaming_media_t* smooth_streaming_media =
      create_manifest(context, moov);

    if(smooth_streaming_media == NULL)
    {
      result = 0;
    }

    if(manifest == 0)
    {
      manifest = smooth_streaming_media;
    }
    else
    {
      manifest_merge(context, manifest, smooth_streaming_media),
      smooth_streaming_media_exit(smooth_streaming_media);
    }
  }

  if(manifest)
  {
    char* buffer = (char*)malloc(1024 * 256);
    char* p = smooth_streaming_media_write(manifest, buffer);
    bucket_insert_tail(buckets, bucket_init_memory(buffer, p - buffer));
    free(buffer);

    smooth_streaming_media_exit(manifest);
  }

  return result;
}

// End Of File

