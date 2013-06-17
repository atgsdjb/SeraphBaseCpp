/*****************************************************************
|
|    AP4 - MP4 to MPEG2-TS File Converter
|
|    Copyright 2002-2009 Axiomatic Systems, LLC
|
|
|    This file is part of Bento4/AP4 (MP4 Atom Processing Library).
|
|    Unless you have obtained Bento4 under a difference license,
|    this version of Bento4 is Bento4|GPL.
|    Bento4|GPL is free software; you can redistribute it and/or modify
|    it under the terms of the GNU General Public License as published by
|    the Free Software Foundation; either version 2, or (at your option)
|    any later version.
|
|    Bento4|GPL is distributed in the hope that it will be useful,
|    but WITHOUT ANY WARRANTY; without even the implied warranty of
|    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
|    GNU General Public License for more details.
|
|    You should have received a copy of the GNU General Public License
|    along with Bento4|GPL; see the file COPYING.  If not, write to the
|    Free Software Foundation, 59 Temple Place - Suite 330, Boston, MA
|    02111-1307, USA.
|
 ****************************************************************/

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
//wu edit
//#define DETECT_MEM_LEAKS
#include <stdio.h>
#ifdef DETECT_MEM_LEAKS
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#else
#include <stdlib.h>
#endif


#include "Ap4Config.h"
#include "Ap4Types.h"
#include "Ap4Utils.h"
#include "Ap4ByteStream.h"
#include "Ap4Sample.h"
#include "Ap4Mpeg2Ts.h"
#include "Ap4FileByteStream.h"
#include "UCFFile.h"

/*----------------------------------------------------------------------
|   constants
+---------------------------------------------------------------------*/
#define BANNER "MP4 To MPEG2-TS File Converter - Version 1.0\n"\
               "(Bento4 Version 1.0)\n"\
               "(c) 2002-2009 Axiomatic Systems, LLC"
 
/*----------------------------------------------------------------------
|   PrintUsageAndExit
+---------------------------------------------------------------------*/
static void
PrintUsageAndExit()
{
    fprintf(stderr, 
            BANNER 
            "\n\nusage: mp42ts [options] <input> <output>\n");
    exit(1);
}

/*----------------------------------------------------------------------
|   WriteSamples
+---------------------------------------------------------------------*/
static void
WriteSamples(//UCF_File * ucf_file,
			 FMP4_File * ucf_file,
             AP4_Mpeg2TsWriter::SampleStream* audio_stream,
             AP4_Mpeg2TsWriter::SampleStream* video_stream,
             AP4_ByteStream& output)
{
    AP4_Sample	 sample;

    for (;;) {
        if(ucf_file){
			if(AP4_SUCCEEDED(ucf_file->GetSample(sample))){
				
				if(AP4_Sample::TYPE_AUDIO == sample.GetType()) {
					if (audio_stream) {
						audio_stream->WriteSample(sample, 
							ucf_file->GetSampleDescription(AP4_Sample::TYPE_AUDIO), 
							!ucf_file->HasVideo(),
							output);
					} else {
						AP4_DataBuffer data_buffer;
						sample.ReadData(data_buffer);
					}
				}
				else if (AP4_Sample::TYPE_VIDEO == sample.GetType()) {
					if (video_stream) {
						video_stream->WriteSample(sample, 
							ucf_file->GetSampleDescription(AP4_Sample::TYPE_VIDEO),
							true, 
							output);
					} else {
						AP4_DataBuffer data_buffer;
						sample.ReadData(data_buffer);
					}
					
				} else {
					AP4_DataBuffer data_buffer;
					sample.ReadData(data_buffer);
				}
			
			}else{
				break;
			}
		}
    }
}

//#define FILE_STREAM 1

