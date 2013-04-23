#ifndef _SERAPHIM_MP4_CREATER_H
#define _SERAPHIM_MP4_CREATER_H
#include<cstdint>
#include"sync_buf.h"
#include"track_param.h"
#include"pthread.h"
#include"../mp4/mp4.h"
#include<map>
//class STrackParam;
//class SAudioTrackParam;
//class SVideoTrackParm;
//class SyncBuffer;
typedef void(*CompleteListener)(void);
using namespace std;


namespace Seraphim{
class SMp4Creater{
private:
	char		*name;
	uint32_t	duration;//sec
	uint8_t		trackCount;
	map<int,int>	trackS;
	map<int,STrackParam*> trackParamS;
	map<int,SyncBuffer*>  trackBufS;
	CompleteListener listener;
public:
	SMp4Creater(char* _name,uint32_t _duration,uint8_t _trackCount,STrackParam *_trackParam,CompleteListener _listener=0);
	void addSample8(uint8_t *sample,size_t size,uint8_t trackIndex);
	void addSample16(uint16_t* sample,size_t size,uint8_t trackIndex);
	void startEncode();
	friend void* encode_task(void*);

};
};
#endif