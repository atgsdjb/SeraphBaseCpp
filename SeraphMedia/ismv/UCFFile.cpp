#include "UCFFile.h"

#ifndef _I64_MIN
#define _I64_MIN    (-9223372036854775807ll - 1)
#define _I64_MAX      9223372036854775807ll
#endif

UCF_File::UCF_File(AP4_ByteStream *input_stream):m_inputStream(input_stream)
{
	m_VideoDesc = NULL;
	m_AudioDesc = NULL;
	m_bAudio = false;
	m_bVideo = false;
	m_bParseHeader = false;

	AP4_SetMemory(&m_FileHeader, 0, sizeof(m_FileHeader));

	m_bParseHeader = ParseFileHeader();
	m_StartTimeStamp = _I64_MAX;
	m_EndTimeStamp = _I64_MIN;
	m_pRemainData = NULL;
	m_RemainSize = 0;
}
// guid1 == guid2?true,false
bool is_equal_guid( const unsigned char guid1[16], const unsigned char guid2[16])
{
	for(int i = 0; i < 16; i++)
	{
		if(guid1[i] != guid2[i])
			return false;
	}
	return true;
}

bool UCF_File::ParseFileHeader()
{
	#define RETURN_IF_FAIL if (AP4_FAILED(result)) return false;

	AP4_Result result;
	AP4_Position stream_position = 0;
	
	result = m_inputStream->Seek(stream_position);
	RETURN_IF_FAIL;

	result = m_inputStream->Read(&m_FileHeader, sizeof(m_FileHeader));
	RETURN_IF_FAIL;

	if(FILEHEADERMAGIC != m_FileHeader.magic || FILEVERSION != m_FileHeader.version)
		return false;

	m_bAudio = false;
	m_bVideo = false;
	
	AVFORMAT *vf = NULL;
	AVFORMAT *va = NULL;
	if (m_FileHeader.video_format.extra_size > EXTRABUFFER)
		vf = (AVFORMAT*)(new AP4_UI08[sizeof(AVFORMAT) + m_FileHeader.video_format.extra_size - EXTRABUFFER]);
	else
		vf = (AVFORMAT*)(new AP4_UI08[sizeof(AVFORMAT)]);

	if (m_FileHeader.audio_format.extra_size > EXTRABUFFER)
		va = (AVFORMAT*)(new AP4_UI08[sizeof(AVFORMAT) + m_FileHeader.audio_format.extra_size - EXTRABUFFER]);
	else
		va= (AVFORMAT*)(new AP4_UI08[sizeof(AVFORMAT)]);
	
	AP4_CopyMemory(vf, &m_FileHeader.video_format, sizeof(AVFORMAT));
	AP4_CopyMemory(va, &m_FileHeader.audio_format, sizeof(AVFORMAT));

	int flag = m_FileHeader.flag;
	AP4_UI08 *temp = m_FileHeader.extra_data; 
	if(flag)
	{
		int extra_size = 0;
		if (flag & EXTRA_KEYID)
		{
			AP4_UI16 m_KeyID[17];
			AP4_UI16 *ps = (AP4_UI16*)(&temp[extra_size]);
			extra_size += sizeof(AP4_UI16);
			memcpy(m_KeyID, &temp[extra_size], *ps);
			m_KeyID[(*ps)/sizeof(AP4_UI16)]=L'\0';
			extra_size += *ps;
			bool m_bIsKeyIDPrensent = true;
		}

		if (flag & EXTRA_AUDIO_FORMAT)
		{
			uint16_t *ps = (uint16_t*)(&temp[extra_size]);
			extra_size += sizeof(uint16_t);
			memcpy(va->extra + EXTRABUFFER, &temp[extra_size], *ps);
			extra_size += *ps;

		}

		if (flag & EXTRA_VIDEO_FROMAT)
		{
			uint16_t *ps = (uint16_t*)(&temp[extra_size]);
			extra_size += sizeof(uint16_t);
			memcpy(vf->extra+EXTRABUFFER, &temp[extra_size], *ps);
			extra_size += *ps;
		}
		if(extra_size != m_FileHeader.extra_size)
		{
			delete[]vf;
			delete[]va;
			return false;
		}
	}

	if(FF_AVFT_WAVEFORMATEX == m_FileHeader.audio_format.format)
	{
		if(is_equal_guid(m_FileHeader.audio_GUID, MEDIASUBTYPE_AAC))
		{	
			m_bAudio = true;
			WAVEFORMATEX* pwfex = (WAVEFORMATEX*)va->extra;//m_FileHeader.audio_format.extra;
			m_AudioDesc = new AP4_MpegAudioSampleDescription(pwfex->nSamplesPerSec, pwfex->nChannels, AP4_MPEG2_STREAM_TYPE_AAC);
		}
		else if( is_equal_guid(m_FileHeader.audio_GUID, MEDIASUBTYPE_MP3) )
		{	
			m_bAudio = true;
			WAVEFORMATEX* pwfex = (WAVEFORMATEX*)m_FileHeader.audio_format.extra;
			m_AudioDesc = new AP4_MpegAudioSampleDescription(pwfex->nSamplesPerSec, pwfex->nChannels, AP4_MPEG2_STREAM_TYPE_MP3);
		}
	}
	if(FF_AVFT_VIDEOINFOHEADER == m_FileHeader.video_format.format || 
		FF_AVFT_VIDEOINFOHEADER2 == m_FileHeader.video_format.format ||
		FF_AVFT_MPEG2VIDEOINFO == m_FileHeader.video_format.format)
	{
		if( is_equal_guid(m_FileHeader.video_GUID, MEDIASUBTYPE_H264) ||
			is_equal_guid(m_FileHeader.video_GUID, MEDIASUBTYPE_h264) ||
			is_equal_guid(m_FileHeader.video_GUID, MEDIASUBTYPE_X264) ||
			is_equal_guid(m_FileHeader.video_GUID, MEDIASUBTYPE_x264) ||
			is_equal_guid(m_FileHeader.video_GUID, MEDIASUBTYPE_AVC1) ||
			is_equal_guid(m_FileHeader.video_GUID, MEDIASUBTYPE_avc1) )
		{
			m_bVideo = true;
			m_VideoDesc = new AP4_AvcSampleDescription(AP4_MPEG2_STREAM_TYPE_AVC);
			if (FF_AVFT_MPEG2VIDEOINFO == m_FileHeader.video_format.format)
			{
				MPEG2VIDEOINFO *mp2info = (MPEG2VIDEOINFO*)vf->extra;
				if(mp2info->cbSequenceHeader)
				{
					AP4_UI08 *p = (AP4_UI08*)mp2info->dwSequenceHeader;
					AP4_UI08 *pEnd = p+mp2info->cbSequenceHeader;
					AP4_DataBuffer sps, pps;
					while(p < pEnd)
					{
						int c= (p[0] << 8) | p[1];
						p+=2;
						
						int type = p[0] & 0x1F;
						if (type == 7) //sps
							sps.SetData(p, c);
						else if (type == 8) //pps
							pps.SetData(p,c);
						
						p += c;
					}
					m_VideoDesc->SetLevel(mp2info->dwLevel);
					m_VideoDesc->SetProfile(mp2info->dwProfile);
					m_VideoDesc->SetNaluLengthSize(mp2info->dwFlags);
					m_VideoDesc->SetSequenceParameters(sps);
					m_VideoDesc->SetPictureParameters(pps);
				}
			}
			
		}
	}
	
	delete[]vf;
	delete[]va;

	return true;
}

