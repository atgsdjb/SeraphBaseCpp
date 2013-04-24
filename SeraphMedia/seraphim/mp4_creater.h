#ifndef _SERAPHIM_MP4_CREATER_H
#define _SERAPHIM_MP4_CREATER_H

#define DEBUG


#include<cstdint>
#include"sync_buf.h"
#include"track_param.h"
#include"pthread.h"
#include"../mp4/mp4.h"
#include<map>
#include<vector>
//class STrackParam;
//class SAudioTrackParam;
//class SVideoTrackParm;
//class SyncBuffer;
typedef void(*CompleteListener)(void);
using namespace std;


namespace Seraphim{
class SMp4Creater{
private:
	const char		*name;
	uint32_t	duration;//sec
	uint8_t		trackCount;
	map<int,int>	trackS;
	map<int,STrackParam*> trackParamS;
	map<int,SyncBuffer*>  trackBufS;
	map<int,bool> trackCompleteS;
	map<int,MP4Duration> trackDurationS;
	map<int,MP4Duration> trackTimesTampS;

	bool isAsyn;
	CompleteListener listener;
	MP4FileHandle file;
	MP4TrackId createAudioTrack(STrackParam* param);
	MP4TrackId createVideoTrack(STrackParam* param);
	bool comlete();
	void initTracks();
	void encodeLoop();
public:
	SMp4Creater(const char* _name,uint32_t _duration,uint8_t _trackCount,STrackParam *_trackParam,SyncBuffer* _trackBufS, bool _isAsyn=false, CompleteListener _listener=0);
	SMp4Creater(const char* _name,uint32_t _duration,const vector<STrackParam*>& _trackParam,const vector<SyncBuffer*>& _trackBufS,bool _isAsyn=false,CompleteListener _listener=0);	
	void addSample8(uint8_t *sample,size_t size,uint8_t trackIndex);
	void addSample16(uint16_t* sample,size_t size,uint8_t trackIndex);
	void startEncode();
	friend void* encode_task(void*);

};
};
#endif