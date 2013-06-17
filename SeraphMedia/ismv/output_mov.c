/*******************************************************************************
 output_mov.c - A library for writing MPEG4.

 Copyright (C) 2009 CodeShop B.V.
 http://www.code-shop.com

 For licensing see the LICENSE file
******************************************************************************/ 

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef __cplusplus
#define __STDC_FORMAT_MACROS // C++ should define this for PRIu64
#define __STDC_LIMIT_MACROS  // C++ should define this for UINT64_MAX
#endif

#include "output_mov.h"
#include "mp4_io.h"
#include "mp4_writer.h"
#include "moov.h"
#include "output_bucket.h"
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>  // FreeBSD doesn't define off_t in stdio.h

// stts = [entries * [sample_count, sample_duration]
static void stts_create(mp4_context_t* mp4_context,
                        stts_t* stts,
                        samples_t const* first, samples_t const* last)
{
  unsigned int entries = 0;
  unsigned int samples = last - first;

  while(first != last)
  {
    unsigned int sample_count = 1;
    unsigned int sample_duration =
      (unsigned int)(first[1].pts_ - first[0].pts_);
    while(++first != last)
    {
      if((first[1].pts_ - first[0].pts_) != sample_duration)
        break;
      ++sample_count;
    }

    if(entries + 1 > stts->entries_)
    {
      stts->table_ = (stts_table_t*)
        realloc(stts->table_, (entries + 1) * sizeof(stts_table_t));
    }

    stts->table_[entries].sample_count_ = sample_count;
    stts->table_[entries].sample_duration_ = sample_duration;
    ++entries;
  }
  stts->entries_ = entries;

  if(stts_get_samples(stts) != samples)
  {
    MP4_WARNING("ERROR: stts_get_samples=%d, should be %d\n",
           stts_get_samples(stts), samples);
  }
}

static void stsz_create(mp4_context_t* mp4_context,
                        stsz_t* stsz,
                        samples_t const* first, samples_t const* last)
{
  unsigned int i = 0;

  stsz->sample_size_ = 0;
  stsz->entries_ = last - first;
  stsz->sample_sizes_ = (uint32_t*)malloc(stsz->entries_ * sizeof(uint32_t));

  while(first != last)
  {
    stsz->sample_sizes_[i] = first->size_;
    ++i;
    ++first;
  }
}

static stss_t* stss_create(mp4_context_t* mp4_context,
                           samples_t const* first, samples_t const* last)
{
  samples_t const* f = first;
  unsigned int sync_samples = 0;
  unsigned int s = 1;
  while(f != last)
  {
    sync_samples += f->is_ss_;
    ++f;
  }

  // if every sample is a random access point, the sync sample box is not
  // present
  if(sync_samples == last - first)
  {
    return 0;
  }
  else
  {
    stss_t* stss = stss_init();
    stss->entries_ = sync_samples;
    stss->sample_numbers_ = (uint32_t*)
      malloc(stss->entries_ * sizeof(uint32_t));

    f = first;
    sync_samples = 0;
    while(f != last)
    {
      if(f->is_ss_)
      {
        stss->sample_numbers_[sync_samples] = s;
        ++sync_samples;
      }
      ++s;
      ++f;
    }
    return stss;
  }
}

static ctts_t* ctts_create(mp4_context_t* mp4_context,
                           samples_t const* first, samples_t const* last)
{
  samples_t const* f = first;
  unsigned int i = 0;
  while(f != last)
  {
    if(f->cto_)
      break;
    ++f;
  }

  if(f == last)
  {
    return 0;
  }
  else
  {
    ctts_t* ctts = ctts_init();
    ctts->entries_ = last - first;
    ctts->table_ = (ctts_table_t*)
      malloc(ctts->entries_ * sizeof(ctts_table_t));

    f = first;
    while(f != last)
    {
      ctts->table_[i].sample_count_ = 1;
      ctts->table_[i].sample_offset_ = f->cto_;
      ++i;
      ++f;
    }

    return ctts;
  }
}

struct interleaved_movie_t
{
  uint64_t filepos;
  uint64_t mdat_size;
};
typedef struct interleaved_movie_t interleaved_movie_t;

struct interleaved_track_t
{
  trak_t const* src_trak;
  samples_t const* first;
  samples_t const* last;
  uint64_t start_pts;

  trak_t* dst_trak;
};
typedef struct interleaved_track_t interleaved_track_t;

