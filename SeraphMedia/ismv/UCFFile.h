#ifndef _UCFFILE_H_
#define _UCFFILE_H_

#include "Ap4Config.h"
#include "Ap4Types.h"
#include "Ap4Utils.h"
#include "Ap4ByteStream.h"
#include "Ap4Sample.h"
#include "Ap4Mpeg2Ts.h"
#include "Ap4FileByteStream.h"

// MP3
// 00000055-0000-0010-8000-00AA00389B71
static const unsigned char MEDIASUBTYPE_MP3[16] =
{ 0x55, 0x00, 0x00, 0x00,0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 };

// AAC
// 000000FF-0000-0010-8000-00AA00389B71
static const unsigned char MEDIASUBTYPE_AAC[16] = 
{ 0xFF, 0x00, 0x00, 0x00, 0x00,0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 };

// AVC1
// 31435641-0000-0010-8000-00AA00389B71
static const unsigned char MEDIASUBTYPE_AVC1[16] =
{ 0x41, 0x56, 0x43, 0x31, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 };

//avc1
// 31637661-0000-0010-8000-00AA00389B71
static const unsigned char MEDIASUBTYPE_avc1[16] =
{ 0x61, 0x76, 0x63, 0x31, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 };

// H264
// 34363248-0000-0010-8000-00aa00389b71
static const unsigned char MEDIASUBTYPE_H264[16] =
{ 0x48, 0x32, 0x36, 0x34, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 };

// h264
// 34363268-0000-0010-8000-00AA00389B71
static const unsigned char MEDIASUBTYPE_h264[16] =
{ 0x68, 0x32, 0x36, 0x34, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 };

// X264
// 34363258-0000-0010-8000-00AA00389B71
static const unsigned char MEDIASUBTYPE_X264[16] =
{ 0x58, 0x32, 0x36, 0x34, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 };

// x264
// 34363278-0000-0010-8000-00AA00389B71
static const unsigned char MEDIASUBTYPE_x264[16] =
{ 0x78, 0x32, 0x36, 0x34, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 };


class UCF_File{
public:
	#pragma pack(push, 1)

	typedef unsigned char		uint8_t;
	typedef unsigned short		uint16_t;
	typedef unsigned int		uint32_t;
	typedef unsigned long long 	uint64_t;

	typedef char				int8_t;
	typedef short				int16_t;
	typedef int					int32_t;
	typedef long long			int64_t;


	#define MAKEMAGIC(ch0, ch1, ch2, ch3) \
	((int)(char)(ch0) | ((int)(char)(ch1) << 8) |   \
	((int)(char)(ch2) << 16) | ((int)(char)(ch3) << 24 ))

	const static int FILEHEADERMAGIC = MAKEMAGIC('F', 'I', 'L', 'E');
	const static int GROUPHEADERMAGIC= MAKEMAGIC('G', 'R', 'O', 'U');
	const static int GROUPENDERMAGIC = MAKEMAGIC('U', 'O', 'R', 'G');
	const static int FRAMEHEADERMAGIC= MAKEMAGIC('F', 'R', 'A', 'M');
	const static int INDEXHEADERMAGIC= MAKEMAGIC('I', 'D', 'X', 'H');
	const static int INDEXENDERMAGIC = MAKEMAGIC('E', 'X', 'D', 'I');
	const static int FILEVERSION = 103;
	
	typedef enum FF_FRAMETYPE
	{
		FF_AUDIO_IFRAME = 0,
		FF_VIDEO_IFRAME = 1,
		FF_VIDEO_PFRAME = 2,
		FF_VIDEO_BFRAME = 4,
		FF_AUDIO_PFRAME = 5
	}FF_FRAMETYPE;

	#define IsAudioFrame(ft) ((ft == FF_AUDIO_IFRAME || ft == FF_AUDIO_PFRAME) ? true : false)
	#define IsVideoFrame(ft) ((ft == FF_VIDEO_IFRAME || ft == FF_VIDEO_PFRAME || ft == FF_VIDEO_BFRAME) ? true : false)
	#define IsKeyFrame(ft) ((ft == FF_AUDIO_IFRAME || ft == FF_VIDEO_IFRAME) ? true : false)

	typedef enum _AVFORMATTYPE
	{
		FF_AVFT_WAVEFORMATEX=1,
		FF_AVFT_VIDEOINFOHEADER,
		FF_AVFT_VIDEOINFOHEADER2,
		FF_AVFT_MPEG2VIDEOINFO
	} AVFORMATTYPE;

