/*****************************************************************
|
|    AP4 - MPEG2 Transport Streams
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

#ifndef _AP4_MPEG2_TS_H_
#define _AP4_MPEG2_TS_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4Types.h"
#include "Ap4Array.h"
#include "Ap4DataBuffer.h"

/*----------------------------------------------------------------------
|   classes
+---------------------------------------------------------------------*/
class AP4_ByteStream;
class AP4_Sample;
class AP4_SampleDescription;
class AP4_MpegAudioSampleDescription;
class AP4_AvcSampleDescription;

class AP4_SampleDescription
{
public:
	AP4_IMPLEMENT_DYNAMIC_CAST(AP4_SampleDescription)
		AP4_SampleDescription(AP4_UI08 type):m_Type(type){}
	virtual ~AP4_SampleDescription() {}
	AP4_UI08 GetType(){return m_Type;}
private:
	AP4_UI08 m_Type;
};



class AP4_MpegAudioSampleDescription : public AP4_SampleDescription {
public:
	AP4_IMPLEMENT_DYNAMIC_CAST_D(AP4_MpegAudioSampleDescription, AP4_SampleDescription)
		enum{
			AP4_MPEG4_AUDIO_OBJECT_TYPE_AAC_MAIN              = 1,  /**< AAC Main Profile                             */
			AP4_MPEG4_AUDIO_OBJECT_TYPE_AAC_LC                = 2  /**< AAC Low Complexity                           */
	};

	AP4_MpegAudioSampleDescription(unsigned int sample_rate, unsigned int channel_count, AP4_UI08 audio_type) :
		AP4_SampleDescription(audio_type),
		m_SampleRate(sample_rate),
		m_ChannelCount(channel_count){}
		virtual ~AP4_MpegAudioSampleDescription(){};
	unsigned int GetSampleRate(){return m_SampleRate;}
	unsigned int GetChannelCount(){return m_ChannelCount;}
	unsigned int GetMpeg4AudioObjectType(){return AP4_MPEG4_AUDIO_OBJECT_TYPE_AAC_LC;}

	unsigned int m_SampleRate;
	unsigned int m_ChannelCount;
private:
};

class AP4_AvcSampleDescription : public AP4_SampleDescription {
public:
	AP4_AvcSampleDescription(AP4_UI08 video_type):
	  AP4_SampleDescription(video_type),
		  m_ConfigurationVersion(1),
		  m_Profile(0),
		  m_Level(0),
		  m_ProfileCompatibility(0),
		  m_NaluLengthSize(0){}
	virtual ~AP4_AvcSampleDescription(){}
	AP4_UI08 GetConfigurationVersion() const { return m_ConfigurationVersion; }
	AP4_UI08 GetProfile() const              { return m_Profile; }
	AP4_UI08 GetLevel() const                { return m_Level; }
	AP4_UI08 GetProfileCompatibility() const { return m_ProfileCompatibility; }
	AP4_UI08 GetNaluLengthSize() const       { return m_NaluLengthSize; }
	
	void SetConfigurationVersion(AP4_UI08 cv)  { m_ConfigurationVersion = cv; }
	void SetProfile(AP4_UI08 pf)               { m_Profile = pf; }
	void SetLevel(AP4_UI08 l)                  { m_Level = l; }
	void SetProfileCompatibility(AP4_UI08 pc)  { m_ProfileCompatibility = pc; }
	void SetNaluLengthSize(AP4_UI08 ns)        { m_NaluLengthSize = ns; }

	AP4_DataBuffer& GetSequenceParameters() { return m_SequenceParameters; }
	AP4_DataBuffer& GetPictureParameters()  { return m_PictureParameters;  }
	void SetSequenceParameters(const AP4_DataBuffer& sps){ m_SequenceParameters.SetData(sps.GetData(), sps.GetDataSize()); }
	void SetPictureParameters(const AP4_DataBuffer& pps) { m_PictureParameters.SetData(pps.GetData(), pps.GetDataSize());  }
private:
	AP4_UI08                  m_ConfigurationVersion;
	AP4_UI08                  m_Profile;
	AP4_UI08                  m_Level;
	AP4_UI08                  m_ProfileCompatibility;
	AP4_UI08                  m_NaluLengthSize;
	AP4_DataBuffer m_SequenceParameters;
	AP4_DataBuffer m_PictureParameters;
};

