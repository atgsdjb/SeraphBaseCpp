/*******************************************************************************
 patch264.cpp - A command line tool for use together with the Streaming Module.
 
 Copyright (C) 2007-2010 CodeShop B.V.
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

#ifdef _MSC_VER
//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>
#ifdef _DEBUG
// -o e:\dest.ts e:\convert\delta.ism?bitrate=2954000&video=0&bitrate=64000&audio=0
// -o e:\dest.mov e:\dst.mp4
#include <vld.h>
#endif
#endif
#include "patch264.h"
#include "mp4_io.h"
#include "mp4_fragment.h"
#include "mp4_process.h"
#include "moov.h"
#include "output_bucket.h"
#include "output_mp4.h"
#include "output_mov.h"
#include "output_flv.h"
#include "output_ism.h"
#include "output_ismc.h"
#include "output_ismv.h"
#include "output_m3u8.h"
#include "ism_reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/stat.h>
#include <memory.h>
#include <getopt.h>

#ifdef WIN32
#define stat _stat64
#define strdup _strdup
#define unlink _unlink
#endif

#ifdef WIN32
#include <windows.h>
#endif

#ifndef WIN32
//#include <glob.h>
#endif

namespace // anonymous
{
	
	std::string change_extension(std::string const& filename,
								 std::string const& new_extension)
	{
		std::string::size_type extension = filename.rfind('.');
		
		return std::string(filename, 0, extension) + new_extension;
	}
	
	std::string path_leaf(std::string const& path)
	{
		std::string::size_type pos = path.rfind('/');
		if(pos == std::string::npos)
		{
			pos = path.rfind('\\');
		}
		if(pos != std::string::npos)
		{
			return path.substr(pos + 1);
		}
		
		return path;
	}
	
	char* backward_to_forward_slashes(char* path)
	{
#ifdef WIN32
		char* p;
		for(p = path; *p; ++p)
		{
			if(*p == '\\')
			{
				*p = '/';
			}
		}
#endif
		
		return path;
	}
	
	struct mp4_files_t
	{
		char* name_;
	};
	
#if 0
	int mp4_scanfiles(const char* input_file, unsigned int* files,
					  struct mp4_files_t* filespecs)
	{
		size_t max_files = *files;
		*files = 0;
		
		{
			const char* dir_end = strrchr(input_file, '/');
			if(dir_end == NULL)
			{
				dir_end = input_file;
			}
			else
			{
				++dir_end;
			}
			
			printf("scanning: %s\n", input_file);
			
#ifdef WIN32
			{
				WIN32_FIND_DATA ffd;
				HANDLE hFind = INVALID_HANDLE_VALUE;
				hFind = FindFirstFile(input_file, &ffd);
				if(hFind != INVALID_HANDLE_VALUE)
				{
					do
					{
						{
							size_t len = dir_end - input_file + strlen(ffd.cFileName);
							if((*files) == max_files)
							{
								break;
							}
							
							filespecs[*files].name_ = (char*)malloc(len + 1);
							filespecs[*files].name_[0] = '\0';
							strncat(filespecs[*files].name_, input_file, dir_end - input_file);
							strcat(filespecs[*files].name_, ffd.cFileName);
							//            filespecs[*files].size_ =
							//              ((uint64_t)(ffd.nFileSizeHigh) << 32) + ffd.nFileSizeLow;
							++(*files);
						}
					} while(FindNextFile(hFind, &ffd) != 0);
					FindClose(hFind);
				}
			}
#else
			{
				glob_t result;
				if(glob(input_file, 0, NULL, &result) != 0)
				{
					return 0;
				}
				unsigned int i;
				for(i = 0; i != result.gl_pathc; ++i)
				{
					if((*files) == max_files)
					{
						break;
					}
					else
					{
						char const* name = result.gl_pathv[i];
						size_t len = strlen(name);
						filespecs[*files].name_ = static_cast<char*>(malloc(len + 1));
						strcpy(filespecs[*files].name_, name);
						++(*files);
					}
				}
				globfree(&result);
			}
#endif
		}
		
		return 1;
	}
#endif
	
	int buckets_write(const char* output_file, bucket_t* buckets,
					  const char* input_file)
	{
		if(!buckets)
		{
			return 1;
		}
		
		uint64_t filesize = 0;
		int bucket_count = 0;
		
		{
			bucket_t* bucket = buckets;
			do
			{
				filesize += bucket->size_;
				bucket = bucket->next_;
				++bucket_count;
			} while(bucket != buckets);
		}
//		printf("writing %u buckets for a total of %"PRIu64" KBytes:\n",
//			   bucket_count, filesize >> 10);
		
		bucket_t* bucket = buckets;
		uint64_t filepos = 0;
		unsigned int last_percentage_shown = static_cast<unsigned int>(-1);
		
		// open input file
		mem_range_t* src_mem_range = mem_range_init_read(input_file);
		if(src_mem_range == 0)
		{
			return 0;
		}
		
		// open output file
		//  if(unlink(output_file) == -1)
		//  {
		//    perror("Could not delete file");
		//  }
		mem_range_t* dst_mem_range = mem_range_init_write(output_file, 0, filesize);
		if(dst_mem_range == 0)
		{
			return 0;
		}
		uint64_t dst_offset = 0;
		
		int result = 1;
		do
		{
			unsigned char const* src;
			for(uint64_t offset = 0; offset != bucket->size_;)
			{
				uint32_t size = static_cast<uint32_t>(
													  (std::min)(static_cast<uint64_t>(16 * 1024 * 1024),
																 bucket->size_ - offset));
				if(bucket->type_ == BUCKET_TYPE_MEMORY)
				{
					src = static_cast<unsigned char const*>(bucket->buf_) + offset;
				} else // if(bucket->type_ == BUCKET_TYPE_FILE)
				{
					src = static_cast<unsigned char const*>
					(mem_range_map(src_mem_range, bucket->offset_ + offset, size));
				}
				
				unsigned char* dst = static_cast<unsigned char*>(
																 mem_range_map(dst_mem_range, dst_offset, size));
				std::copy(src, src + size, dst);
				offset += size;
				dst_offset += size;
				
				filepos += size;
				static char const* progress0 =
				"======================================================================";
				static char const* progress1 =
				"                                                                      ";
				unsigned int percentage = (unsigned int)(100 * filepos / filesize);
				if(last_percentage_shown != percentage)
				{
					last_percentage_shown = percentage;
					int done = (int)(70 * filepos / filesize);
					//wu edit
//					printf("\r%3u%%[%.*s>%.*s]",
//						   percentage,
//						   done, progress0,
//						   70 - done, progress1);
					fflush(stdout);
				}
			}
			
			bucket = bucket->next_;
		} while(bucket != buckets && result);
		
		printf("\n");
		
		mem_range_exit(src_mem_range);
		mem_range_exit(dst_mem_range);
		
		return result;
	}
	
} // anonymous

////////////////////////////////////////////////////////////////////////////////

int segment_main(char* inputfile, char *outfile)
{
	char* input_file = 0;
	char* output_file = 0;
	int verbose = 2;
	
	char* query_params;
	
	output_file = backward_to_forward_slashes(outfile);
	
	
#define MAX_FILES 32
	
	unsigned int files = 0;
	struct mp4_files_t filespecs[MAX_FILES];
	
	filespecs[files].name_ = strdup(backward_to_forward_slashes(inputfile));
	++files;
	
	input_file = filespecs[0].name_;
	
	query_params = strstr(input_file, "?");
	if(query_params)
	{
		query_params[0] = '\0';
		++query_params;
	}
	
	int result = 1;
	struct mp4_split_options_t* options = mp4_split_options_init();
	
	int client_manifest = 0;
	int server_manifest = 0;
	
	int m3u8 = 0;
	
	if(query_params)
	{
		result = mp4_split_options_set(options, query_params, strlen(query_params));
		
		if(!result)
		{
			printf("Error reading query parameters for %s\n", query_params);
		}
	}
	
	bool fragment_file = false;
	if(output_file)
	{
		// the output file defines the output format
		if(ends_with(output_file, ".mp4"))
		{
			options->output_format = OUTPUT_FORMAT_MP4;
		}
		else if(ends_with(output_file, ".mov"))
		{
			options->output_format = OUTPUT_FORMAT_MOV;
		}
		else if(ends_with(output_file, ".aac") || ends_with(output_file, ".264"))
		{
			options->output_format = OUTPUT_FORMAT_RAW;
		}
		else if(ends_with(output_file, ".flv"))
		{
			options->output_format = OUTPUT_FORMAT_FLV;
		}
		else if(ends_with(output_file, ".ts"))
		{
			options->output_format = OUTPUT_FORMAT_TS;
		}
		else if(ends_with(output_file, ".ismv") || ends_with(output_file, ".isma"))
		{
			fragment_file = true;
		}
	}
	
	if(result)
	{
		if(output_file && ends_with(output_file, ".ismc"))
		{
			client_manifest = 1;
			printf("Creating client manifest (%s) for %s\n", output_file, input_file);
		} else
			if(output_file && ends_with(output_file, ".ism"))
			{
				server_manifest = 1;
				printf("Creating server manifest (%s) for %s\n", output_file, input_file);
			} else
				// if it is a fragment request then we read the server manifest file
				// and based on the bitrate and track type we set the filename and track id
				if(ends_with(input_file, ".ism") || ends_with(input_file, ".isms"))
				{
					if(options->output_format != OUTPUT_FORMAT_TS)
					{
						ism_t* ism = ism_init(input_file);
						
						if(ism == NULL)
						{
							printf("Error reading server manifest (%s)\n", input_file);
							result = 0;
						}
						else
						{
							const char* src;
							unsigned int fragment_bitrate;
							char const* fragment_type;
							if(options->audio_fragment_start != UINT64_MAX)
							{
								fragment_bitrate = options->audio_fragment_bitrate;
								fragment_type = fragment_type_audio;
							}
							else
							{
								fragment_bitrate = options->video_fragment_bitrate;
								fragment_type = fragment_type_video;
							}
							
							if(!ism_get_source(ism, fragment_bitrate,
											   fragment_type, &src, &options->fragment_track_id))
							{
								printf("%s with bitrate %u not found in manifest\n",
									   fragment_type, fragment_bitrate);
								result = 0;
							}
							else
							{
								char* dir_end = strrchr(input_file, '/');
								dir_end = dir_end == NULL ? input_file : (dir_end + 1);
								strcpy(dir_end, src);
								printf("Creating MP4 fragment (%s) for %s\n", output_file, input_file);
							}
							
							ism_exit(ism);
						}
					}
				} else
					if(output_file && ends_with(output_file, ".m3u8"))
					{
						m3u8 = 1;
						printf("Creating m3u8 playlist (%s) for %s\n", output_file, input_file);
					} else
					{
						printf("Creating MP4 file (%s) for %s [%.2f-%.2f>\n",
							   output_file, input_file, options->start, options->end);
					}
	}
	
	if(result)
	{
		options->client_is_flash = 1;
		
		// output buckets
		struct bucket_t* buckets = 0;
		
		printf("found %u files\n", files);
		
		if(fragment_file || client_manifest || server_manifest || m3u8)
		{
			mp4_context_t* mp4_context[MAX_FILES];
			memset(mp4_context, 0, sizeof(mp4_context));
			for(unsigned int file = 0; file != files; ++file)
			{
				uint64_t filesize = get_filesize(filespecs[file].name_);
				mp4_open_flags flags = options->fragments ? MP4_OPEN_MFRA : MP4_OPEN_ALL;
				printf("opening: %s\n", filespecs[file].name_);
				mp4_context[file] = mp4_open(filespecs[file].name_,
											 filesize, flags, verbose);
				if(mp4_context[file] == NULL)
				{
					printf("[Error] opening file %s\n", filespecs[file].name_);
					result = 0;
					break;
				}
			}
			
			if(result)
			{
				if(fragment_file)
				{
					mp4_fragment_file(mp4_context[0], &buckets);
				}
				else if(client_manifest)
				{
					// create manifest file for smooth streaming
					result = mp4_create_manifest_client(&mp4_context[0], files, &buckets);
				}
				else if(server_manifest)
				{
					std::string relative_client_manifest =
					path_leaf(change_extension(output_file, ".ismc"));
					result = mp4_create_manifest_server(relative_client_manifest.c_str(),
														&mp4_context[0], files, &buckets);
				}
				else if(m3u8)
				{
					std::string playlist_basename = change_extension(output_file, "");
					result = mp4_create_m3u8(playlist_basename.c_str(),
											 &mp4_context[0], files, &buckets);
				}
			}
			
			for(unsigned int file = 0; file != files; ++file)
			{
				if(mp4_context[file] != NULL)
				{
					mp4_close(mp4_context[file]);
				}
			}
		}
		else
		{
			int http_status = mp4_process(filespecs[0].name_,
										  get_filesize(filespecs[0].name_), verbose, &buckets, options);
			result = http_status == 200 ? 1 : 0;
		}
		
		if(!result)
		{
			printf("mp4split returned error\n");
		}
		else if(output_file != NULL)
		{
			result = buckets_write(output_file, buckets, input_file);
		}
		
		if(buckets)
		{
			buckets_exit(buckets);
		}
	}
	
	for(unsigned int file = 0; file != files; ++file)
	{
		free(filespecs[file].name_);
	}
	
	mp4_split_options_exit(options);
	
	return result == 0 ? 1 : 0;
}
// End Of File