UCF_File::~UCF_File()
{
	if(m_pRemainData)
		delete m_pRemainData;
	if(m_VideoDesc)
		delete m_VideoDesc;
	if(m_AudioDesc)
		delete m_AudioDesc;
}

bool UCF_File::HasVideo()
{
	return m_bVideo;
}

bool UCF_File::HasAudio()
{
	return m_bAudio;
}
void UCF_File::GetRemain(AP4_UI08* pData, AP4_UI32& sizeData)
{
	if(NULL == pData)
	{
		sizeData = m_RemainSize;
		return;
	}
	
	if(m_RemainSize > 0)
	{
		AP4_CopyMemory(pData, m_pRemainData, m_RemainSize);
		sizeData = m_RemainSize;
	}
	
}
void UCF_File::GetSPSAndPPS(AP4_DataBuffer& sps, AP4_DataBuffer& pps)
{
	if (NULL != m_VideoDesc)
	{
		sps.SetData(m_VideoDesc->GetSequenceParameters().GetData(), m_VideoDesc->GetSequenceParameters().GetDataSize());
		pps.SetData(m_VideoDesc->GetPictureParameters().GetData(), m_VideoDesc->GetPictureParameters().GetDataSize());
	}
	
}
void UCF_File::SetSPSAndPPS(const AP4_DataBuffer& sps, const AP4_DataBuffer& pps)
{
	if (NULL != m_VideoDesc)
	{
		m_VideoDesc->SetSequenceParameters(sps);
		m_VideoDesc->SetPictureParameters(pps);
	}
}

AP4_SampleDescription* UCF_File::GetSampleDescription(AP4_Sample::Type track_type)
{
	if(AP4_Sample::TYPE_VIDEO == track_type)
		return m_VideoDesc;
	else if (AP4_Sample::TYPE_AUDIO == track_type)
		return m_AudioDesc;
	else
		return NULL;
}

AP4_Result UCF_File::GetSample(AP4_Sample& sample)
{
	if(!m_bParseHeader)
		return AP4_FAILURE;

	AP4_Result result;
	
	AP4_Position posFrame = 0;
	m_inputStream->Tell(posFrame);
	AP4_LargeSize sizeFile = 0;
	m_inputStream->GetSize(sizeFile);
	
	if(0 == sizeFile - posFrame)
		return AP4_ERROR_EOS;
	// check enough data for this frame
	if (sizeFile - posFrame < 4 )
	{
		if(m_pRemainData)
			delete[] m_pRemainData;
		
		m_RemainSize = (AP4_UI32)(sizeFile - posFrame);
		m_pRemainData = new AP4_Byte[m_RemainSize];
		m_inputStream->Read(m_pRemainData, m_RemainSize);
		
		return AP4_ERROR_EOS;
	}
	
	if ( SeekToNextFrame() )
	{
		FRAMEHEADER	frameHeader;
		
		m_inputStream->Tell(posFrame);
		m_inputStream->GetSize(sizeFile);
		
		if(0 == sizeFile - posFrame)
			return AP4_ERROR_EOS;
		// check enough data for this frame
		if (sizeFile - posFrame < sizeof(frameHeader))
		{
			if(m_pRemainData)
				delete[] m_pRemainData;
			
			m_RemainSize = (AP4_UI32)(sizeFile - posFrame);
			m_pRemainData = new AP4_Byte[m_RemainSize];
			m_inputStream->Read(m_pRemainData, m_RemainSize);
			
			return AP4_ERROR_EOS;
		}


		result = m_inputStream->Read(&frameHeader, sizeof(frameHeader));
		if (AP4_FAILED(result)) 
			return result;
		
		
		if(FRAMEHEADERMAGIC != frameHeader.magic)
			return AP4_FAILURE;
		
		if((sizeFile - posFrame) < (sizeof(frameHeader) + frameHeader.len))
		{
			m_inputStream->Seek(posFrame);

			if(m_pRemainData)
				delete[] m_pRemainData;

			m_RemainSize = (AP4_UI32)(sizeFile - posFrame);
			m_pRemainData = new AP4_Byte[m_RemainSize];
			m_inputStream->Read(m_pRemainData, m_RemainSize);

			return AP4_ERROR_EOS;
		}

		// calculate duration
		if (frameHeader.stamp_start < m_StartTimeStamp)
		{
			m_StartTimeStamp = frameHeader.stamp_start;
		}

		if (frameHeader.stamp_end > m_EndTimeStamp)
		{
			m_EndTimeStamp = frameHeader.stamp_end;
		}

		AP4_Size sample_size = frameHeader.len;
		AP4_UI64 dts = frameHeader.stamp_start;
		AP4_UI32 duration = (AP4_UI32)(frameHeader.stamp_end-frameHeader.stamp_start);
		
		// set the type
		if(IsAudioFrame(frameHeader.frame_type))
			sample.SetType(AP4_Sample::TYPE_AUDIO);
		else
			sample.SetType(AP4_Sample::TYPE_VIDEO);
		
		// set the check sum
		sample.SetCkSum(frameHeader.cksum);

		// set the dts and cts
		sample.SetDuration(duration);
		sample.SetDts(dts);
		sample.SetCts(dts);

		// set the size
		sample.SetSize(sample_size);

		// set the sync flag
		if (IsKeyFrame(frameHeader.frame_type)) {
			sample.SetSync(true);
		} else {
			sample.SetSync(false);
		}

		// set the offset
		AP4_Position stream_position = 0;
		m_inputStream->Tell(stream_position);
		sample.SetOffset(stream_position);

		// set the data stream
		sample.SetDataStream(*m_inputStream);

		return AP4_SUCCESS;
	}
	
	return AP4_FAILURE;

}

