/*******************************************************************************
 mp4_fragment_reader.h - A library for reading MFRA / RXS files.

 Copyright (C) 2009 CodeShop B.V.
 http://www.code-shop.com

 For licensing see the LICENSE file
******************************************************************************/ 

#ifndef MP4_FRAGMENT_READER_H_AKW
#define MP4_FRAGMENT_READER_H_AKW

#include "mod_streaming_export.h"

#ifndef _MSC_VER
#include <inttypes.h>
#else
#include "inttypes.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct mp4_fragment_reader_t;
typedef struct mp4_fragment_reader_t mp4_fragment_reader_t;

MOD_STREAMING_DLL_LOCAL extern
mp4_fragment_reader_t* mp4_fragment_reader_init(char const* filename,
                                                unsigned int track_id);

MOD_STREAMING_DLL_LOCAL extern
void mp4_fragment_reader_exit(mp4_fragment_reader_t* mp4_fragment_reader);

MOD_STREAMING_DLL_LOCAL extern
int mp4_fragment_reader_get_offset(mp4_fragment_reader_t* mp4_fragment_reader,
                                   uint64_t pts,
                                   uint64_t* offset, uint64_t* size,
                                   unsigned int* index);

MOD_STREAMING_DLL_LOCAL extern
int mp4_fragment_reader_get_index(mp4_fragment_reader_t* mp4_fragment_reader,
                                  unsigned int index,
                                  uint64_t* offset, uint64_t* size,
                                  uint64_t* fragment_start);

#ifdef __cplusplus
} /* extern C definitions */
#endif

#endif // MP4_FRAGMENT_READER_H_AKW

// End Of File