/*----------------------------------------------------------------------
|   constants
+---------------------------------------------------------------------*/
const AP4_UI16 AP4_MPEG2_TS_DEFAULT_PID_PMT         = 0x100;
const AP4_UI16 AP4_MPEG2_TS_DEFAULT_PID_AUDIO       = 0x101;
const AP4_UI16 AP4_MPEG2_TS_DEFAULT_PID_VIDEO       = 0x102;
const AP4_UI16 AP4_MPEG2_TS_DEFAULT_STREAM_ID_AUDIO = 0xc0;
const AP4_UI16 AP4_MPEG2_TS_DEFAULT_STREAM_ID_VIDEO = 0xe0;

const AP4_UI08 AP4_MPEG2_STREAM_TYPE_AAC			= 0x0F; // AAC
const AP4_UI08 AP4_MPEG2_STREAM_TYPE_MP3			 = 0x03; // MP3
const AP4_UI08 AP4_MPEG2_STREAM_TYPE_AVC             = 0x1B; // H264

/*----------------------------------------------------------------------
|   AP4_Mpeg2TsWriter
+---------------------------------------------------------------------*/
/**
 * This class is a simple implementation of a converter that can 
 * convert MP4 audio and video access units into an MPEG2 transport
 * stream.
 * It currently only supports one audio tracks with MPEG4 AAC LC, and one
 * video track with MPEG4 AVC.
 */
class AP4_Mpeg2TsWriter
{
public:
    // classes
    class Stream {
    public:
        Stream(AP4_UI16 pid) : m_PID(pid), m_ContinuityCounter(0) {}
        virtual ~Stream() {}
        
        AP4_UI16 GetPID() { return m_PID; }
        void WritePacketHeader(bool            payload_start, 
                               unsigned int&   payload_size,
                               bool            with_pcr,
                               AP4_UI64        pcr,
                               AP4_ByteStream& output);
        
    private:
        unsigned int m_PID;
        unsigned int m_ContinuityCounter;
    };
    
    class SampleStream : public Stream {
    public:
        SampleStream(AP4_UI16 pid, AP4_UI16 stream_id, AP4_UI08 stream_type, AP4_UI32 timescale) :
            Stream(pid), 
            m_StreamId(stream_id),
            m_StreamType(stream_type),
            m_TimeScale(timescale) {}
        
        virtual AP4_Result WritePES(const unsigned char* data, 
                                    unsigned int         data_size, 
                                    AP4_UI64             dts, 
                                    bool                 with_dts, 
                                    AP4_UI64             pts, 
                                    bool                 with_pcr, 
                                    AP4_ByteStream&      output);
        virtual AP4_Result WriteSample(AP4_Sample&            sample, 
                                       AP4_SampleDescription* sample_description,
                                       bool                   with_pcr, 
                                       AP4_ByteStream&        output) = 0;
        
        unsigned int m_StreamId;
        AP4_UI08     m_StreamType;
        AP4_UI32     m_TimeScale;
    };
    
    // constructor
    AP4_Mpeg2TsWriter();
    ~AP4_Mpeg2TsWriter();
    
    Stream* GetPAT() { return m_PAT; }
    Stream* GetPMT() { return m_PMT; }
    AP4_Result WritePAT(AP4_ByteStream& output);
    AP4_Result WritePMT(AP4_ByteStream& output);
    AP4_Result SetAudioStream(AP4_UI08 type, AP4_UI32 timescale, SampleStream*& stream);
    AP4_Result SetVideoStream(AP4_UI08 type, AP4_UI32 timescale, SampleStream*& stream);
    
private:
    Stream*       m_PAT;
    Stream*       m_PMT;
    SampleStream* m_Audio;
    SampleStream* m_Video;
};

#endif // _AP4_MPEG2_TS_H_
