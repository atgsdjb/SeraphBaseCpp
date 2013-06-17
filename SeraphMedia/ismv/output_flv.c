/*******************************************************************************
 output_flv.c - A library for writing FLV.

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

#include "output_flv.h"
#include "mp4_io.h"
#include "moov.h"
#include "output_bucket.h"
// #include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RTMP_AAC_SEQUENCE_HEADER  0
#define RTMP_AAC_RAW              1

#define RTMP_AVC_SEQUENCE_HEADER  0
#define RTMP_AVC_NALU             1

static void write_avc_sequence_header(sample_entry_t const* sample_entry,
                                      bucket_t** buckets)
{
  unsigned char* buffer = (unsigned char*)
    malloc(1 + 1 + 3 + sample_entry->codec_private_data_length_);
  unsigned char* p = buffer;

  p = write_8(p, 0x17);
  p = write_8(p, RTMP_AVC_SEQUENCE_HEADER);
  p = write_24(p, 0);
  memcpy(p, sample_entry->codec_private_data_,
         sample_entry->codec_private_data_length_);
  p += sample_entry->codec_private_data_length_;
  bucket_insert_tail(buckets, bucket_init_memory(buffer, p - buffer));
  free(buffer);
}

static void write_aac_sequence_header(sample_entry_t const* sample_entry,
                                      bucket_t** buckets)
{
  unsigned char* buffer = (unsigned char*)
    malloc(1 + 1 + sample_entry->codec_private_data_length_);
  unsigned char* p = buffer;

  p = write_8(p, 0xaf);
  p = write_8(p, RTMP_AAC_SEQUENCE_HEADER);

  memcpy(p, sample_entry->codec_private_data_,
         sample_entry->codec_private_data_length_);
  p += sample_entry->codec_private_data_length_;
  bucket_insert_tail(buckets, bucket_init_memory(buffer, p - buffer));
  free(buffer);
}

static unsigned int count_keyframes(trak_t const* trak)
{
  // count the number of sync samples (nr of keyframes)
  unsigned int nr_of_keyframes = 0;
  unsigned int start;
  for(start = 0; start != trak->samples_size_; ++start)
  {
    if(trak->samples_[start].is_ss_)
    {
      ++nr_of_keyframes;
    }
  }

  return nr_of_keyframes;
}

enum
{
  amf0_number_marker       = 0x00,
  amf0_boolean_marker      = 0x01,
  amf0_string_marker       = 0x02,
  amf0_object_marker       = 0x03,
  amf0_movieclip_marker    = 0x04,
  amf0_null_marker         = 0x05,
  amf0_undefined_marker    = 0x06,
  amf0_reference_marker    = 0x07,
  amf0_ecma_array_marker   = 0x08,
  amf0_object_end_marker   = 0x09,
  amf0_strict_array_marker = 0x0a,
  amf0_date_marker         = 0x0b,
  amf0_long_string_marker  = 0x0c,
  amf0_unsupported_marker  = 0x0d,
  amf0_recordset_marker    = 0x0e,
  amf0_xml_document_marker = 0x0f,
  amf0_typed_object_marker = 0x10
};

static unsigned char* write_string(unsigned char* buffer, const char* rhs)
{
  size_t size = strlen(rhs);
  if(size == 0)
  {
    buffer = write_8(buffer, amf0_null_marker);
  } else
  if(size < 65536)
  {
    buffer = write_8(buffer, amf0_string_marker);
    buffer = write_16(buffer, size);
  } else
  {
    buffer = write_8(buffer, amf0_long_string_marker);
    buffer = write_32(buffer, size);
  }

  memcpy(buffer, rhs, size);
  buffer += size;

  return buffer;
}

static unsigned char* write_string_no_marker(unsigned char* buffer,
                                             const char* rhs)
{
  size_t size = strlen(rhs);
  buffer = write_16(buffer, size);
  memcpy(buffer, rhs, size);
  buffer += size;

  return buffer;
}

static unsigned char* write_double(unsigned char* buffer, double rhs)
{
  union
  {
    uint64_t integer_;
    double double_;
  } val;

  buffer = write_8(buffer, amf0_number_marker);
  val.double_ = rhs;
  buffer = write_64(buffer, val.integer_);

  return buffer;
}

static unsigned char* write_bool(unsigned char* buffer, int rhs)
{
  buffer = write_8(buffer, amf0_boolean_marker);
  buffer = write_8(buffer, rhs == 0 ? 0 : 1);

  return buffer;
}

static bucket_t* write_metadata(bucket_t* bucket,
                                moov_t const* moov,
                                trak_t const* audio_track,
                                trak_t const* video_track,
                                unsigned int nr_of_keyframes,
                                uint64_t const* filepositions,
                                uint64_t const* times,
                                bucket_t** buckets)
{
  unsigned char* buffer;
  unsigned char* p;
  if(bucket == NULL)
  {
    buffer = (unsigned char*)malloc(8192 + nr_of_keyframes * 2 * 9);
  }
  else
  {
    buffer = (unsigned char*)bucket->buf_;
  }
  p = buffer;

  p = write_string(p, "onMetaData");
  p = write_8(p, amf0_object_marker);

  p = write_string_no_marker(p, "duration");
  p = write_double(p, moov->mvhd_->duration_ / (float)moov->mvhd_->timescale_);

  p = write_string_no_marker(p, "metadatacreator");
  p = write_string(p, "CodeShop's Streaming Utilities");

  p = write_string_no_marker(p, "hasVideo");
  p = write_bool(p, video_track != NULL ? 1 : 0);

  p = write_string_no_marker(p, "hasAudio");
  p = write_bool(p, audio_track != NULL ? 1 : 0);

  if(audio_track != NULL)
  {
    // TODO: "audiosamplerate"
    p = write_string_no_marker(p, "audiodatarate");
    p = write_double(p, (double)trak_bitrate(audio_track) / 1000);

    p = write_string_no_marker(p, "audiocodecid");
    p = write_double(p, (double)10);
  }

  if(video_track != NULL)
  {
    unsigned int i;

    samples_t const* last_sample =
      &video_track->samples_[video_track->samples_size_ - 1];

    double video_duration = (double)trak_time_to_moov_time(
      last_sample->pts_, 1000, video_track->mdia_->mdhd_->timescale_) / 1000;

    p = write_string_no_marker(p, "width");
    p = write_double(p, video_track->tkhd_->width_ / 65536);

    p = write_string_no_marker(p, "height");
    p = write_double(p, video_track->tkhd_->height_ / 65536);

    p = write_string_no_marker(p, "videodatarate");
    p = write_double(p, (double)trak_bitrate(video_track) / 1000);

    p = write_string_no_marker(p, "videocodecid");
    p = write_double(p, (double)7);

    p = write_string_no_marker(p, "framerate");
    p = write_double(p, video_track->samples_size_ / video_duration);

    p = write_string_no_marker(p, "hasKeyframes");
    p = write_bool(p, 1);

    p = write_string_no_marker(p, "keyframes");
    p = write_8(p, amf0_object_marker);

    p = write_string_no_marker(p, "filepositions");
    p = write_8(p, amf0_strict_array_marker);
    p = write_32(p, nr_of_keyframes);
    for(i = 0; i != nr_of_keyframes; ++i)
    {
      p = write_double(p, (double)filepositions[i]);
    }
    p = write_string_no_marker(p, "times");
    p = write_8(p, amf0_strict_array_marker);
    p = write_32(p, nr_of_keyframes);
    for(i = 0; i != nr_of_keyframes; ++i)
    {
      p = write_double(p, (double)times[i] / 1000.0);
    }
    p = write_24(p, 0x000009);
  }

  p = write_24(p, 0x000009);

  if(bucket == NULL)
  {
    bucket = bucket_init_memory(buffer, p - buffer);
    bucket_insert_tail(buckets, bucket);
    free(buffer);
  }

  return bucket;
}

static bucket_t* flv_tag_start(unsigned char tag_type, unsigned int timestamp,
                               bucket_t** buckets)
{
  unsigned char flv_tag[11];
  bucket_t* bucket;

  write_8(flv_tag + 0, tag_type);
//write_24(flv_tag + 1, sizeof(AUDIODATA || VIDEODATA))
  write_24(flv_tag + 4, timestamp);
  write_8(flv_tag + 7, timestamp >> 24);
  write_24(flv_tag + 8, 0);

  bucket = bucket_init_memory(flv_tag, 11);
  bucket_insert_tail(buckets, bucket);

  return bucket;
}

static uint64_t flv_tag_end(bucket_t* tag_bucket, bucket_t** buckets)
{
  unsigned char previous_tag_size[4];

  // get size of audio/video/script data
  uint64_t data_size = 0;
  bucket_t* bucket = tag_bucket->next_;
  while(*buckets != bucket)
  {
    data_size += bucket->size_;
    bucket = bucket->next_;
  }
  write_24((unsigned char*)(tag_bucket->buf_) + 1, (unsigned int)data_size);

  // write previous tag size
  write_32(previous_tag_size, (unsigned int)(data_size + 11));
  bucket_insert_tail(buckets, bucket_init_memory(previous_tag_size, 4));

  return 11 + data_size + 4;
}

extern int output_flv(struct mp4_context_t const* mp4_context,
                      unsigned int* trak_sample_start,
                      unsigned int* trak_sample_end,
                      struct bucket_t** buckets,
                      struct mp4_split_options_t* options)
{
  moov_t* moov = mp4_context->moov;
  unsigned int track = 0;
  uint64_t fileposition = 0;

  int audio_track_index = -1;
  int video_track_index = -1;
  trak_t const* audio_track = 0;
  trak_t const* video_track = 0;
  int has_audio;
  int has_video;

  unsigned int sample_position[MAX_TRACKS];

  unsigned int nr_of_keyframes = 0;
  uint64_t* filepositions = 0;
  uint64_t* times = 0;

  for(track = 0; track != moov->tracks_; ++track)
  {
    trak_t const* trak = moov->traks_[track];
    sample_position[track] = trak_sample_start[track];
    if(video_track_index == -1 &&
       trak->mdia_->hdlr_->handler_type_ == FOURCC('v', 'i', 'd', 'e'))
    {
      video_track_index = track;
      video_track = trak;
    } else
    if(audio_track_index == -1 &&
       trak->mdia_->hdlr_->handler_type_ == FOURCC('s', 'o', 'u', 'n'))
    {
      audio_track_index = track;
      audio_track = trak;
    }
  }

  has_audio = audio_track_index == -1 ? 0 : 1;
  has_video = video_track_index == -1 ? 0 : 1;

  // The FLV header
  {
    unsigned char flags = (has_audio << 2) + (has_video << 0);
    char flv_header[9] = { 'F', 'L', 'V', 0x01, flags, 0x00, 0x00, 0x00, 0x09 };
    bucket_insert_tail(buckets,
      bucket_init_memory(flv_header, sizeof(flv_header)));
    fileposition += 9;

    // The FLV file body
    {
      unsigned char previous_tag_size[4];
      write_32(previous_tag_size, sizeof(flv_header));
      bucket_insert_tail(buckets, bucket_init_memory(previous_tag_size, 4));
      fileposition += 4;
    }
  }

  if(has_video)
  {
    // count the number of sync samples (nr of keyframes)
    nr_of_keyframes = count_keyframes(moov->traks_[video_track_index]);
    filepositions = (uint64_t*)calloc(nr_of_keyframes, sizeof(uint64_t));
    times = (uint64_t*)calloc(nr_of_keyframes, sizeof(uint64_t));
  }

  // write onMetaData
  {
    bucket_t* metadata_bucket = 0;

    int partial_request = trak_sample_start[0] != 0;

    if(!partial_request)
    {
      bucket_t* flv_script_bucket = flv_tag_start(18, 0, buckets);
      metadata_bucket =
        write_metadata(NULL, moov, audio_track, video_track,
                       nr_of_keyframes, filepositions, times,
                       buckets);
      fileposition += flv_tag_end(flv_script_bucket, buckets);
    }

    if(has_audio)
    {
      trak_t const* trak = moov->traks_[audio_track_index];
      stsd_t const* stsd = trak->mdia_->minf_->stbl_->stsd_;
      sample_entry_t const* sample_entry = &stsd->sample_entries_[0];
      bucket_t* tag_bucket = flv_tag_start(8, 0, buckets);
      write_aac_sequence_header(sample_entry, buckets);
      fileposition += flv_tag_end(tag_bucket, buckets);
    }
    if(has_video)
    {
      trak_t const* trak = moov->traks_[video_track_index];
      stsd_t const* stsd = trak->mdia_->minf_->stbl_->stsd_;
      sample_entry_t const* sample_entry = &stsd->sample_entries_[0];
      bucket_t* tag_bucket = flv_tag_start(9, 0, buckets);
      write_avc_sequence_header(sample_entry, buckets);
      fileposition += flv_tag_end(tag_bucket, buckets);
    }

    {
      unsigned int keyframe = 0;
      uint64_t begin_pts = (uint64_t)(-1);
      while(audio_track_index != -1 || video_track_index != -1)
      {
        int next_trak = -1;
        uint64_t pts = 0;
        for(track = 0; track != moov->tracks_; ++track)
        {
          if(track == audio_track_index || track == video_track_index)
          {
            unsigned int s = sample_position[track];
            trak_t const* trak = moov->traks_[track];
            if(s == trak_sample_end[track])
            {
              continue;
            }

            // FLV uses a fixed 1000 timescale
            {
              uint64_t track_pts = trak_time_to_moov_time(
                trak->samples_[s].pts_, 1000, trak->mdia_->mdhd_->timescale_);

              // get track with earliest pts
              if(next_trak == -1 || track_pts < pts)
              {
                next_trak = track;
                pts = track_pts;
              }
            }
          }
        }

        if(begin_pts == (uint64_t)(-1))
        {
          begin_pts = pts;
        }

        {
          trak_t const* trak = moov->traks_[next_trak];
          unsigned int s = sample_position[next_trak];

          // FLV tag
          bucket_t* tag_bucket =
            flv_tag_start(next_trak == audio_track_index ? 8 : 9,
              (unsigned int)(pts - begin_pts), buckets);

          uint64_t sample_pos = trak->samples_[s].pos_;
          unsigned int sample_size = trak->samples_[s].size_;

          if(next_trak == audio_track_index)
          {
            // AUDIODATA
            unsigned char header[2];
            write_8(header, 0xaf);
            write_8(header + 1, RTMP_AAC_RAW);
            // AACAUDIODATA
            bucket_insert_tail(buckets, bucket_init_memory(header, 2));
            bucket_insert_tail(buckets, bucket_init_file(sample_pos, sample_size));
          } else
          if(next_trak == video_track_index)
          {
            int cto = trak->samples_[s].cto_;

            // FLV uses a fixed 1000 timescale
            unsigned int composition_time = (unsigned int)
              (trak_time_to_moov_time(cto, 1000, trak->mdia_->mdhd_->timescale_));

            // VIDEODATA
            unsigned char header[5];
            unsigned int is_keyframe = trak->samples_[s].is_ss_;
            unsigned int codec_id = 7;          // AVC
            write_8(header, ((is_keyframe ? 1 : 2) << 4) + codec_id);

            write_8(header + 1, RTMP_AVC_NALU);
            write_24(header + 2, composition_time);
            bucket_insert_tail(buckets, bucket_init_memory(header, 5));
            bucket_insert_tail(buckets, bucket_init_file(sample_pos, sample_size));

            if(is_keyframe)
            {
              times[keyframe] = pts;
              filepositions[keyframe] = fileposition;
              ++keyframe;
            }
          }
          fileposition += flv_tag_end(tag_bucket, buckets);
        }

        // advance track by one sample
        ++sample_position[next_trak];

        if(sample_position[next_trak] == trak_sample_end[next_trak])
        {
          if(next_trak == audio_track_index)
          {
            audio_track_index = -1;
          }
          if(next_trak == video_track_index)
          {
            video_track_index = -1;
          }
        }
      }
    }

    if(metadata_bucket)
    {
      write_metadata(metadata_bucket, moov, audio_track, video_track,
                     nr_of_keyframes, filepositions, times,
                     buckets);
    }
  }

  if(filepositions)
  {
    free(filepositions);
  }
  if(times)
  {
    free(times);
  }

  return 1;
}

// End Of File

