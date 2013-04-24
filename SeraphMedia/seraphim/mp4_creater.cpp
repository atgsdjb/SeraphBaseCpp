#include"mp4_creater.h"
#ifdef DEBUG
#include<iostream>
#include<cassert>
#endif

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
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	SMp4Creater::SMp4Creater(const char* _name,uint32_t _duration,const vector<STrackParam*>&_trackParam,const vector<SyncBuffer*>& _trackBufS,bool _isAsyn,CompleteListener _listener)
	:name(_name),duration(_duration),listener(_listener),isAsyn(_isAsyn){
		int i = 0;
		assert(_trackParam.size() == _trackBufS.size());
		trackCount = _trackParam.size();
		for(i;i<trackCount;i++){
			trackS[i] = MP4_INVALID_TRACK_ID;
			trackBufS[i] = _trackBufS[i];
			trackParamS[i] = _trackParam[i];
		}
	}


	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	SMp4Creater::SMp4Creater(const char* _name,uint32_t _duration,uint8_t _trackCount,STrackParam* _trackParam,SyncBuffer* _trackBufS,bool _isAsyn,CompleteListener _listener)
		:name(_name),duration(_duration),listener(_listener),isAsyn(_isAsyn){
		int i;
		for (i = 0;i<trackCount;i++){
			trackS[i] = MP4_INVALID_TRACK_ID;
			trackBufS[i] = &(_trackBufS[i]);
			trackParamS[i] = &(_trackParam[i]);
		}
		initTracks();
	}
	void SMp4Creater::addSample8(uint8_t *sample,size_t size,uint8_t trackIndex){
		//trackBufS[trackIndex].write23(sample,size);
	}
	void SMp4Creater::addSample16(uint16_t*sample,size_t size,uint8_t trackIndex){

	}
	void SMp4Creater::startEncode(){
		if(isAsyn){
			pthread_t tid;
			pthread_create(&tid,0,encode_task,this);
		}else{
			encodeLoop();
		}
		
	}

	MP4TrackId SMp4Creater::createAudioTrack(STrackParam * param){
		SAudioTrackParam *p =(SAudioTrackParam*)param;
		uint32_t timeScale = 0;
		MP4Duration sampleDuration;
		return MP4AddAudioTrack(file,timeScale,sampleDuration);
	}
	MP4TrackId SMp4Creater::createVideoTrack(STrackParam* param){
		SVideoTrackParm *p =(SVideoTrackParm*)param;
		uint32_t timeScale=0;
		MP4Duration sampleDuration = 0;
		uint16_t width =0;
		uint16_t height = 0;
		return MP4AddVideoTrack(file,timeScale,sampleDuration,width,height);
	}

	void SMp4Creater::initTracks(){
		file = MP4CreateEx(name, MP4_DETAILS_ALL, 0, 1, 1, 0, 0, 0, 0);//创建mp4文件
		assert(file !=MP4_INVALID_FILE_HANDLE);
		MP4SetTimeScale(file,90000);
		MP4SetVideoProfileLevel(file, 0x7F);
		MP4SetAudioProfileLevel(file,0x2);
		int i = 0;
		for(i;i<trackCount;i++){
			int type = trackParamS[i]->type;
			MP4TrackId id  = MP4_INVALID_TRACK_ID;
			if(type == 0){
				id = createVideoTrack(trackParamS[i]);
			}else if(type == 1){
				id = createAudioTrack(trackParamS[i]);
			}else{
				assert(0);
			}
			trackS[i] = id;
			trackCompleteS[i]=false;
			trackDurationS[i] = duration*trackParamS[i]->timeScale;
			trackTimesTampS[i] = 0;
		}
	}
	void SMp4Creater::encodeLoop(){
		int i = 0;
		uint8_t* sample;
		while(comlete()){
			for(i;i<trackCount;i++){
				if(trackCompleteS[i])
					continue;
				int len = trackBufS[i]->read(&sample);
				if(len==0)
					continue;
				if(len == -1){
					trackCompleteS[i]=true;
					continue;
				}
				int type = trackParamS[i]->type;
				if(type==0){
					if(trackTimesTampS[i] > trackDurationS[i]){   //保证没一video track  从SPS PPS开始
						if(sample[4] == 0x65){
							trackBufS[i]->writeBack(sample,len);
							trackCompleteS[i]=true;
							continue;
						}
					}else{									     //正常写入A FRAME
						//mp
						MP4WriteSample(file,trackS[i],sample,len);
						//没有区分非视频数据
						trackTimesTampS[i] += ((SVideoTrackParm*)trackParamS[i])->durationPreFrame; 

					}
				}else if(type == 1){  
					trackTimesTampS[i]+=((SAudioTrackParam*)trackParamS[i])->durationPreFrame;
					trackCompleteS[i] = trackTimesTampS[i] >= trackDurationS[i]; 
				}
				MP4WriteSample(file,trackS[i],sample,len);


			}
		}
		MP4Close(file);
		if(isAsyn && listener != 0)
			listener();

		
	}
	bool SMp4Creater::comlete(){
		map<int,bool>::iterator i ;// trackCompleteS.begin();
		for(i=trackCompleteS.begin();i!=trackCompleteS.end();i++){
			if(!i->second)
				return false;
		}
		return true;
	}
	
};