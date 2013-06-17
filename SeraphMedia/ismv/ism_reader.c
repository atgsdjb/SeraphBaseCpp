/*******************************************************************************
 ism_reader.c - A library for reading SMIL.

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

#include "ism_reader.h"
#include "mp4_io.h"
#include "expat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#define strdup _strdup
#endif

extern smil_switch_t* smil_switch_init()
{
  smil_switch_t* smil_switch = (smil_switch_t*)malloc(sizeof(smil_switch_t));

  smil_switch->type_ = NULL;
  smil_switch->src_ = NULL;
  smil_switch->system_bitrate_ = 0;
  smil_switch->track_id_ = 0;
  smil_switch->next_ = 0;

  return smil_switch;
}

extern void smil_switch_exit(smil_switch_t* smil_switch)
{
  free(smil_switch->type_);
  free(smil_switch->src_);
  free(smil_switch);
}

static void XMLCALL
startElement(void *userData, const char *name, const char **atts)
{
  ism_t* ism = (ism_t*)(userData);

  if(!strcmp(name, "video") || !strcmp(name, "audio"))
  {
    smil_switch_t* smil_switch = smil_switch_init();
    smil_switch->next_ = ism->smil_switch_;
    ism->smil_switch_ = smil_switch;
    ism->smil_switch_->type_ = strdup(name);

    while(*atts != 0)
    {
      const char* name = *atts++;
      const char* value = *atts++;
//      printf("%s=%s", name, value);
      if(!strcmp(name, "src"))
      {
        ism->smil_switch_->src_ = strdup(value);
      } else
      if(!strcmp(name, "systemBitrate"))
      {
        ism->smil_switch_->system_bitrate_ = atoi(value);
      }
    }
  } else
  if(!strcmp(name, "param"))
  {
    const char* param_name = NULL;
    const char* param_value = NULL;

    while(*atts != 0)
    {
      const char* name = *atts++;
      const char* value = *atts++;
      if(!strcmp(name, "name"))
      {
        param_name = value;
      } else
      if(!strcmp(name, "value"))
      {
        param_value = value;
      }
    }

    if(param_name != NULL && param_value != NULL)
    {
      if(!strcmp(param_name, "trackID"))
      {
        ism->smil_switch_->track_id_ = atoi(param_value);
      }
    }
  }
}

static void XMLCALL
endElement(void *userData, const char *name)
{
  ism_t* ism = (ism_t*)(userData);
  ism->depth -= 1;
}

static int ism_read(ism_t* ism)
{
  int result = 1;
  XML_Parser parser;

  mem_range_t* mem_range = mem_range_init_read(ism->filename_);
  if(mem_range == NULL)
  {
    return 0;
  }

  parser = XML_ParserCreate(NULL);
  XML_SetUserData(parser, ism);
  XML_SetElementHandler(parser, startElement, endElement);

  {
    int done = 1;
    char const* first = (char const*)
      (mem_range_map(mem_range, 0, (uint32_t)mem_range->filesize_));
    char const* last = first + mem_range->filesize_;

    if(XML_Parse(parser, first, last - first, done) == XML_STATUS_ERROR)
    {
      result = 0;
    }
  }

  XML_ParserFree(parser);

  mem_range_exit(mem_range);

  return result;
}

extern ism_t* ism_init(const char* filename)
{
  ism_t* ism = (ism_t*)malloc(sizeof(ism_t));
  ism->filename_ = strdup(filename);
  ism->depth = 0;
  ism->smil_switch_ = 0;

  if(!ism_read(ism))
  {
    ism_exit(ism);
    return 0;
  }

  return ism;
}

extern void ism_exit(ism_t* ism)
{
  smil_switch_t* smil_switch = ism->smil_switch_;
  while(smil_switch)
  {
    smil_switch_t* next = smil_switch->next_;
    smil_switch_exit(smil_switch);
    smil_switch = next;
  }

  free(ism->filename_);
  free(ism);
}

extern int ism_get_source(ism_t const* ism,
                          unsigned int bit_rate, char const* track_type,
                          const char** src, unsigned int* track_id)
{
  smil_switch_t* smil_switch = ism->smil_switch_;

  for(; smil_switch; smil_switch = smil_switch->next_)
  {
    if(bit_rate && smil_switch->system_bitrate_ != bit_rate)
    {
      continue;
    }

    if(strcmp(smil_switch->type_, track_type))
    {
      continue;
    }

    *src = smil_switch->src_;
    *track_id = smil_switch->track_id_;
      return 1;
  }

  return 0;
}

// End Of File

