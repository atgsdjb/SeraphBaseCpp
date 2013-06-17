/*******************************************************************************
 output_m3u8.c - A library for writing M3U8 playlists.

 Copyright (C) 2009 CodeShop B.V.
 http://www.code-shop.com

 For licensing see the LICENSE file
******************************************************************************/ 

#ifndef OUTPUT_M3U8_H_AKW
#define OUTPUT_M3U8_H_AKW

#include "mod_streaming_export.h"

#ifdef __cplusplus
extern "C" {
#endif

struct mp4_context_t;
struct bucket_t;
struct mp4_split_options_t;

MOD_STREAMING_DLL_LOCAL extern
int mp4_create_m3u8(char const* basename,
                    struct mp4_context_t** mp4_context,
                    unsigned int mp4_contexts,
                    struct bucket_t** buckets);

#ifdef __cplusplus
} /* extern C definitions */
#endif

#endif // OUTPUT_M3U8_H_AKW

// End Of File