/*----------------------------------------------------------------------
|   main
+---------------------------------------------------------------------*/
//wu edit
int
test1_main(int argc, char** argv)
{
#ifdef DETECT_MEM_LEAKS
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
	if (argc < 3) {
        PrintUsageAndExit();
    }

    // parse command line
    AP4_Result result;
    char** args = argv+1;

#ifdef FILE_STREAM
	// create the input stream
    AP4_ByteStream* input = NULL;
    result = AP4_FileByteStream::Create(*args++, AP4_FileByteStream::STREAM_MODE_READ, input);
    if (AP4_FAILED(result)) {
        fprintf(stderr, "ERROR: cannot open input (%d)\n", result);
    }
    
	// create the output stream
    AP4_ByteStream* output = NULL;
    result = AP4_FileByteStream::Create(*args++, AP4_FileByteStream::STREAM_MODE_WRITE, output);
    if (AP4_FAILED(result)) {
        fprintf(stderr, "ERROR: cannot open output (%d)\n", result);
    }
#else
	// create the input stream
	
	int parts = 10;
	int part=0;

	FILE *pf = fopen("e:\\1\\frag.mp4", "rb");

	AP4_UI08 *part_data =NULL;
	AP4_Size file_size = 0;
	AP4_Size part_size = 0;
	//AP4_Size header_size = sizeof(UCF_File::FILEHEADER);
	AP4_Size header_size = 0;
	{
		AP4_UI08 hdr[8];
		AP4_UI32 size = 0;
		fread(hdr, 1, 8, pf);
		size  = ((((AP4_UI32)hdr[0])<<24) | (((AP4_UI32)hdr[1])<<16) | (((AP4_UI32)hdr[2])<< 8) | (((AP4_UI32)hdr[3])));
		header_size += size;
		fseek(pf, size, SEEK_SET);
		fread(hdr, 1, 8, pf);
		size  = ((((AP4_UI32)hdr[0])<<24) | (((AP4_UI32)hdr[1])<<16) | (((AP4_UI32)hdr[2])<< 8) | (((AP4_UI32)hdr[3])));
		header_size += size;
		fseek(pf, 0, SEEK_SET);

	}
	AP4_UI32 remain_size=0;
	if(pf)
	{
		// get the size
		if (fseek(pf, 0, SEEK_END) >= 0) {
			file_size = ftell(pf);
			AP4_fseek(pf, 0, SEEK_SET);
		}
		
		part_size = (file_size-header_size)/parts;
		part_data = new AP4_UI08[part_size+header_size+10*1024*1024];
		fread(part_data,1, header_size, pf);

	}
	
	AP4_DataBuffer sps;
	AP4_DataBuffer pps;
	
	do
	{

	fread(part_data + header_size + remain_size, 1, part_size, pf);

	AP4_ByteStream* input = NULL;
	input = new AP4_MemoryByteStream(part_data, part_size+header_size+remain_size);
	
	
	// create the output stream
	AP4_MemoryByteStream* output = new AP4_MemoryByteStream();

#endif
	// open the file
	//UCF_File * input_file = new UCF_File(input);
	FMP4_File * input_file = new FMP4_File(input);

    if(!input_file->HasVideo() && !input_file->HasAudio())
		return -1;

#ifndef FILE_STREAM
	if (sps.GetDataSize() > 0)
	{
		input_file->SetSPSAndPPS(sps, pps);
	}
#endif

    // create an MPEG2 TS Writer
    AP4_Mpeg2TsWriter writer;
    AP4_Mpeg2TsWriter::SampleStream* audio_stream = NULL;
    AP4_Mpeg2TsWriter::SampleStream* video_stream = NULL;
    
	
    // add the audio stream
	if (input_file->HasAudio()) {
		AP4_MpegAudioSampleDescription* aac_desc = AP4_DYNAMIC_CAST(AP4_MpegAudioSampleDescription, input_file->GetSampleDescription(AP4_Sample::TYPE_AUDIO));
		if (aac_desc == NULL) return 0;
		result = writer.SetAudioStream(aac_desc->GetType(), 
									   input_file->GetMediaTimeScale(),
                                       audio_stream);
        if (AP4_FAILED(result)) {
            fprintf(stderr, "could not create audio stream (%d)\n", result);
            goto end;
        }
    }
    
    // add the video stream
	if (input_file->HasVideo()) {
		AP4_AvcSampleDescription* avc_desc = AP4_DYNAMIC_CAST(AP4_AvcSampleDescription, input_file->GetSampleDescription(AP4_Sample::TYPE_VIDEO));
		if (avc_desc == NULL) return 0;
        result = writer.SetVideoStream(avc_desc->GetType(),
									   input_file->GetMediaTimeScale(),
                                       video_stream);
        if (AP4_FAILED(result)) {
            fprintf(stderr, "could not create video stream (%d)\n", result);
            goto end;
        }
    }

    writer.WritePAT(*output);
    writer.WritePMT(*output);

	WriteSamples(input_file, 
				 audio_stream,
				 video_stream,
				 *output);

end:
#ifdef FILE_STREAM
	AP4_UI32 duration = input_file->GetDuration();
    delete input_file;
    input->Release();
    output->Release();
#else
	AP4_Position pos = 0;
	input->Tell(pos);
	input_file->GetSPSAndPPS(sps, pps);

	AP4_UI32 duration = input_file->GetDuration();
	remain_size = 0;
	input_file->GetRemain(NULL, remain_size);
	if(remain_size)
	{
		input_file->GetRemain(part_data+header_size, remain_size);
	}

	delete input_file;
	
	AP4_Size size_out = output->GetDataSize();
	char fn[128];
	sprintf(fn, "e:\\part%d.ts", part+1);

	FILE* part_file = fopen(fn, "wb");


	if(part_file)
	{
		fwrite(output->GetData(), 1, output->GetDataSize(), part_file);
		fclose(part_file);
		fprintf(stderr, "done %d file\n", part+1);
	}
	
	input->Release();
	output->Release();
	
	part++;

	} while(part < parts);
	
	delete[] part_data;

	fclose(pf);

#endif


    return 0;
}

