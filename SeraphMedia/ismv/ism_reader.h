/*******************************************************************************
 ism_reader.h - A library for reading SMIL.

 Copyright (C) 2009 CodeShop B.V.
 http://www.code-shop.com

 For licensing see the LICENSE file
******************************************************************************/ 

#ifndef ISM_READER_H_AKW
#define ISM_READER_H_AKW

#include "mod_streaming_export.h"

#ifndef _MSC_VER
#include <inttypes.h>
#else
#include "inttypes.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct smil_switch_t
{
  char* type_;
  char* src_;
  unsigned int system_bitrate_;
  unsigned int track_id_;
  struct smil_switch_t* next_;
};
typedef struct smil_switch_t smil_switch_t;
MOD_STREAMING_DLL_LOCAL extern smil_switch_t* smil_switch_init();
MOD_STREAMING_DLL_LOCAL extern void smil_switch_exit(smil_switch_t* smil_switch);

struct ism_t
{
  char* filename_;
  int depth;
  smil_switch_t* smil_switch_;
};
typedef struct ism_t ism_t;

MOD_STREAMING_DLL_LOCAL extern ism_t* ism_init(const char* filename);
MOD_STREAMING_DLL_LOCAL extern void ism_exit(ism_t* ism);

// return the name of the file and track id that corresponds to the given
// bit rate and // track type.
MOD_STREAMING_DLL_LOCAL extern
int ism_get_source(ism_t const* ism,
                   unsigned int bit_rate, char const* track_type,
                   const char** src, unsigned int* track_id);

#ifdef __cplusplus
} /* extern C definitions */
#endif

#endif // SMIL_PARSER_H_AKW

// End Of File