static void stco_add_chunk(stco_t* stco, uint64_t offset)
{
  stco->chunk_offsets_ = (uint64_t*)
    realloc(stco->chunk_offsets_, sizeof(uint64_t) * (stco->entries_ + 1));
  stco->chunk_offsets_[stco->entries_] = offset;
  stco->entries_ += 1;
}

static void stsc_add_chunk(stsc_t* stsc, unsigned int first_chunk,
                           unsigned int samples_per_chunk,
                           unsigned int sample_description_index)
{
  // compress
  if(stsc->entries_)
  {
    stsc_table_t const* prev = &stsc->table_[stsc->entries_ - 1];
    if(prev->samples_ == samples_per_chunk &&
       prev->id_ == sample_description_index)
    {
      return;
    }
  }

  stsc->table_ = (stsc_table_t*)
    realloc(stsc->table_, sizeof(stsc_table_t) * (stsc->entries_ + 1));
  stsc->table_[stsc->entries_].chunk_ = first_chunk;
  stsc->table_[stsc->entries_].samples_ = samples_per_chunk;
  stsc->table_[stsc->entries_].id_ = sample_description_index;
  stsc->entries_ += 1;
}

static void stco_shift_offsets_inplace(unsigned char* stco, int offset)
{
  unsigned int entries = read_32(stco + 4);
  unsigned int* table = (unsigned int*)(stco + 8);
  unsigned int i;
  for(i = 0; i != entries; ++i)
    write_32((unsigned char*)&table[i], (read_32((unsigned char*)&table[i]) + offset));
}

void chunk_write(mp4_context_t* mp4_context,
                 bucket_t** buckets, interleaved_movie_t* imovie,
                 interleaved_track_t* itrack,
                 uint64_t end_pts)
{
  long trak_time_scale = itrack->src_trak->mdia_->mdhd_->timescale_;
//  uint64_t pts = itrack->first->pts_;

  // new chunk
  unsigned int first_chunk =
    itrack->dst_trak->mdia_->minf_->stbl_->stco_->entries_;
  unsigned int samples_per_chunk = 0;
  unsigned int sample_description_index = 1;
  bucket_t* bucket_prev = 0;

  MP4_INFO("New %c%c%c%c chunk pts=%"PRIu64"\n", 
    itrack->src_trak->mdia_->hdlr_->handler_type_ >> 24,
    itrack->src_trak->mdia_->hdlr_->handler_type_ >> 16,
    itrack->src_trak->mdia_->hdlr_->handler_type_ >>  8,
    itrack->src_trak->mdia_->hdlr_->handler_type_ >>  0,
    trak_time_to_moov_time(itrack->first->pts_ - itrack->start_pts, 1000,
      trak_time_scale));

  stco_add_chunk(itrack->dst_trak->mdia_->minf_->stbl_->stco_,
    imovie->mdat_size);

  while(itrack->first != itrack->last)
  {
    uint64_t pts =
      trak_time_to_moov_time(itrack->first->pts_ - itrack->start_pts,
      1000, trak_time_scale);

    if(pts > end_pts)
      break;

    {
      uint64_t sample_pos = itrack->first->pos_;
      uint64_t sample_size = itrack->first->size_;

      uint64_t sample_pos_prev = bucket_prev ?
        (bucket_prev->offset_ + bucket_prev->size_) : 0;

      // write sample in chunk
      if(bucket_prev &&
         sample_pos == bucket_prev->offset_ + bucket_prev->size_)
      {
        bucket_prev->size_ += sample_size;
      }
      else
      {
        bucket_prev = bucket_init_file(sample_pos, sample_size);
        bucket_insert_tail(buckets, bucket_prev);
      }
      imovie->mdat_size += itrack->first->size_;

      MP4_INFO(
        "pts=%"PRIi64" src:offset=%"PRIu64" src:size=%u seek=%"PRId64"\n",
        pts,
        itrack->first->pos_, itrack->first->size_,
        itrack->first->pos_ - sample_pos_prev);
    }

    ++itrack->first;
    ++samples_per_chunk;
  }
  stsc_add_chunk(itrack->dst_trak->mdia_->minf_->stbl_->stsc_,
    first_chunk, samples_per_chunk, sample_description_index);
}

extern int output_mov(struct mp4_context_t* mp4_context,
                      unsigned int const* trak_sample_start,
                      unsigned int const* trak_sample_end,
                      struct bucket_t** buckets,
                      struct mp4_split_options_t* options)
{
  unsigned int i;

