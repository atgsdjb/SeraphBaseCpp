/*******************************************************************************
 output_ism.c - A library for writing Smooth Streaming Server Manifests.

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

#include "output_ism.h"
#include "mp4_io.h"
#include "mp4_reader.h"
#include "output_bucket.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int mp4_create_manifest_server(char const* client_manifest_filename,
                                      struct mp4_context_t** mp4_context,
                                      unsigned int mp4_contexts,
                                      struct bucket_t** buckets)
{
  unsigned int file;
  int result = 1;

  char* buffer = (char*)malloc(1024 * 256);
  char* p = buffer;

  p += sprintf(p, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
  p += sprintf(p, "<!--Created with mod_smooth_streaming(%s)-->\n",
               X_MOD_SMOOTH_STREAMING_VERSION);
  p += sprintf(p, "<smil xmlns=\"http://www.w3.org/2001/SMIL20/Language\">\n");
  p += sprintf(p, "  <head>\n");

  p += sprintf(p, "    "
    "<meta name=\"clientManifestRelativePath\" content=\"%s\" />\n",
    client_manifest_filename);

  p += sprintf(p, "  </head>\n");

  p += sprintf(p, "  <body>\n");
  p += sprintf(p, "    <switch>\n");

  for(file = 0; file != mp4_contexts; ++file)
  {
    struct mp4_context_t* context = mp4_context[file];
    struct moov_t const* moov = context->moov;
    unsigned int track;

    if(!moov_build_index(context, context->moov))
    {
      return 0;
    }

    for(track = 0; track != moov->tracks_; ++track)
    {
      struct trak_t const* trak = moov->traks_[track];
      unsigned int bitrate = ((trak_bitrate(trak) + 999) / 1000) * 1000;

      int is_video =
        trak->mdia_->hdlr_->handler_type_ == FOURCC('v', 'i', 'd', 'e');
      int is_audio =
        trak->mdia_->hdlr_->handler_type_ == FOURCC('s', 'o', 'u', 'n');

      if(is_video || is_audio)
      {
        const char* filename = strrchr(context->filename_, '/');
        filename = filename == NULL ? context->filename_ : (filename + 1);

        p += sprintf(p, "      "
          "<%s src=\"%s\" systemBitrate=\"%u\">\n",
          is_video ? "video" : "audio", filename, bitrate);
        p += sprintf(p, "        "
          "<param name=\"trackID\" value=\"%u\" valueType=\"data\" />\n",
          trak->tkhd_->track_id_);
        p += sprintf(p, "      "
          "</%s>\n", is_video ? "video" : "audio");
      }
    }
  }

  p += sprintf(p, "    </switch>\n");
  p += sprintf(p, "  </body>\n");
  p += sprintf(p, "</smil>\n");

  bucket_insert_tail(buckets, bucket_init_memory(buffer, p - buffer));
  free(buffer);

  return result;
}

// End Of File