bool UCF_File::SeekToNextFrame()
{
	AP4_Result result;
	uint32_t magic = 0;

	while (AP4_SUCCESS == m_inputStream->Read(&magic, sizeof(magic)))
	{
		switch(magic)
		{
		case FILEHEADERMAGIC: // file  header
			{
				AP4_Position stream_position = 0;
				m_inputStream->Tell(stream_position);
				stream_position += sizeof(FILEHEADER)-sizeof(magic);
				result = m_inputStream->Seek(stream_position);
				if (AP4_FAILED(result)) 
					return false;

				break;
			}
		case GROUPHEADERMAGIC: // group header
			{
				AP4_Position stream_position = 0;
				m_inputStream->Tell(stream_position);
				stream_position += (sizeof(GROUPHEADER)-sizeof(magic));
				result = m_inputStream->Seek(stream_position);
				if (AP4_FAILED(result)) 
					return false;

				break;
			}
		case GROUPENDERMAGIC:	// group ender
			{
				AP4_Position stream_position = 0;
				m_inputStream->Tell(stream_position);
				stream_position += (sizeof(GROUPENDER)-sizeof(magic));
				result = m_inputStream->Seek(stream_position);
				if (AP4_FAILED(result)) 
					return false;

				break;
			}
		case FRAMEHEADERMAGIC:	// frame header
			{
				AP4_Position stream_position = 0;
				m_inputStream->Tell(stream_position);
				stream_position -= 4;
				result = m_inputStream->Seek(stream_position);
				if (AP4_FAILED(result)) 
					return false;

				return true;
				
				break;
			}
		case INDEXHEADERMAGIC:	// index header
			{
				return false;
				break;
			}
		case INDEXENDERMAGIC:	// index ender
			{
				return false;
				break;
			}
		default:				// seek to next bytes
			{
				AP4_Position stream_position = 0;
				m_inputStream->Tell(stream_position);
				stream_position -= 3;
				result = m_inputStream->Seek(stream_position);
				if (AP4_FAILED(result)) 
					return false;
				break;
			}
		}
	}
	
	return false;
}

//////////////////////////////////////////////////////////////////////////
// class Track
//////////////////////////////////////////////////////////////////////////


Track::Track(Movie* movie, AP4_UI32 trackid)
{
	m_Movie = movie;
	m_TrackID = trackid;
	m_Type = AP4_Sample::TYPE_UNKNOWN;
	m_TrackTimeScale = 0;
	m_Samples = NULL;
	m_SamplesCount = 0;
	m_SamplesIndex = 0;
	m_FragStart = 0;
}
Track::~Track(){
	if(m_Samples) 
		delete[] m_Samples;
}

void Track::SetType(AP4_Sample::Type type)
{
	m_Type = type;
}
AP4_Sample::Type Track::GetType()
{
	return m_Type;
}
AP4_UI32 Track::GetTrackID()
{
	return m_TrackID;
}

void Track::SetTrackTimeScale(AP4_UI32 track_time_scale)
{
	m_TrackTimeScale=track_time_scale;
}

AP4_UI32 Track::GetTrackTimeScale()
{
	return m_TrackTimeScale;
}

AP4_UI32 Track::SetSamples(FRAG_SAMPLE* samples, AP4_UI32 samples_count)
{
	if(m_Samples){
		delete[] m_Samples;
		m_Samples = NULL;
	}

	m_Samples = new FRAG_SAMPLE[ samples_count + 1 ];
	m_SamplesCount = samples_count;
	m_SamplesIndex = 0;
	AP4_CopyMemory(m_Samples, samples, (samples_count) * sizeof( FRAG_SAMPLE ));

	AP4_SI64 dts = m_FragStart;
	AP4_SI64 dur = 0;
	AP4_UI32 i = 0;
	for ( i = 0;i < m_SamplesCount; ++i)
	{
		dur = AP4_ConvertTime(m_Samples[i].dts, m_TrackTimeScale, m_Movie->GetMovieTimeScale());
		m_Samples[i].dts = dts ;
		m_Samples[i].cto = (AP4_SI32)AP4_ConvertTime(m_Samples[i].cto, m_TrackTimeScale, m_Movie->GetMovieTimeScale());
		dts += dur;
	}
	m_Samples[i].dts = dts;
	m_Samples[i].cto = 0;
	m_Samples[i].offset = 0;
	m_Samples[i].size = 0;

	return 0;
}

AP4_UI32& Track::SamplesIndex()
{
	return m_SamplesIndex;
}
AP4_UI32 Track::GetSamplesCount()
{
	return m_SamplesCount;
}
FRAG_SAMPLE* Track::GetEndSample()
{
	return &m_Samples[m_SamplesCount];
}
FRAG_SAMPLE* Track::GetSample( AP4_UI32 samples_index)
{
	if(!m_Samples || samples_index >= m_SamplesCount){
		return NULL;
	}

	return &m_Samples[samples_index];
}

//////////////////////////////////////////////////////////////////////////
// class Movie
//////////////////////////////////////////////////////////////////////////

Movie::Movie(AP4_UI32 time_scale)
{
	m_TrackCount = 0;
	m_MovieTimeScale = time_scale;
}
Movie::~Movie()
{
	if(m_TrackCount)
	{
		for (AP4_UI32 i = 0; i < m_TrackCount; ++i)
		{
			delete m_Track[i];
			m_Track[i] = NULL;
		}
	}
}

Track* Movie::GetTrack(AP4_UI32 trackid)
{
	for (AP4_UI32 i = 0; i< m_TrackCount; ++i)
	{
		if(m_Track[i]->GetTrackID() == trackid)
			return m_Track[i];
	}
	return NULL;
}
Track* Movie::GetTrackByIndex(AP4_UI32 index)
{
	if(index < m_TrackCount)
		return m_Track[index];
	return NULL;
}