	#define EXTRABUFFER 128
	typedef struct _AVFORMAT
	{
		AVFORMATTYPE format;
		uint32_t extra_size;
		char extra[EXTRABUFFER];
	}AVFORMAT;

	typedef struct tagRECT
	{
		int32_t    left;
		int32_t    top;
		int32_t    right;
		int32_t    bottom;
	} RECT;

	typedef struct tagBITMAPINFOHEADER{
		uint32_t       biSize;
		int32_t				biWidth;
		int32_t				biHeight;
		uint16_t      biPlanes;
		uint16_t      biBitCount;
		uint32_t       biCompression;
		uint32_t       biSizeImage;
		int32_t				biXPelsPerMeter;
		int32_t				biYPelsPerMeter;
		uint32_t       biClrUsed;
		uint32_t       biClrImportant;
	} BITMAPINFOHEADER;

	typedef struct tagVIDEOINFOHEADER {

		RECT            rcSource;          // The bit we really want to use
		RECT            rcTarget;          // Where the video should go
		uint32_t   dwBitRate;         // Approximate bit data rate
		uint32_t   dwBitErrorRate;    // Bit error rate for this stream
		int64_t			AvgTimePerFrame;   // Average time per frame (100ns units)

		BITMAPINFOHEADER bmiHeader;

	} VIDEOINFOHEADER;

	typedef struct tagVIDEOINFOHEADER2 {
		RECT                rcSource;
		RECT                rcTarget;
		uint32_t            dwBitRate;
		uint32_t            dwBitErrorRate;
		int64_t				AvgTimePerFrame;
		uint32_t            dwInterlaceFlags;   // use AMINTERLACE_* defines. Reject connection if undefined bits are not 0
		uint32_t            dwCopyProtectFlags; // use AMCOPYPROTECT_* defines. Reject connection if undefined bits are not 0
		uint32_t            dwPictAspectRatioX; // X dimension of picture aspect ratio, e.g. 16 for 16x9 display
		uint32_t            dwPictAspectRatioY; // Y dimension of picture aspect ratio, e.g.  9 for 16x9 display
		union {
			uint32_t		dwControlFlags;     // use AMCONTROL_* defines, use this from now on
			uint32_t		dwReserved1;        // for backward compatiblity (was "must be 0";  connection rejected otherwise)
		};
		uint32_t            dwReserved2;        // must be 0; reject connection otherwise
		BITMAPINFOHEADER    bmiHeader;
	} VIDEOINFOHEADER2;

	typedef struct tagMPEG2VIDEOINFO {
		VIDEOINFOHEADER2    hdr;
		uint32_t            dwStartTimeCode;        //  ?? not used for DVD ??
		uint32_t            cbSequenceHeader;       // is 0 for DVD (no sequence header)
		uint32_t            dwProfile;              // use enum MPEG2Profile
		uint32_t            dwLevel;                // use enum MPEG2Level
		uint32_t            dwFlags;                // use AMMPEG2_* defines.  Reject connection if undefined bits are not 0
		uint32_t            dwSequenceHeader[1];    // uint32_t instead of Byte for alignment purposes
		//   For MPEG-2, if a sequence_header is included, the sequence_extension
		//   should also be included
	} MPEG2VIDEOINFO;
#define FIELD_OFFSET(type, field)    ((int32_t)(int32_t)&(((type *)0)->field))
#define SIZE_MPEG2VIDEOINFO(pv) (FIELD_OFFSET(MPEG2VIDEOINFO, dwSequenceHeader[0]) + (pv)->cbSequenceHeader)
#define MPEG2_SEQUENCE_INFO(pv) ((const BYTE *)(pv)->dwSequenceHeader)

	typedef struct tagWAVEFORMATEX
	{
		unsigned short        wFormatTag;         // format type 
		unsigned short        nChannels;          // number of channels (i.e. mono, stereo...) 
		unsigned long       nSamplesPerSec;       // sample rate 
		unsigned long       nAvgBytesPerSec;      // for buffer estimation 
		unsigned short        nBlockAlign;        // block size of data
		unsigned short        wBitsPerSample;     // number of bits per sample of mono data 
		unsigned short        cbSize;             // the count in bytes of the size of 
		// extra information (after cbSize) 
	} WAVEFORMATEX;

