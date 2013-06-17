/*******************************************************************************
 output_ismv.c - A library for reading and writing Fragmented MPEG4.

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

#include "output_ismv.h"
#include "mp4_io.h"
#include "mp4_reader.h"
#include "ism_reader.h"
#include "moov.h"
#include "output_bucket.h"
#include <stdlib.h>
#include <string.h>

static char const version_data[] =
{
  0x0, 0x0, 0x0,  44, 'f', 'r', 'e', 'e',
  'v', 'i', 'd', 'e', 'o', ' ', 's', 'e',
  'r', 'v', 'e', 'd', ' ', 'b', 'y', ' ',
  'm', 'o', 'd', '_', 's', 'm', 'o', 'o',
  't', 'h', '_', 's', 't', 'r', 'e', 'a',
  'm', 'i', 'n', 'g'
};

extern int ism_lookup_track(char* filename, struct mp4_split_options_t* options)
{
  ism_t* ism = ism_init(filename);

  if(ism == NULL)
  {
    return 0;
  }

  {
    char* dir_end = strrchr(filename, '/');
    unsigned int fragment_bitrate;
    const char* fragment_type;
    const char* src;

    if(options->audio_fragment_start != UINT64_MAX)
    {
      fragment_type = fragment_type_audio;
      fragment_bitrate = options->audio_fragment_bitrate;
    }
    else
    {
      fragment_type = fragment_type_video;
      fragment_bitrate = options->video_fragment_bitrate;
    }

    if(!ism_get_source(ism, fragment_bitrate,
      fragment_type, &src, &options->fragment_track_id))
    {
      ism_exit(ism);
      return 0;
    }

    dir_end = dir_end == NULL ? filename : (dir_end + 1);
    strcpy(dir_end, src);

    ism_exit(ism);
  }

  return 1;
}

static int mfra_get_track_fragment(struct mfra_t const* mfra,
                                   struct mp4_context_t const* mp4_context,
                                   struct bucket_t** buckets,
                                   char const* fragment_type,
                                   uint64_t fragment_start,
                                   struct mp4_split_options_t const* options)
{
  uint64_t moof_offset = 0;
  uint64_t moof_size = 0;

  tfra_t const* tfra = mfra_get_track(mfra, options->fragment_track_id);

  if(tfra == NULL)
  {
    MP4_ERROR("Requested %s track (with id=%u) not found in mfra atom\n",
              fragment_type, options->fragment_track_id);
    return 0;
  }

  {
    struct tfra_table_t* table = tfra->table_;
    unsigned int i;
    for(i = 0; i != tfra->number_of_entry_; ++i)
    {
      if(fragment_start == table[i].time_)
      {
        struct mp4_atom_t fragment_moof_atom;
        struct mp4_atom_t fragment_mdat_atom;

        moof_offset = table[i].moof_offset_;

        // find the size of the MOOF and following MDAT atom
        fseeko(mp4_context->infile, moof_offset, SEEK_SET);
        if(!mp4_atom_read_header(mp4_context, mp4_context->infile, &fragment_moof_atom))
        {
          MP4_ERROR("%s", "Error reading MOOF atom\n");
          return 0;
        }
        fseeko(mp4_context->infile, fragment_moof_atom.end_, SEEK_SET);
        if(!mp4_atom_read_header(mp4_context, mp4_context->infile, &fragment_mdat_atom))
        {
          MP4_ERROR("%s", "Error reading MDAT atom\n");
          return 0;
        }

        moof_size = fragment_moof_atom.size_ + fragment_mdat_atom.size_;
        break;
      }
    }

    if(moof_size == 0)
    {
      MP4_ERROR("%s", "No matching MOOF atom found for fragment\n");
      return 0;
    }

    bucket_insert_tail(buckets, bucket_init_file(moof_offset, moof_size));

    bucket_insert_tail(buckets,
      bucket_init_memory(version_data, sizeof(version_data)));
  }

  return 1;
}

int output_ismv(struct mp4_context_t const* mp4_context,
                struct bucket_t** buckets,
                struct mp4_split_options_t const* options)
{
  int result = 0;

  if(mp4_context->mfra_data)
  {
    mfra_t* mfra = (mfra_t*)
      mfra_read(mp4_context, NULL,
                mp4_context->mfra_data + ATOM_PREAMBLE_SIZE,
                mp4_context->mfra_atom.size_ - ATOM_PREAMBLE_SIZE);

    if(mfra != NULL)
    {
      char const* fragment_type;
      uint64_t fragment_start;

      if(options->audio_fragment_start != UINT64_MAX)
      {
        fragment_type = fragment_type_audio;
        fragment_start = options->audio_fragment_start;
      }
      else
      {
        fragment_type = fragment_type_video;
        fragment_start = options->video_fragment_start;
      }

      result = mfra_get_track_fragment(mfra, mp4_context, buckets,
        fragment_type, fragment_start, options);
      mfra_exit(mfra);
    }
  }

  return result;
}

// End Of File