AP4_UI32 Movie::GetTrackCount()
{
	return m_TrackCount;
}

AP4_UI32 Movie::GetMovieTimeScale()
{
	return m_MovieTimeScale;
};
bool Movie::AddTrack(AP4_UI32 trackid)
{
	if(m_TrackCount < MAX_TRACKS)
	{
		m_Track[m_TrackCount++] = new Track(this, trackid);
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
// class FMP4_File
//////////////////////////////////////////////////////////////////////////
//m_TotalVideoTs = 0;
//m_TotalAudioSample = 0;
AP4_UI64                        m_TotalVideoTs = 0;
AP4_UI64						m_TotalAudioSample = 0;
FMP4_File::FMP4_File(AP4_ByteStream *input_stream)
: m_inputStream(input_stream)
{
	m_Movie = NULL;
	m_VideoDesc = NULL;
	m_AudioDesc = NULL;
	m_bAudio = false;
	m_bVideo = false;
	m_bParseHeader = false;
	m_StartTimeStamp = _I64_MAX;
	m_EndTimeStamp = _I64_MIN;
	m_pRemainData = NULL;
	m_RemainSize = 0;
	m_bNeedParseMoof = true;
	m_MoofPos = 0;

	//<- modify by robert@2011.09.22
	//m_TotalVideoTs = 0;
	//m_TotalAudioSample = 0;
	//->
	
	m_bParseHeader = ParseFileHeader();
}

FMP4_File::~FMP4_File()
{
	if(m_pRemainData)
		delete m_pRemainData;
	if(m_VideoDesc)
		delete m_VideoDesc;
	if(m_AudioDesc)
		delete m_AudioDesc;
	if(m_Movie)
		delete m_Movie;
}

AP4_SampleDescription* FMP4_File::GetSampleDescription(AP4_Sample::Type track_type)
{
	if(AP4_Sample::TYPE_VIDEO == track_type)
		return m_VideoDesc;
	else if (AP4_Sample::TYPE_AUDIO == track_type)
		return m_AudioDesc;
	else
		return NULL;
}

AP4_UI32 FMP4_File::GetMediaTimeScale()
{
	if(m_Movie)
		return m_Movie->GetMovieTimeScale();
	return 10000000;// in 100 nanosecond
}

bool FMP4_File::HasVideo()
{
	return m_bVideo;
}

bool FMP4_File::HasAudio()
{
	return m_bAudio;
}

void FMP4_File::GetSPSAndPPS(AP4_DataBuffer& sps, AP4_DataBuffer& pps)
{
	if (NULL != m_VideoDesc)
	{
		sps.SetData(m_VideoDesc->GetSequenceParameters().GetData(), m_VideoDesc->GetSequenceParameters().GetDataSize());
		pps.SetData(m_VideoDesc->GetPictureParameters().GetData(), m_VideoDesc->GetPictureParameters().GetDataSize());
	}

}
void FMP4_File::SetSPSAndPPS(const AP4_DataBuffer& sps, const AP4_DataBuffer& pps)
{
	if (NULL != m_VideoDesc)
	{
		m_VideoDesc->SetSequenceParameters(sps);
		m_VideoDesc->SetPictureParameters(pps);
	}
}

AP4_UI32 FMP4_File::GetDuration()
{
	return (AP4_UI32)((m_EndTimeStamp-m_StartTimeStamp)/m_Movie->GetMovieTimeScale());
}

void FMP4_File::GetRemain(AP4_UI08* pData, AP4_UI32& sizeData)
{
	if(NULL == pData)
	{
		sizeData = m_RemainSize;
		return;
	}

	if(m_RemainSize > 0)
	{
		AP4_CopyMemory(pData, m_pRemainData, m_RemainSize);
		sizeData = m_RemainSize;
	}
}

// byte re-ordering
inline AP4_UI16 SwapShort(const AP4_UI08* pByte)
{
	return (pByte[0] << 8)  |
		pByte[1];
}

inline AP4_UI32 SwapLong(const AP4_UI08* pByte)
{
	return (pByte[0] << 24) |
		(pByte[1] << 16) |
		(pByte[2] << 8)  |
		pByte[3];
}

inline AP4_UI64 SwapI64(const AP4_UI08* pByte)
{
	return ((AP4_UI64)SwapLong(pByte))<< 32 |
		SwapLong(pByte + 4);
}

AP4_UI32 FMP4_File::ParseAtomHeader(AP4_Position pos, AP4_UI64& atom_size, AP4_UI32& atom_type)
{
	AP4_Result result;
	AP4_UI32 header_size = 0;
	AP4_Position stream_position = pos;
	result = m_inputStream->Seek(stream_position);
	if(AP4_FAILED(result))
		return 0;

	AP4_UI08 hdr[8];
	result = m_inputStream->Read(hdr, 8);
	if(AP4_FAILED(result))
		return 0;

	atom_size = SwapLong(hdr);
	atom_type = SwapLong(hdr+4);
	header_size += 8;

	if (atom_size == 1)
	{
		result = m_inputStream->Read(hdr, 8);
		if(AP4_FAILED(result))
			return 0;
		atom_size = SwapI64(hdr);
		header_size += 8;
	}
	
	return header_size;
}

// --- descriptor parsing ----------------------
bool 
Descriptor::Parse(const AP4_UI08* pBuffer, long cBytes)
{
	m_pBuffer = pBuffer;
	m_type = (eType)pBuffer[0];
	long idx = 1;
	m_cBytes = 0;
	do {
		m_cBytes = (m_cBytes << 7) + (pBuffer[idx] & 0x7f);
	} while ((idx < cBytes) && (pBuffer[idx++] & 0x80));

	m_cHdr = idx;
	if ((m_cHdr + m_cBytes) > cBytes)
	{
		m_cHdr = m_cBytes = 0;
		return false;
	}
	return true;
}


bool 
Descriptor::DescriptorAt(long cOffset, Descriptor& desc)
{
	return desc.Parse(Start() + cOffset, Length() - cOffset); 
}


bool FMP4_File::ParseFileHeader()
{
#define RETURN_IF_FAIL if (AP4_FAILED(result)) return false;

	AP4_Result result;
	AP4_Position stream_position = 0;
	AP4_UI64 atom_size = 0;
	AP4_UI32 atom_type = 0;
	AP4_UI32 atom_header_size = 0;
	m_bAudio = false;
	m_bVideo = false;
	m_bNeedParseMoof = true;

	result = m_inputStream->Seek(stream_position);
	RETURN_IF_FAIL;
	
	atom_header_size = ParseAtomHeader(stream_position, atom_size, atom_type);
	if(atom_header_size == 0)
		return false;
	
	if(atom_type != AP4_ATOM_TYPE('f', 't', 'y', 'p'))
		return false;
	
	stream_position += atom_size;

	atom_header_size = ParseAtomHeader(stream_position, atom_size, atom_type);
	if(atom_header_size == 0)
		return false;

	if(atom_type != AP4_ATOM_TYPE('m', 'o', 'o', 'v'))
		return false;
	
	AP4_UI32 cur_track_id;
	AP4_Position moov_end = stream_position + atom_size;
	stream_position += atom_header_size;
	
	do 
	{
		AP4_Position pos = stream_position;
		result = m_inputStream->Seek(pos);
		RETURN_IF_FAIL;
		
		atom_header_size = ParseAtomHeader(stream_position, atom_size, atom_type);
		if(atom_header_size == 0)
			return false;

		if(atom_type == AP4_ATOM_TYPE('m', 'v', 'h', 'd'))
		{
			AP4_UI32 time_scale = 0;
			AP4_UI08 ver = 0;
			result = m_inputStream->Read(&ver, 1);
			RETURN_IF_FAIL;

			if(ver == 1)
				pos += atom_header_size + 20;
			else
				pos += atom_header_size + 12;

			result = m_inputStream->Seek(pos);
			RETURN_IF_FAIL;
			
			AP4_UI08 timeScale[4];
			result = m_inputStream->Read(timeScale, 4);
			RETURN_IF_FAIL;
			
			time_scale = SwapLong(timeScale);

			//m_Movie = new Movie(time_scale);
			m_Movie = new Movie(10000000);	//
			stream_position += atom_size;
		}else
		if(atom_type == AP4_ATOM_TYPE('t', 'r', 'a', 'k'))
		{
			stream_position += atom_header_size;
		}else
		if(atom_type == AP4_ATOM_TYPE('t', 'k', 'h', 'd'))
		{
			AP4_UI32 track_id = 0;
			AP4_UI08 ver = 0;
			result = m_inputStream->Read(&ver, 1);
			RETURN_IF_FAIL;

			if(ver == 1)
				pos += atom_header_size + 20;
			else
				pos += atom_header_size + 12;

			result = m_inputStream->Seek(pos);
			RETURN_IF_FAIL;

			AP4_UI08 trackID[4];
			result = m_inputStream->Read(trackID, 4);
			RETURN_IF_FAIL;

			track_id = SwapLong(trackID);

			m_Movie->AddTrack(track_id);
			cur_track_id = track_id;
			stream_position += atom_size;
		}else
		if(atom_type == AP4_ATOM_TYPE('m', 'd', 'i', 'a'))
		{
			stream_position += atom_header_size;
		}else
		if(atom_type == AP4_ATOM_TYPE('m', 'd', 'h', 'd'))
		{
			AP4_UI32 time_scale = 0;
			AP4_UI08 ver = 0;
			result = m_inputStream->Read(&ver, 1);
			RETURN_IF_FAIL;

			if(ver == 1)
				pos += atom_header_size + 20;
			else
				pos += atom_header_size + 12;

			result = m_inputStream->Seek(pos);
			RETURN_IF_FAIL;

			AP4_UI08 timeScale[4];
			result = m_inputStream->Read(timeScale, 4);
			RETURN_IF_FAIL;

			time_scale = SwapLong(timeScale);

			Track* cur_track = m_Movie->GetTrack(cur_track_id);
			if(cur_track)
				cur_track->SetTrackTimeScale(time_scale);

			stream_position += atom_size;
		}else
		if(atom_type == AP4_ATOM_TYPE('h', 'd', 'l', 'r'))
		{
			AP4_UI32 handler_type = 0;

			pos += atom_header_size + 8;
			result = m_inputStream->Seek(pos);
			RETURN_IF_FAIL;

			AP4_UI08 handlerType[4];
			result = m_inputStream->Read(handlerType, 4);
			RETURN_IF_FAIL;

			handler_type = SwapLong(handlerType);

			Track* cur_track = m_Movie->GetTrack(cur_track_id);
			if ( NULL != cur_track )
			{
				if(handler_type == AP4_ATOM_TYPE('v', 'i', 'd', 'e'))
					cur_track->SetType(AP4_Sample::TYPE_VIDEO);
				else if(handler_type == AP4_ATOM_TYPE('s', 'o', 'u', 'n'))
					cur_track->SetType(AP4_Sample::TYPE_AUDIO);
				else
					cur_track->SetType(AP4_Sample::TYPE_UNKNOWN);
			}

			stream_position += atom_size;
		}else
		if(atom_type == AP4_ATOM_TYPE('m', 'i', 'n', 'f'))
		{
			stream_position += atom_header_size;
		}else
		if(atom_type == AP4_ATOM_TYPE('s', 't', 'b', 'l'))
		{
			stream_position += atom_header_size;
		}else
		if(atom_type == AP4_ATOM_TYPE('s', 't', 's', 'd'))
		{
			pos += atom_header_size + 4 ;
			result = m_inputStream->Seek(pos);
			RETURN_IF_FAIL;

			AP4_UI32 entry_count = 0;
			AP4_UI08 entryCount[4];
			result = m_inputStream->Read(entryCount, 4);
			entry_count = SwapLong(entryCount);
			RETURN_IF_FAIL;

			stream_position = pos + 4;
		}
		else
		if(atom_type == AP4_ATOM_TYPE('m', 'p', '4', 'a'))
		{
			pos += atom_header_size + 6 + 2 + 8 ;
			result = m_inputStream->Seek(pos);
			RETURN_IF_FAIL;

			AP4_UI32 channel_count = 2;
			AP4_UI08 channelCount[2];
			result = m_inputStream->Read(channelCount, 2);
			RETURN_IF_FAIL;
			channel_count = SwapShort(channelCount);

			AP4_UI32 sample_size = 16;
			AP4_UI08 sampleSize[2];
			result = m_inputStream->Read(sampleSize, 2);
			RETURN_IF_FAIL;
			sample_size = SwapShort(sampleSize);
			
			pos += 8;
			result = m_inputStream->Seek(pos);
			RETURN_IF_FAIL;

			AP4_UI32 sample_rate = 44100;
			AP4_UI08 sampleRate[4];
			result = m_inputStream->Read(sampleRate, 4);
			RETURN_IF_FAIL;
			sample_rate = SwapLong(sampleRate);
			sample_rate >>= 16;
			
			m_AudioDesc = new AP4_MpegAudioSampleDescription(sample_rate, channel_count, AP4_MPEG2_STREAM_TYPE_AAC);
			m_bAudio = true;
			stream_position = pos + 4;

		}else
		if(atom_type == AP4_ATOM_TYPE('a', 'v', 'c', '1'))
		{
			m_VideoDesc = new AP4_AvcSampleDescription(AP4_MPEG2_STREAM_TYPE_AVC);
			m_bVideo = true;
			stream_position += atom_header_size + 78;
		}else
		if(atom_type == AP4_ATOM_TYPE('e', 's', 'd', 's'))
		{
			pos += atom_header_size + 4;
			result = m_inputStream->Seek(pos);
			RETURN_IF_FAIL;
			
			AP4_UI08 payload[256];
			AP4_Size sizePayload = (AP4_Size)atom_size - 12;

			if(sizePayload > 256)
				return false;
			result = m_inputStream->Read(payload, sizePayload);
			RETURN_IF_FAIL;
			
			Descriptor ESDescr;
			if( !ESDescr.Parse(payload, sizePayload) || ESDescr.Type() != Descriptor::ES_DescrTag)
				return false;
			long cOffset = 3;
			AP4_UI08 flags = ESDescr.Start()[2];
			if (flags & 0x80)
			{
				cOffset += 2;   // depends-on stream
			}
			if (flags & 0x40)
			{
				// URL string -- count of chars precedes string
				cOffset += ESDescr.Start()[cOffset] + 1;
			}
			if (flags & 0x20)
			{
				cOffset += 2;   // OCR id
			}
			Descriptor dscDecoderConfig;
			if (!ESDescr.DescriptorAt(cOffset, dscDecoderConfig))
			{
				return false;
			}
			Descriptor dscSpecific;
			if (!dscDecoderConfig.DescriptorAt(13, dscSpecific))
			{
				return false;
			}

			//<- modify by robert
			int buf_size = dscSpecific.Length();
			const AP4_UI08* p_uint8 = dscSpecific.Start();
			int audio_obj_type;
			int sample_freq_index;
			int chn_config;

			if( buf_size >= 2 )
			{
				/** audio obj type, 5 bit */
				audio_obj_type = (p_uint8[0] >> 3) & 0x1F;
				/** sample freq index, 4 bit */
				sample_freq_index = ( ((p_uint8[0] & 0x7) << 1) | ((p_uint8[1] >> 7) & 0x1) );
				/** chn config, 4 bit */
				chn_config = (p_uint8[1] >> 3) & 0xF;

				if( m_AudioDesc )
				{
					switch( chn_config )
					{
					case 1:
						{
							m_AudioDesc->m_ChannelCount = 1;
						}
						break;

					case 2:
						{
							m_AudioDesc->m_ChannelCount = 2;
						}
						break;

					case 3:
						{
							m_AudioDesc->m_ChannelCount = 3;
						}
						break;

					case 4:
						{
							m_AudioDesc->m_ChannelCount = 4;
						}
						break;

					default:
						{
						}
						break;
					}
				}
			}
			//->
			
			stream_position += atom_size;

		}else
		if (atom_type == AP4_ATOM_TYPE('a', 'v', 'c', 'C'))
		{
			if(m_VideoDesc)
			{
				AP4_DataBuffer sps, pps;
				AP4_UI08 *p = NULL;
				AP4_UI08 avcC[256];
				AP4_Size size_avcC;
				
				pos += atom_header_size;
				result = m_inputStream->Seek(pos);
				RETURN_IF_FAIL;

				size_avcC = (AP4_Size)atom_size - 8;
				result = m_inputStream->Read(avcC, size_avcC);
				RETURN_IF_FAIL;

				m_VideoDesc->SetProfile(avcC[1]);
				m_VideoDesc->SetLevel(avcC[3]);
				m_VideoDesc->SetNaluLengthSize( ( avcC[4] & 0x3 ) + 1);
				
				int cSeq = avcC[5] & 0x1f;
				p = &avcC[6];
				for (int i = 0; i < cSeq; i++)
				{
					int c =  (p[0] << 8) + p[1];
					sps.SetData(&p[2], c);
					p += c+2;
				}
				int cPPS = *p++;
				for (int i = 0; i < cPPS; i++)
				{
					int c = (p[0] << 8) + p[1];
					pps.SetData(&p[2],c);
					p += c+2;
				}
				m_VideoDesc->SetSequenceParameters(sps);
				m_VideoDesc->SetPictureParameters(pps);

			}

			stream_position += atom_size;
		}
		else
		{
			stream_position += atom_size;
		}

	} while(stream_position < moov_end );
	
	if(stream_position != moov_end)
		return false;
	

	result = m_inputStream->Seek(stream_position);
	RETURN_IF_FAIL;
	
	m_MoofPos = stream_position;
	for (AP4_UI32 i = 0; i < m_Movie->GetTrackCount(); ++i)
	{
		Track *track = m_Movie->GetTrackByIndex(i);
		track->SetFragStart(0);
	}
	
	return true;
}

AP4_Result FMP4_File::ParseMoofAtom(AP4_Position moof_pos)
{
	#define RETURN_FOR_FAIL if (AP4_FAILED(result)) return result;
	FRAG_SAMPLE* pfragSample = NULL;
	AP4_Result result;
	AP4_UI32 cur_trackid = 0;
	AP4_Position stream_position = moof_pos;
	AP4_UI64 base_offset = 0;
	AP4_UI64 atom_size = 0;
	AP4_UI32 atom_type = 0;
	AP4_UI32 atom_header_size = 0;
	AP4_UI64 moof_end = 0;
	AP4_UI64 base_data_offset = 0;
	AP4_SI32 sample_description_index = 0;
	AP4_SI32 default_sample_duration = 0;
	AP4_SI32 default_sample_size = 0;
	AP4_SI32 default_sample_flags = 0;
	AP4_UI32 sample_count = 0;

	result = m_inputStream->Seek(stream_position);
	RETURN_FOR_FAIL;

	base_offset = stream_position;
	
	atom_header_size = ParseAtomHeader(stream_position, atom_size, atom_type);
	if(atom_header_size == 0)
		return AP4_FAILURE;

	if(atom_type != AP4_ATOM_TYPE('m', 'o', 'o', 'f'))
		return AP4_FAILURE;
		
	moof_end = stream_position + atom_size;
	stream_position += atom_header_size;

	do
	{
		AP4_Position pos = stream_position;
		result = m_inputStream->Seek(pos);
		RETURN_FOR_FAIL;
		atom_header_size = ParseAtomHeader(pos, atom_size, atom_type);
		if(atom_header_size == 0)
			return AP4_FAILURE;

		if(atom_type == AP4_ATOM_TYPE('m', 'f', 'h', 'd'))
		{
			AP4_UI08 verflag[4];
			m_inputStream->Read(verflag, 4);
			int ver = verflag[0];
			int flag = verflag[1] << 16 | verflag[2] << 8 | verflag[3];

			AP4_UI08 seq[4];
			result = m_inputStream->Read( seq, 4 );
			RETURN_FOR_FAIL;

			AP4_UI32 moof_seq = 0;
			moof_seq = SwapLong(seq);
			stream_position += atom_size;

		}else 
		if(atom_type == AP4_ATOM_TYPE('t', 'r', 'a', 'f'))
		{
			stream_position += atom_header_size;
		}else
		if(atom_type == AP4_ATOM_TYPE('t', 'f', 'h', 'd'))
		{
			AP4_UI08 verflag[4];
			m_inputStream->Read(verflag, 4);
			int ver = verflag[0];
			int flag = verflag[1] << 16 | verflag[2] << 8 | verflag[3];

			AP4_UI08 trackID[4];
			m_inputStream->Read(trackID, 4);
			RETURN_FOR_FAIL;
			cur_trackid = SwapLong(trackID);
			
			base_data_offset = 0;
			sample_description_index = 0;
			default_sample_duration = 0;
			default_sample_size = 0;
			default_sample_flags = 0;
			sample_count = 0;

			AP4_UI08 value[8];
			if(flag & 0x000001) // base_data_offset_present
			{
				result = m_inputStream->Read(value, 8);
				RETURN_FOR_FAIL;
				base_data_offset = SwapI64(value);
				base_offset = base_data_offset;
			}	
			if(flag & 0x000002) //sample_description_index_present
			{
				result = m_inputStream->Read(value, 4);
				RETURN_FOR_FAIL;
				sample_description_index = SwapLong(value);
			}
			if(flag & 0x000008) // default-sample-duration-present 
			{
				result = m_inputStream->Read(value, 4);
				RETURN_FOR_FAIL;
				default_sample_duration = SwapLong(value);
			}
			if(flag & 0x000010) //default-sample-size-present 
			{
				result = m_inputStream->Read(value, 4);
				RETURN_FOR_FAIL;
				default_sample_size = SwapLong(value);
			}
			if(flag & 0x000020) //default-sample-flags-present 
			{
				result = m_inputStream->Read(value, 4);
				RETURN_FOR_FAIL;
				default_sample_flags = SwapLong(value);
			}

			stream_position += atom_size;

		}else
		if(atom_type == AP4_ATOM_TYPE('t', 'r', 'u', 'n'))
		{
			
			AP4_SI32 data_offset = 0;
			AP4_SI32 first_sample_flag = 0;
			AP4_SI32 first_sample_flag_present = 0;
			AP4_SI32 sample_duration= 0;
			AP4_SI32 sample_size = 0;
			AP4_SI32 sample_flag = 0;
			AP4_SI32 sample_cto = 0;

			AP4_UI08 verflag[4];
			result = m_inputStream->Read(verflag, 4);
			RETURN_FOR_FAIL;
			//int ver = verflag[0];
			int flag = verflag[1] << 16 | verflag[2] << 8 | verflag[3];

			AP4_UI08 scount[4];
			result = m_inputStream->Read(scount, 4);
			RETURN_FOR_FAIL;
			AP4_UI32 trun_sample_count = (AP4_UI32)SwapLong(scount);
			
			if(trun_sample_count > 0)
			{
				FRAG_SAMPLE* pPreviousRunSample = pfragSample;
				pfragSample = new FRAG_SAMPLE[sample_count + trun_sample_count];
				AP4_CopyMemory(pfragSample, pPreviousRunSample, sample_count * sizeof(FRAG_SAMPLE));
				delete[] pPreviousRunSample;
			}
			
			AP4_UI08 value[4];
			if(flag & 0x000001)	// data_offset
			{
				result = m_inputStream->Read(value, 4);
				RETURN_FOR_FAIL;
				data_offset = SwapLong(value);
				base_offset += data_offset;
			}
			if(flag & 0x000004)	// first_sample_flag
			{
				result = m_inputStream->Read(value, 4);
				RETURN_FOR_FAIL;
				first_sample_flag = SwapLong(value);
				first_sample_flag_present = 1;
			}

			for(AP4_UI32 k = 0; k < trun_sample_count; k++, sample_count++)
			{
				if(pfragSample)
				{
					pfragSample[sample_count].offset = base_offset;
					pfragSample[sample_count].size = default_sample_size;
					pfragSample[sample_count].dts = default_sample_duration;
					pfragSample[sample_count].cto = 0;
					pfragSample[sample_count].sync = 0;
					if(k == 0 && first_sample_flag_present)
					{
						if( ! (first_sample_flag & 0x00010000) )
							pfragSample[sample_count].sync = 1;
						else
							pfragSample[sample_count].sync = 0;
					}
					
				}
				if(flag & 0x000100)	// sample-duration-present
				{
					result = m_inputStream->Read(value, 4);
					RETURN_FOR_FAIL;
					sample_duration = SwapLong(value);

					if(pfragSample)
						pfragSample[sample_count].dts = sample_duration;
				}

				if(flag & 0x000200)	// sample-size-present
				{
					result = m_inputStream->Read(value, 4);
					RETURN_FOR_FAIL;
					sample_size = SwapLong(value);

					if(pfragSample){
						pfragSample[sample_count].offset = base_offset;
						pfragSample[sample_count].size = sample_size;
					}

					base_offset += sample_size;
				}
				else
				{
					base_offset += default_sample_size;
				}

				if(flag & 0x000400)	// sample-flag-present
				{
					result = m_inputStream->Read(value, 4);
					RETURN_FOR_FAIL;
					sample_flag = SwapLong(value);
				}

				if(flag & 0x000800)	// sample-cto-present
				{
					result = m_inputStream->Read(value, 4);
					RETURN_FOR_FAIL;
					sample_cto = SwapLong(value);

					if(pfragSample)
						pfragSample[sample_count].cto = sample_cto;
				}
			}
			if(pfragSample)
			{
				m_Movie->GetTrack(cur_trackid)->SetSamples(pfragSample, sample_count);
				delete[] pfragSample;
				pfragSample = NULL;
			}

			stream_position += atom_size;
		}
		else
		{
			stream_position += atom_size;
		}

	}while( stream_position < moof_end );

	return AP4_SUCCESS;

}

AP4_Result FMP4_File::GetSample(AP4_Sample& sample)
{
	AP4_Result result = AP4_SUCCESS;
	AP4_Position mdat_pos = 0;
	AP4_UI64 atom_size = 0;
	AP4_UI32 atom_type = 0;
	AP4_UI32 atom_header_size = 0;
	AP4_LargeSize sizeFile = 0;
	bool	bEnoughData = false;
	
	if(!m_bParseHeader || !m_Movie)
		return AP4_FAILURE;

	m_inputStream->GetSize(sizeFile);
	
	while(m_bNeedParseMoof)
	{
		result = m_inputStream->Seek(m_MoofPos);
		if(AP4_FAILED(result))
			break;

		if( sizeFile - m_MoofPos < 24) // moof+mfhd
			break;
		
		atom_header_size = ParseAtomHeader(m_MoofPos, atom_size, atom_type);
		if(atom_header_size == 0)
			break;
		
		if(atom_type != AP4_ATOM_TYPE('m', 'o', 'o', 'f'))
		{
			m_MoofPos += 1;
			continue;
		}	
		
		mdat_pos = m_MoofPos + atom_size;
		result = m_inputStream->Seek(mdat_pos);
		if(AP4_FAILED(result))
			break;

		
		atom_header_size = ParseAtomHeader(mdat_pos, atom_size, atom_type);
		if(atom_header_size == 0)
			break;
		
		
		if(mdat_pos + atom_size > sizeFile)
			break;
	
		result = ParseMoofAtom(m_MoofPos);
		m_MoofPos = mdat_pos + atom_size;
		if(AP4_SUCCEEDED(result))
		{
			m_bNeedParseMoof = false;
			bEnoughData = true;
			break;
		}
	}
	
	if(!bEnoughData && m_bNeedParseMoof)
	{
		if(m_pRemainData)
			delete[] m_pRemainData;

		m_RemainSize = (AP4_UI32)(sizeFile - m_MoofPos);
		m_pRemainData = new AP4_Byte[m_RemainSize];
		m_inputStream->Seek(m_MoofPos);
		m_inputStream->Read(m_pRemainData, m_RemainSize);

		return AP4_ERROR_EOS;
	}
	
	if(!m_bNeedParseMoof)
	{
		AP4_UI32 sample_index = 0;
		AP4_SI64 min_timestamp = _I64_MAX;
		Track *track_select = NULL;
		for (AP4_UI32 i = 0;i < m_Movie->GetTrackCount(); ++i)
		{
			Track *track = m_Movie->GetTrackByIndex(i);
			if (track) {
				FRAG_SAMPLE * sample = track->GetSample(track->SamplesIndex());
				if(sample) {
					if(sample->dts < min_timestamp) {
						min_timestamp = sample->dts;
						track_select = track;
					}
				}
				
			}
		
		}
		if (track_select)
		{
			FRAG_SAMPLE *frag_sample = track_select->GetSample(track_select->SamplesIndex());
			++track_select->SamplesIndex();

			// set the type

			sample.SetType(track_select->GetType());

			// set the dts and cts
			FRAG_SAMPLE *frag_next_sample =track_select->GetSample(track_select->SamplesIndex());
			if(track_select->SamplesIndex() == track_select->GetSamplesCount())
				frag_next_sample =track_select->GetEndSample();
			
			sample.SetDuration((AP4_UI32)(frag_next_sample->dts - frag_sample->dts));
			
			// GetPreviousRunTime() only for 1 track run in 1 fragment
			sample.SetDts(frag_sample->dts );
			
			sample.SetCtsDelta(frag_sample->cto);

			// set the size
			sample.SetSize(frag_sample->size);

			// set the sync flag
			if(track_select->GetType() == AP4_Sample::TYPE_AUDIO)
			{
				sample.SetSync(true);
				//<- modify by robert@2011.09.22
				if( m_AudioDesc && m_AudioDesc->m_SampleRate )
				{
					sample.SetDts(m_TotalAudioSample * 90000 / m_AudioDesc->m_SampleRate);
					m_TotalAudioSample += 1024;
				}
				//->
			}
			if(track_select->GetType() == AP4_Sample::TYPE_VIDEO)
			{
				sample.SetSync(frag_sample->sync == 1 ? true : false);
				//<- modify by robert@2011.09.22
					sample.SetDts(m_TotalVideoTs);
					m_TotalVideoTs += (90000 / 25);
				//->
			}

			// set the offset
			sample.SetOffset(frag_sample->offset);

			// set the data stream
			sample.SetDataStream(*m_inputStream);
			
			// calculate duration
			if ((AP4_SI64)sample.GetDts() < m_StartTimeStamp)
			{
				m_StartTimeStamp = sample.GetDts();
			}

			if ((AP4_SI64)sample.GetCts()  > m_EndTimeStamp)
			{
				m_EndTimeStamp = sample.GetCts();
			}

		}
		else
		{
			sample.SetSize(0);
			sample.SetType(AP4_Sample::TYPE_UNKNOWN);
			m_bNeedParseMoof = true;
			
			for (AP4_UI32 i = 0;i < m_Movie->GetTrackCount(); ++i)
			{
				Track *track = m_Movie->GetTrackByIndex(i);
				if (track) 
				{
					FRAG_SAMPLE * end_sample = track->GetEndSample();
					if(end_sample) 
					{
						track->SetFragStart(end_sample->dts);
					}
				}
			}
		}
		
		return AP4_SUCCESS;

	}

	return AP4_FAILURE;
}