	#define FILEHEADERHEADERSIZE 1024*1
	typedef struct _FILEHEADER
	{
		uint32_t magic;
		uint32_t version;
		unsigned char video_GUID[16];
		unsigned char audio_GUID[16]; 
		AVFORMAT audio_format;
		AVFORMAT video_format;
		uint64_t index_pos;
		uint32_t flag;
		uint32_t extra_size;
		uint8_t  extra_data[FILEHEADERHEADERSIZE];
	}FILEHEADER,*PFILEHEADER;
	
	typedef enum _EXTRAFLAG{
		EXTRA_KEYID = 1,
		EXTRA_AUDIO_FORMAT  = 2,
		EXTRA_VIDEO_FROMAT = 4
	}EXTRAFLAG;

	typedef struct _GROUPHEADER
	{
		uint32_t magic;
		uint32_t audio_format;
		uint32_t video_format;
		uint32_t reserved;
		int64_t start_stamp;
	} GROUPHEADER,*PGROUPHEADER;

	typedef struct _GROUPENDER
	{
		uint32_t magic;
		uint32_t len;
		int64_t end_stamp;
	} GROUPENDER,*PGROUPENDER;

	typedef struct _FRAMEHEADER
	{
		int magic;
		unsigned short frame_type;
		unsigned short cksum;
		int64_t stamp_start;
		int64_t stamp_end;
		uint32_t len;
		uint32_t reserved;
	} FRAMEHEADER,*PFRAMEHEADER;

	typedef struct _FRAMEDATA
	{
		FRAMEHEADER hdr;
		unsigned char* data;
	} FRAMEDATA,*PFRAMEDATA;

	typedef struct _INDEXHEADER
	{
		uint32_t magic;
		uint32_t node_count;
	}INDEXHEADER, *PINDEXHEADER;

	typedef struct _INDEXENDER
	{
		uint32_t magic;
		uint32_t reserved;
	}INDEXENDER, *PINDEXENDER;

	#pragma pack(pop)

public:
	UCF_File(AP4_ByteStream *input_stream);
	virtual ~UCF_File();
	AP4_SampleDescription *GetSampleDescription(AP4_Sample::Type track_type);
	AP4_Result GetSample(AP4_Sample& sample);
	AP4_UI32 GetMediaTimeScale(){return 10000000;};	// in 100 nanosecond
	bool HasVideo();
	bool HasAudio();
	bool ParseFileHeader();
	void GetSPSAndPPS(AP4_DataBuffer& sps, AP4_DataBuffer& pps);
	void SetSPSAndPPS(const AP4_DataBuffer& sps, const AP4_DataBuffer& pps);
	AP4_UI32 GetDuration(){return (AP4_UI32)((m_EndTimeStamp-m_StartTimeStamp)/10000000);}
	void GetRemain(AP4_UI08* pData, AP4_UI32& sizeData);
protected:

private:
	bool SeekToNextFrame();

private:
	AP4_ByteStream*					m_inputStream;
	AP4_MpegAudioSampleDescription*	m_AudioDesc;
	AP4_AvcSampleDescription*		m_VideoDesc;
	bool							m_bAudio;
	bool							m_bVideo;
	FILEHEADER						m_FileHeader;
	bool							m_bParseHeader;
	int64_t							m_StartTimeStamp;
	int64_t							m_EndTimeStamp;
	AP4_UI08*						m_pRemainData;
	AP4_UI32						m_RemainSize;
};

//////////////////////////////////////////////////////////////////////////
// class FMP4_File
//////////////////////////////////////////////////////////////////////////

class Descriptor
{
public:
	Descriptor()
		: m_pBuffer(NULL),
		m_cBytes(0),
		m_cHdr(0),
		m_type(InvalidTag)
	{
	}

	bool Parse(const AP4_UI08* pBuffer, long cBytes);

	enum eType
	{
		InvalidTag = 0,
		ES_DescrTag = 3,
		DecoderConfigDescrTag = 4,
		DecSpecificInfoTag = 5,
	};
	eType Type() {
		return m_type;
	}
	const AP4_UI08* Start() {
		return m_pBuffer + Header();
	}
	long Header() {
		return m_cHdr;
	}
	long Length() {
		return m_cBytes;
	}
	bool DescriptorAt(long cOffset, Descriptor& desc);

private:
	eType m_type;
	long m_cHdr;
	long m_cBytes;
	const AP4_UI08* m_pBuffer;
};