  // ftyp
  {
    static char const free_data[] = {
      0x0, 0x0, 0x0,  42, 'f', 'r', 'e', 'e',
      'v', 'i', 'd', 'e', 'o', ' ', 's', 'e',
      'r', 'v', 'e', 'd', ' ', 'b', 'y', ' ',
      'm', 'o', 'd', '_', 'h', '2', '6', '4',
      '_', 's', 't', 'r', 'e', 'a', 'm', 'i',
      'n', 'g'
    };
    uint32_t size_of_header = (uint32_t)mp4_context->ftyp_atom.size_ +
                              sizeof(free_data);
    unsigned char* buffer = (unsigned char*)malloc(size_of_header);

    if(mp4_context->ftyp_atom.size_)
    {
      fseeko(mp4_context->infile, mp4_context->ftyp_atom.start_, SEEK_SET);
      if(fread(buffer, (off_t)mp4_context->ftyp_atom.size_, 1, mp4_context->infile) != 1)
      {
        MP4_ERROR("%s", "Error reading ftyp atom\n");
        free(buffer);
        return 0;
      }
    }

    // copy free data
    memcpy(buffer + mp4_context->ftyp_atom.size_, free_data, sizeof(free_data));
    bucket_insert_tail(buckets, bucket_init_memory(buffer, size_of_header));
    free(buffer);
  }

