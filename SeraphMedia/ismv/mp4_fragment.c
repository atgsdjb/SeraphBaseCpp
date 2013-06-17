/*******************************************************************************
 mp4_fragment.c - A library for reading and writing Fragmented MPEG4.

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

#include "mp4_fragment.h"
#include "mp4_io.h"
#include "mp4_reader.h"
#include "mp4_writer.h"
#include "output_bucket.h"
#include <stdio.h>
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
static char woov_data[] =
{
	0x0, 0x0, 0x0,  20, 'f', 'r', 'e', 'e',
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0
};
struct woov_t{
	moov_t* moov;
	uint64_t mdat_size;
};

static void stco_add_chunk(stco_t* stco, uint64_t offset);

static void stsc_add_chunk(stsc_t* stsc, unsigned int first_chunk,
													 unsigned int samples_per_chunk,
													 unsigned int sample_description_index);

////////////////////////////////////////////////////////////////////////////////

static int moof_create(struct mp4_context_t const* mp4_context,
struct moov_t* fmoov,
struct woov_t* woov,
struct moof_t* moof,
struct mfra_t* mfra,
	uint64_t moof_offset,
	unsigned int seq,
struct bucket_t** buckets,
	int output_raw)
{
	uint32_t mdat_size = ATOM_PREAMBLE_SIZE;
	bucket_t* mdat_bucket = 0;
	unsigned int i = 0;
	uint64_t start_time = 0, end_time = 0;
	unsigned int start=0, end=0;
	if(!output_raw)
	{
		unsigned char mdat_buffer[32];
		mp4_atom_t mdat_atom;
		int mdat_header_size;
		mdat_atom.type_ = FOURCC('m', 'd', 'a', 't');
		mdat_atom.short_size_ = 0;
		mdat_header_size = mp4_atom_write_header(mdat_buffer, &mdat_atom);
		mdat_bucket = bucket_init_memory(mdat_buffer, mdat_header_size);
		bucket_insert_tail(buckets, mdat_bucket);
	}

	moof->mfhd_ = mfhd_init();
	moof->mfhd_->sequence_number_ = seq;

	for(i = 0;i < fmoov->tracks_; i++)
	{
		uint32_t trun_mdat_size=0;
		struct trak_t * trak = fmoov->traks_[i];
		struct stsd_t const* stsd = trak->mdia_->minf_->stbl_->stsd_;
		struct sample_entry_t const* sample_entry = &stsd->sample_entries_[0];
		//    int is_avc = sample_entry->fourcc_ == FOURCC('a', 'v', 'c', '1');

		struct traf_t* traf = traf_init();
		moof->trafs_[moof->tracks_] = traf;
		++moof->tracks_;

		start = trak->smoothes_[moof->mfhd_->sequence_number_-1].start;
		end = trak->smoothes_[moof->mfhd_->sequence_number_].start;


		{
			//      struct ctts_t const* ctts = trak->mdia_->minf_->stbl_->ctts_;
			unsigned int trun_index = 0;
			unsigned int s;
			struct bucket_t* bucket_prev = 0;

			traf->tfhd_ = tfhd_init();
			// 0x000020 = default-sample-flags present
			traf->tfhd_->flags_ = 0x000020;
			traf->tfhd_->track_id_ = trak->tkhd_->track_id_;
			// sample_degradation_priority
			traf->tfhd_->default_sample_flags_ = 0x000000;
			// sample_is_difference_sample
			if(trak->mdia_->hdlr_->handler_type_ == FOURCC('v', 'i', 'd', 'e'))
			{
				traf->tfhd_->default_sample_flags_ |= (1 << 16);
			}

			traf->trun_ = trun_init();
			// 0x0001 = data-offset is present
			// 0x0004 = first_sample_flags is present
			// 0x0100 = sample-duration is present
			// 0x0200 = sample-size is present
			traf->trun_->flags_ = 0x000305;
			// 0x0800 = sample-composition-time-offset is present
			//      if(ctts)
			{
				traf->trun_->flags_ |= 0x000800;
			}

			traf->trun_->sample_count_ = end - start;
			//    traf->trun_->data_offset_ = // set below
			traf->trun_->first_sample_flags_= 0x00000000;
			traf->trun_->table_ = (trun_table_t*)malloc(traf->trun_->sample_count_ * sizeof(trun_table_t));

			//      traf->trun_->trak_ = trak;
			//      traf->trun_->start_ = start;
			//    traf->trun_->uuid0_pts_ = trak_time_to_moov_time(
			//        trak->samples_[start].pts_, 10000000, trak->mdia_->mdhd_->timescale_);

			for(s = start; s != end; ++s)
			{
				uint64_t pts1 = trak->samples_[s + 1].pts_;
				uint64_t pts0 = trak->samples_[s + 0].pts_;

				unsigned int sample_duration = (unsigned int)(pts1 - pts0);

				uint64_t sample_pos = trak->samples_[s].pos_;
				unsigned int sample_size = trak->samples_[s].size_;
				unsigned int cto = trak->samples_[s].cto_;

				traf->trun_->table_[trun_index].sample_duration_ = sample_duration;
				traf->trun_->table_[trun_index].sample_size_ = sample_size;
				traf->trun_->table_[trun_index].sample_composition_time_offset_ = cto;

				MP4_INFO(
					"frame=%u pts=%"PRIi64" cto=%u duration=%u offset=%"PRIu64" size=%u\n",
					s,
					trak->samples_[s].pts_,
					trak->samples_[s].cto_,
					sample_duration,
					sample_pos, sample_size);

				if(trak->mdia_->hdlr_->handler_type_ == FOURCC('v', 'i', 'd', 'e'))
				{
#if 0
					if(bucket_prev == NULL)
					{
						// TODO: return error when no SPS and PPS are available
						if(is_avc)
						{
							unsigned char* buffer;
							unsigned char* p;
							unsigned int sps_pps_size =
								sample_entry->nal_unit_length_ + sample_entry->sps_length_ +
								sample_entry->nal_unit_length_ + sample_entry->pps_length_;

							if(sps_pps_size == 0)
							{
								MP4_ERROR("%s", "[Error] No SPS or PPS available\n");
								return 0;
							}

							buffer = (unsigned char*)malloc(sps_pps_size);
							p = buffer;

							// sps
							p = write_32(p, 0x00000001);
							memcpy(p, sample_entry->sps_, sample_entry->sps_length_);
							p += sample_entry->sps_length_;

							// pps
							p = write_32(p, 0x00000001);
							memcpy(p, sample_entry->pps_, sample_entry->pps_length_);
							p += sample_entry->pps_length_;

							bucket_insert_tail(buckets, bucket_init_memory(buffer, sps_pps_size));
							free(buffer);

							traf->trun_->table_[trun_index].sample_size_ += sps_pps_size;
							mdat_size += sps_pps_size;
							trun_mdat_size += sps_pps_size;
						}
					}
#endif

#if 0
					if(is_avc)
					{
						static const char nal_marker[4] = { 0, 0, 0, 1 };
						uint64_t first = sample_pos;
						uint64_t last = sample_pos + sample_size;
						while(first != last)
						{
							unsigned char buffer[4];
							unsigned int nal_size;
							bucket_insert_tail(buckets, bucket_init_memory(nal_marker, 4));

							if(fseeko(mp4_context->infile, first, SEEK_SET) != 0)
							{
								MP4_ERROR("%s", "Reached end of file prematurely\n");
								return 0;
							}
							if(fread(buffer, sample_entry->nal_unit_length_, 1, mp4_context->infile) != 1)
							{
								MP4_ERROR("%s", "Error reading NAL size\n");
								return 0;
							}
							nal_size = read_n(buffer, sample_entry->nal_unit_length_ * 8);

							if(nal_size == 0)
							{
								MP4_ERROR("%s", "Invalid NAL size (0)\n");
								return 0;
							}

							bucket_prev = bucket_init_file(first + sample_entry->nal_unit_length_, nal_size);
							bucket_insert_tail(buckets, bucket_prev);

							first += sample_entry->nal_unit_length_ + nal_size;
						}
					}
					else
#endif
					{
						// try to merge buckets
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
					}
				} else
					if(trak->mdia_->hdlr_->handler_type_ == FOURCC('s', 'o', 'u', 'n'))
					{
						// ADTS frame header
						if(sample_entry->wFormatTag == 0x00ff && output_raw)
						{
							unsigned char buffer[7];
							sample_entry_get_adts(sample_entry, sample_size, buffer);

							bucket_insert_tail(buckets, bucket_init_memory(buffer, 7));

							traf->trun_->table_[trun_index].sample_size_ += 7;
							mdat_size += 7;
							trun_mdat_size += 7;

							bucket_prev = NULL;
						}

						// try to merge buckets
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
					}

					mdat_size += sample_size;
					trun_mdat_size += sample_size;

					++trun_index;

					{
						// update woov track samples
						woov->moov->traks_[moof->tracks_-1]->samples_[woov->moov->traks_[moof->tracks_-1]->samples_size_++]=trak->samples_[s];
						woov->moov->traks_[moof->tracks_-1]->samples_[woov->moov->traks_[moof->tracks_-1]->samples_size_]=trak->samples_[s+1];
					}
			}
			{

				struct tfra_t* tfra = mfra->tfras_[moof->tracks_-1];
				tfra_table_t* table = &tfra->table_[moof->mfhd_->sequence_number_-1];
				table->time_ = trak->samples_[start].pts_;
				table->moof_offset_ = moof_offset;
				table->traf_number_ = moof->tracks_-1;
				table->trun_number_ = 0;
				table->sample_number_ = 0;
				stco_add_chunk(woov->moov->traks_[moof->tracks_-1]->mdia_->minf_->stbl_->stco_, woov->mdat_size);
				woov->mdat_size += trun_mdat_size;
				stsc_add_chunk(woov->moov->traks_[moof->tracks_-1]->mdia_->minf_->stbl_->stsc_, 
					woov->moov->traks_[moof->tracks_-1]->mdia_->minf_->stbl_->stco_->entries_-1, end-start, 1); 

			}
			// update size of mdat atom
			if(mdat_bucket)
			{
				write_32((unsigned char*)mdat_bucket->buf_, mdat_size);
			}
		}
	}

	return 1;
}

static stts_t* stts_create(mp4_context_t const* mp4_context,
													 samples_t const* first, samples_t const* last)
{
	unsigned int entries = 0;
	unsigned int samples = last - first;
	stts_t* stts = stts_init();
	stts->table_ = (stts_table_t*) realloc(stts->table_, samples * sizeof(stts_table_t));

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
	return stts;
}
//
static ctts_t* ctts_create(mp4_context_t const* mp4_context,
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
		unsigned int prev_cto = 0;
		unsigned int samples = last - first;
		ctts_t* ctts = ctts_init();
		ctts->table_ = (ctts_table_t*)
			malloc((samples) * sizeof(ctts_table_t));

		f = first; i = 0;
		prev_cto = f->cto_;
		while(f != last)
		{
			unsigned int sc = 0;
			while(f->cto_ == prev_cto && f != last)
			{
				++sc;
				++f;
			}
			ctts->table_[i].sample_count_ = sc ;
			ctts->table_[i].sample_offset_ = prev_cto;
			prev_cto = f->cto_;
			++i;
		}
		ctts->entries_ = i;
		if(ctts_get_samples(ctts) != samples)
		{
			MP4_WARNING("ERROR: stts_get_samples=%d, should be %d\n",
				ctts_get_samples(ctts), samples);
		}
		return ctts;
	}
}
static stsz_t* stsz_create(mp4_context_t const* mp4_context,
													 samples_t const* first, samples_t const* last)
{
	unsigned int i = 0;
	unsigned int first_sample_size = first->size_;
	unsigned int size_changed = 0;
	stsz_t* stsz = stsz_init();

	stsz->sample_size_ = 0;
	stsz->entries_ = last - first;
	stsz->sample_sizes_ = (uint32_t*)malloc(stsz->entries_ * sizeof(uint32_t));

	while(first != last)
	{
		stsz->sample_sizes_[i] = first->size_;
		if(stsz->sample_sizes_[i] != first_sample_size)
			size_changed = 1;
		++i;
		++first;

	}

	if (!size_changed)
	{
		stsz->sample_size_ = first_sample_size;
		free(stsz->sample_sizes_);
		stsz->sample_sizes_=0;
	}



	return stsz;
}

static stss_t* stss_create(mp4_context_t const* mp4_context,
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

static struct woov_t* woov_init(	struct mp4_context_t const* mp4_context,
struct moov_t const* fmoov)
{
	unsigned int i;
	struct woov_t* woov = malloc(sizeof(struct woov_t));
	woov->moov = moov_init();
	woov->mdat_size = 0;

	woov->moov->mvhd_ = mvhd_copy(fmoov->mvhd_);
	woov->moov->mvhd_->duration_ = 0;
	woov->moov->tracks_ = fmoov->tracks_;

	for(i = 0; i != woov->moov->tracks_; ++i)
	{

		trak_t const* src_trak = fmoov->traks_[i];
		trak_t* trak = trak_init();
		woov->moov->traks_[i] = trak;
		trak->tkhd_ = tkhd_copy(src_trak->tkhd_);
		trak->tkhd_->duration_ =0;
		trak->mdia_ = mdia_init();
		trak->mdia_->mdhd_ = mdhd_copy(src_trak->mdia_->mdhd_);
		trak->mdia_->mdhd_->duration_ = 0;
		trak->samples_size_ = 0;
		trak->samples_ = (samples_t*)
			malloc((src_trak->samples_size_ + 1) * sizeof(samples_t));
		trak->mdia_->hdlr_ = hdlr_copy(src_trak->mdia_->hdlr_);
		trak->mdia_->minf_ = minf_init();
		trak->mdia_->minf_->smhd_ = src_trak->mdia_->minf_->smhd_ == NULL ? NULL : smhd_copy(src_trak->mdia_->minf_->smhd_);
		trak->mdia_->minf_->vmhd_ = src_trak->mdia_->minf_->vmhd_ == NULL ? NULL : vmhd_copy(src_trak->mdia_->minf_->vmhd_);
		trak->mdia_->minf_->dinf_ = dinf_copy(src_trak->mdia_->minf_->dinf_);
		trak->mdia_->minf_->stbl_ = stbl_init();
		trak->mdia_->minf_->stbl_->stsd_ = stsd_copy(src_trak->mdia_->minf_->stbl_->stsd_);
		trak->mdia_->minf_->stbl_->stsc_ = stsc_init(); //sample-to-chunk, partial data-offset information
		trak->mdia_->minf_->stbl_->stco_ = stco_init(); //chunk offset, partial data-offset information
	}

	return woov;
}
static uint32_t woov_write(	struct mp4_context_t const* mp4_context,
struct woov_t* woov,
struct bucket_t** buckets)
{
	unsigned int i;
	uint32_t woov_size=0;
	bucket_t* woov_bucket = bucket_init(BUCKET_TYPE_MEMORY);
	bucket_insert_tail(buckets, woov_bucket);


	for(i = 0; i != woov->moov->tracks_; ++i)
	{
		trak_t* trak = woov->moov->traks_[i];

		samples_t const* first = (samples_t const*)&trak->samples_[0];
		samples_t const* last  = (samples_t const*)&trak->samples_[trak->samples_size_];

		// info need to update
		trak->mdia_->minf_->stbl_->stts_ = stts_create(mp4_context, first, last); //time-to-sample
		trak->mdia_->minf_->stbl_->ctts_ = ctts_create(mp4_context, first, last); //composition time-to-sample table
		trak->mdia_->minf_->stbl_->stsz_ = stsz_create(mp4_context, first, last); //sample sizes (framing)
		trak->mdia_->minf_->stbl_->stss_ = stss_create(mp4_context, first, last); //sync sample

		// update trak duration
		trak->mdia_->mdhd_->duration_ = last->pts_ - first->pts_;
		trak->tkhd_->duration_ = trak_time_to_moov_time(trak->mdia_->mdhd_->duration_,
			woov->moov->mvhd_->timescale_, trak->mdia_->mdhd_->timescale_);

		// update movie duration
		if(trak->tkhd_->duration_ > woov->moov->mvhd_->duration_)
		{
			woov->moov->mvhd_->duration_ = trak->tkhd_->duration_ ;
		}
	}

	woov_bucket->buf_ = malloc( 16 * 1024 * 1024 );
	woov_bucket->size_ = woov_size = moov_write(woov->moov, (unsigned char*)woov_bucket->buf_);
	//write_32((unsigned char*)woov_bucket->buf_ + 4, FOURCC('m', 'o', 'o', 'v'));
	write_32((unsigned char*)woov_bucket->buf_ + 4, FOURCC('f', 'r', 'e', 'e'));

	return woov_size;
}

static void woov_exit(struct woov_t *woov)
{
	if(woov)
	{
		if(woov->moov)
			moov_exit(woov->moov);
		free (woov);
	}

}

extern int mp4_fragment_file(struct mp4_context_t const* mp4_context,
struct bucket_t** buckets)
{
	uint64_t filepos = 0;
	int result = 1;

	moov_t* moov = mp4_context->moov;
	moov_t* fmoov;
	struct woov_t* woov = NULL;
	mfra_t* mfra;

	if(!moov_build_index(mp4_context, mp4_context->moov))
	{
		return 0;
	}

	// Start with the ftyp
	{
		unsigned char ftyp[28];
		unsigned char* buffer = ftyp;
		buffer = write_32(buffer, 28);
		buffer = write_32(buffer, FOURCC('f', 't', 'y', 'p'));
		buffer = write_32(buffer, FOURCC('a', 'v', 'c', '1'));  // major_brand
		buffer = write_32(buffer, 0);                           // minor_version
		buffer = write_32(buffer, FOURCC('i', 's', 'o', 'm'));  // compatible_brands
		buffer = write_32(buffer, FOURCC('i', 's', 'o', '2'));
		buffer = write_32(buffer, FOURCC('f', 'r', 'a', 'g'));
		bucket_insert_tail(buckets, bucket_init_memory(ftyp, sizeof(ftyp)));
		filepos += sizeof(ftyp);
	}
	{
		uint32_t i;
		struct trak_weight_t{
			int w;
			void* v;
		}wtrack[MAX_TRACKS];

		for(i = 0; i < moov->tracks_; i++)
		{
			if(moov->traks_[i]->mdia_->hdlr_->handler_type_  == FOURCC('s', 'o', 'u', 'n'))
			{wtrack[i].w=1;wtrack[i].v=moov->traks_[i];}
			else if(moov->traks_[i]->mdia_->hdlr_->handler_type_  == FOURCC('v', 'i', 'd', 'e'))
			{wtrack[i].w=2;wtrack[i].v=moov->traks_[i];}
			else
			{wtrack[i].w=3;wtrack[i].v=moov->traks_[i];}
		}
		for (i = 1; i < moov->tracks_; ++i)
		{
			unsigned int j;
			for (j= moov->tracks_ - 1; j>=i;j--)
			{
				if(wtrack[j].w < wtrack[j-1].w)
				{
					struct trak_weight_t t = wtrack[j];
					wtrack[j] = wtrack[j-1];
					wtrack[j-1] = t;
				}
			}
		}
		for (i = 0; i < moov->tracks_; ++i)
		{
			moov->traks_[i] = wtrack[i].v;
			moov->traks_[i]->tkhd_->track_id_=i+1;
		}
		moov->mvhd_->next_track_id_=i+1;

	}

	// A fragmented MPEG4 file starts with a MOOV atom with only the mandatory
	// atoms
	fmoov = moov_init();
	{
		unsigned int i;
		mvex_t* mvex = mvex_init();

		fmoov->mvhd_ = mvhd_copy(moov->mvhd_);
		fmoov->mvhd_->duration_ = 0;
		fmoov->tracks_ = moov->tracks_;
		fmoov->mvex_ = mvex;

		for(i = 0; i != moov->tracks_; ++i)
		{
			unsigned int s;
			trak_t* trak = moov->traks_[i];
			trak_t* ftrak = trak_init();
			mdia_t* mdia = trak->mdia_;
			mdia_t* fmdia = mdia_init();
			minf_t* minf = mdia->minf_;
			minf_t* fminf = minf_init();
			stbl_t* stbl = minf->stbl_;
			stbl_t* fstbl = stbl_init();

			fmoov->traks_[i] = ftrak;
			ftrak->tkhd_ = tkhd_copy(trak->tkhd_);
			ftrak->tkhd_->duration_ = 0;
			ftrak->mdia_ = fmdia;
			ftrak->samples_size_ = trak->samples_size_;
			ftrak->samples_ = (samples_t*)
				malloc((trak->samples_size_ + 1) * sizeof(samples_t));
			memcpy(ftrak->samples_, trak->samples_,
				(trak->samples_size_ + 1) * sizeof(samples_t));
			ftrak->smoothes_size_ = trak->smoothes_size_;
			ftrak->smoothes_ = (struct smooth_t*)
				malloc((trak->samples_size_ + 1) * sizeof(struct smooth_t));
			memcpy(ftrak->smoothes_, trak->smoothes_,
				(trak->smoothes_size_ + 1) * sizeof(struct smooth_t));
			fmdia->mdhd_ = mdhd_copy(mdia->mdhd_);
			// convert trak's timescale and duration
			fmdia->mdhd_->version_ = 1;
			fmdia->mdhd_->timescale_ = 10000000;
			fmdia->mdhd_->duration_ = 0;
			//        trak_time_to_moov_time(fmdia->mdhd_->duration_,
			//          fmdia->mdhd_->timescale_, mdia->mdhd_->timescale_);

			fmdia->hdlr_ = hdlr_copy(mdia->hdlr_);
			fmdia->minf_ = fminf;
			fminf->smhd_ = minf->smhd_ == NULL ? NULL : smhd_copy(minf->smhd_);
			fminf->vmhd_ = minf->vmhd_ == NULL ? NULL : vmhd_copy(minf->vmhd_);
			fminf->dinf_ = dinf_copy(minf->dinf_);
			fminf->stbl_ = fstbl;
			fstbl->stts_ = stts_init();
			fstbl->ctts_ = ctts_init();
			fstbl->stsz_ = stsz_init();
			fstbl->stsc_ = stsc_init();
			fstbl->stco_ = stco_init();
			fstbl->stsd_ = stsd_copy(stbl->stsd_);

			for(s = 0; s != ftrak->samples_size_ + 1; ++s)
			{
				// SmoothStreaming uses a fixed 10000000 timescale
				ftrak->samples_[s].pts_ = trak_time_to_moov_time(
					ftrak->samples_[s].pts_, ftrak->mdia_->mdhd_->timescale_,
					trak->mdia_->mdhd_->timescale_);
				ftrak->samples_[s].cto_ = (unsigned int)(trak_time_to_moov_time(
					ftrak->samples_[s].cto_, ftrak->mdia_->mdhd_->timescale_,
					trak->mdia_->mdhd_->timescale_));
			}

			{
				// update trak duration
				samples_t const* first = (samples_t const*)&ftrak->samples_[0];
				samples_t const* last  = (samples_t const*)&ftrak->samples_[ftrak->samples_size_];
				ftrak->mdia_->mdhd_->duration_ = last->pts_ - first->pts_;
				ftrak->tkhd_->duration_ = trak_time_to_moov_time(ftrak->mdia_->mdhd_->duration_,
					fmoov->mvhd_->timescale_, ftrak->mdia_->mdhd_->timescale_);

				// update movie duration
				if(ftrak->tkhd_->duration_ > fmoov->mvhd_->duration_)
				{
					fmoov->mvhd_->duration_ = ftrak->tkhd_->duration_ ;
				}
			}

			{
				trex_t* trex = trex_init();
				trex->track_id_ = trak->tkhd_->track_id_;
				trex->default_sample_description_index_ = 1;
				mvex->trexs_[mvex->tracks_] = trex;
				++mvex->tracks_;
			}

		}

		{
			unsigned char* moov_data = mp4_context->moov_data;
			uint32_t moov_size = moov_write(fmoov, moov_data);
			bucket_insert_tail(buckets, bucket_init_memory(moov_data, moov_size));
			filepos += moov_size;
		}
	}

	woov = woov_init(mp4_context, fmoov);

	mfra = mfra_init();
	mfra->tracks_ = fmoov->tracks_;

	{
		unsigned int i;
		unsigned int tfra_entries = 0;
		for(i = 0; i != fmoov->tracks_; ++i)
		{
			trak_t const* trak = fmoov->traks_[i];

			struct tfra_t* tfra = tfra_init();
			mfra->tfras_[i] = tfra;
			tfra->version_ = 1;
			tfra->flags_ = 0;
			tfra->track_id_ = trak->tkhd_->track_id_;
			tfra->length_size_of_traf_num_ = 1;
			tfra->length_size_of_trun_num_ = 1;
			tfra->length_size_of_sample_num_ = 1;

			// count the number of smooth sync samples (nr of moofs)
			tfra->number_of_entry_ = 0;
			{
				unsigned int start;
				for(start = 0; start != trak->samples_size_; ++start)
				{
					{
						if(trak->samples_[start].is_smooth_ss_)
						{
							++tfra->number_of_entry_;
						}
					}
				}
			}
			tfra->table_ = (tfra_table_t*)
				malloc(tfra->number_of_entry_ * sizeof(tfra_table_t));

			tfra_entries += tfra->number_of_entry_;

			// next track
		}

		{

			unsigned int tfra_index = 0;
			trak_t const* base_trak = fmoov->traks_[0];
			while(tfra_index != base_trak->smoothes_size_)
			{

				// insert moof bucket
				{
					moof_t* moof = moof_init();

					bucket_t* bucket = bucket_init(BUCKET_TYPE_MEMORY);
					bucket_insert_tail(buckets, bucket);

					// create moof and write samples
					moof_create(mp4_context, fmoov, woov,  moof, mfra, filepos, tfra_index+1, buckets, 0 /* OUTPUT_FORMAT_MP4 */);

					//          if(options->output_format == OUTPUT_FORMAT_MP4)
					{
						unsigned int samples_count = 0;
						unsigned char* moof_data = NULL;
						unsigned int moof_size = 0;
						for(i = 0;i < moof->tracks_; ++i)
							samples_count += moof->trafs_[i]->trun_->sample_count_;
						moof_data =	(unsigned char*)malloc(8192 + (samples_count) * 12);
						moof_size = moof_write(moof, moof_data);

						// now that we know the size of the moof atom, we know where the mdat
						// will start. We patch the 'data_offset' field to skip the
						// moof atom and the mdat header.
						moof->trafs_[0]->trun_->data_offset_ = moof_size + ATOM_PREAMBLE_SIZE;
						moof_size = moof_write(moof, moof_data);

						bucket->buf_ = malloc(moof_size);
						bucket->size_ = moof_size;
						memcpy(bucket->buf_, moof_data, (size_t)bucket->size_);
						free(moof_data);
					}
					moof_exit(moof);

					// advance filepos for moof and mdat atom
					while(*buckets != bucket)
					{
						filepos += bucket->size_;
						bucket = bucket->next_;
					}
				}

				// next fragment
				++tfra_index;
			}
		}

		moov_exit(fmoov);

		{
			uint32_t woov_size = woov_write(mp4_context, woov, buckets);
			int offset = 0;

			offset += 28; // ftyp
			offset += woov_size; //woov
			offset += ATOM_PREAMBLE_SIZE; //mdat
			woov->mdat_size+=8;	// header;
			if(woov->mdat_size > UINT32_MAX)
			{
				offset+=8;
				woov->mdat_size+=8;
			}	

			for(i = 0; i != woov->moov->tracks_; ++i)
			{
				trak_t* trak = woov->moov->traks_[i];
				stco_shift_offsets_inplace(
					(unsigned char*)trak->mdia_->minf_->stbl_->stco_->stco_inplace_,
					(int)offset);
			}

			write_32(woov_data + 8, woov_size);
			write_64(woov_data + 12, woov->mdat_size);
			bucket_insert_tail(buckets,
				bucket_init_memory(woov_data, sizeof(woov_data)));

			woov_exit(woov);
		}
		// Write the Movie Fragment Random Access (MFRA) atom
		{
			unsigned char* mfra_data =
				(unsigned char*)malloc(8192 + tfra_entries * 28);
			uint32_t mfra_size = mfra_write(mfra, mfra_data);
			bucket_insert_tail(buckets, bucket_init_memory(mfra_data, mfra_size));
			mfra_exit(mfra);
			free(mfra_data);
		}
	}

	return result;
}

// End Of File