typedef struct  
{
	AP4_SI64 offset;
	AP4_SI64 dts;
	AP4_SI32 size;
	AP4_SI32 cto;
	AP4_SI32 sync;
}FRAG_SAMPLE;

#define MAX_TRACKS 8
class Movie;

class Track{
public:
	Track(Movie* movie, AP4_UI32 trackid);
	virtual ~Track();
	
	void SetType(AP4_Sample::Type type);
	AP4_Sample::Type GetType();
	AP4_UI32 GetTrackID();
	void SetTrackTimeScale(AP4_UI32 track_time_scale);
	AP4_UI32 GetTrackTimeScale();
	AP4_UI32 SetSamples(FRAG_SAMPLE* samples, AP4_UI32 samples_count);
	AP4_UI32& SamplesIndex();
	AP4_UI32 GetSamplesCount();
	FRAG_SAMPLE* GetEndSample();
	FRAG_SAMPLE* GetSample( AP4_UI32 samples_index);
	void SetFragStart(AP4_SI64 start){m_FragStart = start;}
	AP4_SI64 GetFragStart(AP4_SI64 start){return m_FragStart;}
private:
	Movie*				m_Movie;
	AP4_UI32			m_TrackID;
	AP4_Sample::Type	m_Type;
	AP4_UI32			m_TrackTimeScale;
	FRAG_SAMPLE*		m_Samples;
	AP4_UI32			m_SamplesCount;
	AP4_UI32			m_SamplesIndex;
	AP4_SI64			m_FragStart;
};

class Movie{
public:
	Movie(AP4_UI32 time_scale);
	virtual ~Movie();

	Track* GetTrack(AP4_UI32 trackid);
	Track* GetTrackByIndex(AP4_UI32 index);
	AP4_UI32 GetTrackCount();
	AP4_UI32 GetMovieTimeScale();
	bool AddTrack(AP4_UI32 trackid);
private:
	AP4_UI32	m_MovieTimeScale;
	Track*		m_Track[MAX_TRACKS];
	AP4_UI32	m_TrackCount;

};

class FMP4_File
{
public:
#define AP4_ATOM_TYPE(c1,c2,c3,c4)  \
	((((AP4_UI32)c1)<<24) |  \
	(((AP4_UI32)c2)<<16) |  \
	(((AP4_UI32)c3)<< 8) |  \
	(((AP4_UI32)c4)    ))

public:
	FMP4_File(AP4_ByteStream *input_stream);
	virtual ~FMP4_File();
	AP4_SampleDescription *GetSampleDescription(AP4_Sample::Type track_type);
	AP4_Result GetSample(AP4_Sample& sample);
	AP4_UI32 GetMediaTimeScale();
	bool HasVideo();
	bool HasAudio();
	bool ParseFileHeader();
	void GetSPSAndPPS(AP4_DataBuffer& sps, AP4_DataBuffer& pps);
	void SetSPSAndPPS(const AP4_DataBuffer& sps, const AP4_DataBuffer& pps);
	AP4_UI32 GetDuration();
	void GetRemain(AP4_UI08* pData, AP4_UI32& sizeData);
protected:

private:
	bool SeekToNextFrame();
	AP4_UI32 ParseAtomHeader(AP4_Position pos, AP4_UI64& atom_size, AP4_UI32& atom_type);
	AP4_Result ParseMoofAtom(AP4_Position moof_pos);
private:
	AP4_ByteStream*					m_inputStream;
	AP4_MpegAudioSampleDescription*	m_AudioDesc;
	AP4_AvcSampleDescription*		m_VideoDesc;
	bool							m_bAudio;
	bool							m_bVideo;
	bool							m_bParseHeader;
	AP4_SI64						m_StartTimeStamp;
	AP4_SI64						m_EndTimeStamp;
	AP4_UI08*						m_pRemainData;
	AP4_UI32						m_RemainSize;
	Movie*							m_Movie;
	bool							m_bNeedParseMoof;
	AP4_UI64						m_MoofPos;

	//<- modify by robert@2011.09.22
	//AP4_UI64                        m_TotalVideoTs;
	//AP4_UI64						m_TotalAudioSample;
};





#endif // _UCFFILE_H_