  // moov
  {
    moov_t* moov = moov_init();
    uint64_t start_pts = UINT64_MAX;

    bucket_t* moov_bucket = bucket_init(BUCKET_TYPE_MEMORY);
    bucket_insert_tail(buckets, moov_bucket);

    moov->mvhd_ = mvhd_copy(mp4_context->moov->mvhd_);
    moov->mvhd_->duration_ = 0;
    for(i = 0; i != mp4_context->moov->tracks_; ++i)
    {
      trak_t const* src_trak = mp4_context->moov->traks_[i];
      trak_t* trak = trak_init();
      trak->tkhd_ = tkhd_copy(src_trak->tkhd_);
      trak->mdia_ = mdia_init();
      trak->mdia_->mdhd_ = mdhd_copy(src_trak->mdia_->mdhd_);
      trak->mdia_->mdhd_->duration_ = 0;
      trak->mdia_->hdlr_ = hdlr_copy(src_trak->mdia_->hdlr_);
      trak->mdia_->minf_ = minf_init();
      trak->mdia_->minf_->smhd_ = src_trak->mdia_->minf_->smhd_ == NULL ?
        NULL : smhd_copy(src_trak->mdia_->minf_->smhd_);
      trak->mdia_->minf_->vmhd_ = src_trak->mdia_->minf_->vmhd_ == NULL ?
        NULL : vmhd_copy(src_trak->mdia_->minf_->vmhd_);
      trak->mdia_->minf_->dinf_ = dinf_copy(src_trak->mdia_->minf_->dinf_);
      trak->mdia_->minf_->stbl_ = stbl_init();
      trak->mdia_->minf_->stbl_->stts_ = stts_init();
      trak->mdia_->minf_->stbl_->ctts_ = ctts_init();
      trak->mdia_->minf_->stbl_->stsz_ = stsz_init();
      trak->mdia_->minf_->stbl_->stsc_ = stsc_init();
      trak->mdia_->minf_->stbl_->stco_ = stco_init();
      trak->mdia_->minf_->stbl_->stsd_ = stsd_copy(src_trak->mdia_->minf_->stbl_->stsd_);

      {
        unsigned int start_sample = trak_sample_start[i];
        unsigned int end_sample = trak_sample_end[i];
        samples_t const* first = &src_trak->samples_[start_sample];
        samples_t const* last = &src_trak->samples_[end_sample];

        uint64_t pts = trak_time_to_moov_time(first->pts_,
            moov->mvhd_->timescale_, src_trak->mdia_->mdhd_->timescale_);
        if(start_pts > pts)
        {
           start_pts = pts;
        }

        // update trak duration
        trak->mdia_->mdhd_->duration_ = last->pts_ - first->pts_;
        trak->tkhd_->duration_ =
          trak_time_to_moov_time(trak->mdia_->mdhd_->duration_,
            moov->mvhd_->timescale_, src_trak->mdia_->mdhd_->timescale_);

        // update movie duration
        if(trak->tkhd_->duration_ > moov->mvhd_->duration_)
        {
          moov->mvhd_->duration_ = trak->tkhd_->duration_ ;
        }

        // create stts
        stts_create(mp4_context, trak->mdia_->minf_->stbl_->stts_, first, last);

        // create stsz
        stsz_create(mp4_context, trak->mdia_->minf_->stbl_->stsz_, first, last);

        // create stss
		if(trak->mdia_->minf_->stbl_->stss_)
			stss_exit(trak->mdia_->minf_->stbl_->stss_);
        trak->mdia_->minf_->stbl_->stss_ = stss_create(mp4_context, first, last);

        // create ctts
		if(trak->mdia_->minf_->stbl_->ctts_)
			ctts_exit(trak->mdia_->minf_->stbl_->ctts_);
        trak->mdia_->minf_->stbl_->ctts_ = ctts_create(mp4_context, first, last);
      }

      moov->traks_[moov->tracks_] = trak;
      ++moov->tracks_;
    }

    // TODO: create edts for tracks when the pts_offset != 0 to
    // synchronize tracks
    for(i = 0; i != mp4_context->moov->tracks_; ++i)
    {
      trak_t const* src_trak = mp4_context->moov->traks_[i];
      unsigned int start_sample = trak_sample_start[i];
  //    unsigned int end_sample = trak_sample_end[i];
      samples_t const* first = &src_trak->samples_[start_sample];
  //    samples_t const* last = &src_trak->samples_[end_sample];
      uint64_t pts = trak_time_to_moov_time(first->pts_,
          moov->mvhd_->timescale_, src_trak->mdia_->mdhd_->timescale_);

      MP4_INFO("pts_offset=%"PRIu64"\n", 
        trak_time_to_moov_time(pts - start_pts, 1000, moov->mvhd_->timescale_));
    }

    // mdat
    {
      struct mp4_atom_t mdat_atom;
      bucket_t* mdat_bucket;
      {
        mdat_atom.type_ = FOURCC('m', 'd', 'a', 't');
        mdat_atom.short_size_ = 0;
        mdat_atom.size_ = ATOM_PREAMBLE_SIZE;

        {
          unsigned char buffer[32];
          int mdat_header_size = mp4_atom_write_header(buffer, &mdat_atom);
          mdat_bucket = bucket_init_memory(buffer, mdat_header_size);
          bucket_insert_tail(buckets, mdat_bucket);
        }
      }

      // interleave tracks
      {
        interleaved_movie_t imovie;
        interleaved_track_t* itracks = (interleaved_track_t*)
          malloc(mp4_context->moov->tracks_ * sizeof(interleaved_track_t));
        imovie.filepos = 0;
        imovie.mdat_size = ATOM_PREAMBLE_SIZE;

        for(i = 0; i != mp4_context->moov->tracks_; ++i)
        {
          trak_t* src_trak = mp4_context->moov->traks_[i];
          unsigned int start_sample = trak_sample_start[i];
          unsigned int end_sample = trak_sample_end[i];
          itracks[i].src_trak = src_trak;
          itracks[i].first = &src_trak->samples_[start_sample];
          itracks[i].last = &src_trak->samples_[end_sample];
          itracks[i].start_pts = itracks[i].first->pts_;
          itracks[i].dst_trak = moov->traks_[i];
        }

        {
          uint64_t pts = 0;
          for(;;)
          {
            unsigned int tracks = 0;
            for(i = 0; i != mp4_context->moov->tracks_; ++i)
            {
              interleaved_track_t* itrack = &itracks[i];
              if(itrack->first == itrack->last)
              {
                ++tracks;
                continue;
              }

              chunk_write(mp4_context, buckets, &imovie, itrack, pts + 500);
            }
            if(tracks == mp4_context->moov->tracks_)
            {
              break;
            }
            pts += 500;
          }
        }
        write_32((unsigned char*)mdat_bucket->buf_, imovie.mdat_size);
        free(itracks);
      }

      // moov
      {
        moov_bucket->buf_ = malloc(32 * 1024 * 1024);
        moov_bucket->size_ = moov_write(moov, (unsigned char*)moov_bucket->buf_);

        {
          bucket_t* bucket = *buckets;
          uint64_t offset = 0;
          do
          {
            offset += bucket->size_;
            bucket = bucket->next_;
          } while(bucket != mdat_bucket);

          for(i = 0; i != moov->tracks_; ++i)
          {
            trak_t* trak = moov->traks_[i];
            stco_shift_offsets_inplace(
              (unsigned char*)trak->mdia_->minf_->stbl_->stco_->stco_inplace_,
              offset);
          }
        }
        moov_exit(moov);
      }
    }
  }

  return 1;
}

// End Of File

