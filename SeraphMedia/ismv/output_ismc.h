/*******************************************************************************
 output_ismc.h - A library for writing Smooth Streaming Client Manifests.

 Copyright (C) 2009 CodeShop B.V.
 http://www.code-shop.com

 For licensing see the LICENSE file
******************************************************************************/ 

#ifndef OUTPUT_ISMC_H_AKW
#define OUTPUT_ISMC_H_AKW

#include "mod_streaming_export.h"

#ifdef __cplusplus
extern "C" {
#endif

struct mp4_context_t;
struct bucket_t;

MOD_STREAMING_DLL_LOCAL extern
int mp4_create_manifest_client(struct mp4_context_t** mp4_context,
                               unsigned int mp4_contexts,
                               struct bucket_t** buckets);

#ifdef __cplusplus
} /* extern C definitions */
#endif

#endif // OUTPUT_ISMC_H_AKW

// End Of File

