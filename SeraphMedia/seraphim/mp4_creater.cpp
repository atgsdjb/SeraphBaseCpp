
#include"mp4_creater.h"
/**test**/
#include<iostream>

/**test**/

namespace Seraphim{
	static void* encode_task(void *handle){
		SMp4Creater *creater = (SMp4Creater*)handle;
		int i =0;
		for(;i<19;i++){
			cout<<i<<endl;
		}
		return 0;
	}


							   //char* _name,uint32_t _duration,uint8_t _trackCount,STrackParam *_trackParam,CompleteListener _listener
	SMp4Creater::SMp4Creater(char* _name,uint32_t _duration,uint8_t _trackCount,STrackParam* _trackParam,CompleteListener _listener)/*:name(_name),duration(_duration),listener(_listener)*/{
		int i;
		int trackCount = 12;
		for (i = 0;i<trackCount;i++){
			trackS[i] = MP4_INVALID_TRACK_ID;
			trackBufS[i] = new SyncBuffer;
			trackParamS[i] = &(_trackParam[i]);
		}

	}
	void SMp4Creater::addSample8(uint8_t *sample,size_t size,uint8_t trackIndex){
		//trackBufS[trackIndex].write23(sample,size);
	}
	void SMp4Creater::addSample16(uint16_t*sample,size_t size,uint8_t trackIndex){

	}
	void SMp4Creater::startEncode(){
		pthread_t tid;
		pthread_create(&tid,0,encode_task,this);
	}
	
};