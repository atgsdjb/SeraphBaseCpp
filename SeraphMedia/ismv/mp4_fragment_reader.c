/*******************************************************************************
 mp4_fragment_reader.c - A library for reading MFRA / RXS files.

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

#include "mp4_fragment_reader.h"
#include "mp4_io.h"
#include "mp4_reader.h"
#include "moov.h"
#include <stdlib.h>
#include <stdio.h>

#ifdef WIN32
#define snprintf _snprintf
#endif

struct mp4_fragment_reader_t
{
  // rxs
  mem_range_t* mem_range_;

  // mfra
  mp4_context_t* mp4_context_;
  mfra_t* mfra_;
  tfra_t* tfra_;
};
// typedef struct mp4_fragment_reader_t mp4_fragment_reader_t;

static int open_rxs(mp4_fragment_reader_t* mp4_fragment_reader,
                    char const* filename, unsigned int track_id)
{
  char rxs_filename[256];

  // create rxs filename
  snprintf(rxs_filename, sizeof(rxs_filename), "%s.%u.rxs", filename, track_id);

  mp4_fragment_reader->mem_range_ = mem_range_init_read(rxs_filename);
  if(!mp4_fragment_reader->mem_range_)
  {
    return 0;
  }

  return 1;
}

static int open_mfra(mp4_fragment_reader_t* mp4_fragment_reader,
                     char const* filename, unsigned int track_id)
{
  uint64_t filesize = get_filesize(filename);
  int verbose = 1;
  mp4_fragment_reader->mp4_context_ =
    mp4_open(filename, filesize, MP4_OPEN_MFRA, verbose);

  if(!mp4_fragment_reader->mp4_context_ ||
     !mp4_fragment_reader->mp4_context_->mfra_data)
  {
    return 0;
  }

  mp4_fragment_reader->mfra_ = (mfra_t*)
    mfra_read(mp4_fragment_reader->mp4_context_, NULL,
              mp4_fragment_reader->mp4_context_->mfra_data + ATOM_PREAMBLE_SIZE,
              mp4_fragment_reader->mp4_context_->mfra_atom.size_ - ATOM_PREAMBLE_SIZE);

  if(mp4_fragment_reader->mfra_ == NULL)
  {
    return 0;
  }

  {
    mfra_t const* mfra = mp4_fragment_reader->mfra_;
    mp4_context_t const* mp4_context = mp4_fragment_reader->mp4_context_;

    unsigned int mfra_track = 0;
    for(mfra_track = 0; mfra_track != mfra->tracks_; ++mfra_track)
    {
      tfra_t* tfra = mfra->tfras_[mfra_track];
      if(tfra->track_id_ == track_id)
        break;
    }
    if(mfra_track == mfra->tracks_)
    {
      MP4_ERROR("Requested track (with id=%u) not found in mfra atom\n",
        track_id);
      return 0;
    }

    mp4_fragment_reader->tfra_ = mfra->tfras_[mfra_track];
  }

  return 1;
}

int mp4_fragment_open(mp4_fragment_reader_t* mp4_fragment_reader,
                      char const* filename, unsigned int track_id)
{
  if(open_rxs(mp4_fragment_reader, filename, track_id))
  {
    return 1;
  }

  if(open_mfra(mp4_fragment_reader, filename, track_id))
  {
    return 1;
  }

  return 0;
}

mp4_fragment_reader_t* mp4_fragment_reader_init(char const* filename,
                                                unsigned int track_id)
{
  mp4_fragment_reader_t* mp4_fragment_reader = (mp4_fragment_reader_t*)
    malloc(sizeof(mp4_fragment_reader_t));

  mp4_fragment_reader->mem_range_ = 0;
  mp4_fragment_reader->mp4_context_ = 0;
  mp4_fragment_reader->mfra_ = 0;

  if(!mp4_fragment_open(mp4_fragment_reader, filename, track_id))
  {
    mp4_fragment_reader_exit(mp4_fragment_reader);
    return 0;
  }

  return mp4_fragment_reader;
}

void mp4_fragment_reader_exit(mp4_fragment_reader_t* mp4_fragment_reader)
{
  if(mp4_fragment_reader->mem_range_)
  {
    mem_range_exit(mp4_fragment_reader->mem_range_);
  }

  if(mp4_fragment_reader->mp4_context_)
  {
    mp4_close(mp4_fragment_reader->mp4_context_);
  }

  if(mp4_fragment_reader->mfra_)
  {
    mfra_exit(mp4_fragment_reader->mfra_);
  }

  free(mp4_fragment_reader);
}

int mp4_fragment_reader_get_offset(mp4_fragment_reader_t* mp4_fragment_reader,
                                   uint64_t pts,
                                   uint64_t* offset, uint64_t* size,
                                   unsigned int* index)
{
  mem_range_t* mem_range = mp4_fragment_reader->mem_range_;
  if(mem_range)
  {
    // rxs
    unsigned char const* first = (unsigned char const*)(
      mem_range_map(mem_range, 0, (uint32_t)mem_range->filesize_));
    unsigned char const* last = first + mem_range->filesize_;

    *index = 0;
    while(first != last)
    {
      uint64_t time = read_64(first);
      if(time == pts)
      {
        *offset = read_64(first + 8);
        *size = read_64(first + 16);
        return 1;
      }
      first += sizeof(rxs_t);
      ++(*index);
    }
  }
  else
  {
    // mfra
    tfra_t const* tfra = mp4_fragment_reader->tfra_;
    tfra_table_t const* table = tfra->table_;
    unsigned int i;
    for(i = 0; i != tfra->number_of_entry_; ++i)
    {
      if(pts == table[i].time_)
      {
        *offset = table[i].moof_offset_;
        *size = table[i + 1].moof_offset_ - *offset;
        *index = i;
        return 1;
      }
    }
  }

//  MP4_ERROR("%s", "No matching MOOF atom found for fragment\n");

  return 0;
}

int mp4_fragment_reader_get_index(mp4_fragment_reader_t* mp4_fragment_reader,
                                  unsigned int index,
                                  uint64_t* offset, uint64_t* size,
                                  uint64_t* fragment_start)
{
  mem_range_t* mem_range = mp4_fragment_reader->mem_range_;
  if(mem_range)
  {
    // rxs
    unsigned char const* first = (unsigned char const*)(
      mem_range_map(mem_range, 0, (uint32_t)mem_range->filesize_));
    unsigned char const* last = first + mem_range->filesize_;

    first += index * (3 * 8);

    if(first < last)
    {
      *fragment_start = read_64(first + 0);
      *offset = read_64(first + 8);
      *size = read_64(first + 16);
      return 1;
    }
  }
  else
  {
    // mfra
    tfra_t const* tfra = mp4_fragment_reader->tfra_;
    tfra_table_t const* table = tfra->table_;
    if(index < tfra->number_of_entry_)
    {
      *fragment_start = table[index].time_;
      *offset = table[index].moof_offset_;
      *size = table[index + 1].moof_offset_ - *offset;
      return 1;
    }
  }

  return 0;
}

// End Of File

