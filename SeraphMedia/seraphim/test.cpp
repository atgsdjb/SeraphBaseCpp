#include"s_config.h" 
#include<cassert>
#include<iostream>
#include<vector>
#include"pthread.h"
//
#include"mp4_creater.h"
#include"track_param.h"
#include"sync_buf.h"
#include"nalu_help.h"
#include"yuv420.h"
extern"C"{
#include"x264.h"
};

using namespace std;
using namespace Seraphim;

static int g_index =0;

static void initParam(x264_param_t* pX264Param,int width ,int height){
	pX264Param->i_threads = X264_SYNC_LOOKAHEAD_AUTO;	//* 取空缓冲区继续使用不死锁的保证.//* video Properties
	pX264Param->i_frame_total = 0;						//* 编码总帧数.不知道用0.
	pX264Param->i_keyint_max = 10;	
	pX264Param->i_bframe = 5;
	pX264Param->b_open_gop = 0;
	pX264Param->i_bframe_pyramid = 0;
	pX264Param->i_bframe_adaptive = X264_B_ADAPT_TRELLIS;
	pX264Param->b_annexb = 1;
	pX264Param->i_log_level = X264_LOG_NONE;
	pX264Param->rc.i_bitrate = 1024 * 500;				//* 码率(比特率,单位Kbps), muxing parameters
	pX264Param->i_fps_den = 1;							//* 帧率分母
	pX264Param->i_fps_num = 25;							//* 帧率分子
	pX264Param->i_timebase_den = pX264Param->i_fps_num;
	pX264Param->i_timebase_num = pX264Param->i_fps_den;
	pX264Param->i_width = width;
	pX264Param->i_height=height;
}
static void* encodeTask(void* _param){
	SMp4Creater *handler =(SMp4Creater*)  _param ;
	SyncBuffer* syncBuff = handler->getBuffer(0);
	int iResult = 0;
	int iNal;
	x264_nal_t* pNals= new x264_nal_t;
	//FILE* pFile= fopen("D:\\1video\\seraphim.h264","wb");
	x264_t * pX264Handle = NULL;
	x264_param_t *param= new x264_param_t;
	x264_param_default(param);
	initParam(param,352,288);
	x264_param_apply_profile (param, x264_profile_names[1]);
	pX264Handle = x264_encoder_open (param);
	assert (pX264Handle);
	iResult = x264_encoder_headers (pX264Handle, &pNals, &iNal);
	int iMaxFrames = x264_encoder_maximum_delayed_frames (pX264Handle);
	iNal = 0;
	pNals = NULL;
	x264_picture_t * pPicIn = new x264_picture_t;
	x264_picture_t * pPicOut = new x264_picture_t;
	x264_picture_init (pPicOut);
	x264_picture_alloc (pPicIn, X264_CSP_I420, param->i_width,param->i_height);
	pPicIn->img.i_csp = X264_CSP_I420;
	pPicIn->img.i_plane = 3;
	Yuv420 *yuv = new Yuv420(352,288,0,"D:\\1video\\test_w352_h288_f2000.yuv");
	int iDataLen = param->i_width * param->i_height;
	uint8_t *y = yuv->getY();
	uint8_t* sampleBuffer;// = new uint8_t[1024*1024*100];
	unsigned int  indexFrame=0;
	unsigned int  indexNALU = 0;
	//int lt_index = 0;
	bool notPPS = true;
	bool notSPS = true;
	while(y!=NULL /*&& lt_index++ < 4*/){
		pPicIn->img.plane[0]=y;
		pPicIn->img.plane[1]=yuv->getU();
		pPicIn->img.plane[2]=yuv->getV();
		iResult = x264_encoder_encode(pX264Handle,&pNals,&iNal,pPicIn,pPicOut);
		uint8_t* pps = NULL;
		uint8_t* sps = NULL;
		if(iResult<0){
			cout<<"编码器错误"<<endl;
			return NULL;
		}else if(iResult ==0){
			//cout<<"编码成功,但是被缓存"<<endl;
			continue;
		}else{
			int l_postion = 0;
			size_t size = 0;
		for(int j = 0;j<iNal;++j)
				size+= pNals[j].i_payload;
			sampleBuffer = NULL;
			sampleBuffer = new uint8_t[size];
			assert(sampleBuffer);
			for (int i = 0; i < iNal; ++i)
			{
				/*fwrite (pNals[i].p_payload, 1, pNals[i].i_payload, pFile);
				fflush(pFile);*/
				if(notSPS || notPPS){
					int len  = pNals[i].i_payload;//- pNals[i].b_long_startcode;
					uint8_t  *l_bs = new uint8_t[len];
					memcpy(l_bs,pNals[i].p_payload /* +pNals[i].b_long_startcode */,len);
					if(pNals[i].i_type == NAL_PPS){
						notPPS = false;
						handler->addPPS(pps,len,0);
					}else if(pNals[i].i_type == NAL_SPS){
						notSPS = false;
						handler->addSPS(sps,len,0);
					}
				}
				memcpy(sampleBuffer+l_postion,pNals[i].p_payload,pNals[i].i_payload);
				l_postion+=pNals[i].i_payload;
				//cout<<"编码成功,写入文件"<<"----------------------"<<indexNALU++<<"------NALU-size=-----"<<pNals[i].i_payload<<"------------"<<endl;
			} 
			cout<<"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"<<g_index++<<"~~~~~~~~~~~~"<<"size="<<size<<"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"<<endl;
			syncBuff->write23(sampleBuffer,size);
			//cout<<"编码成功,写入文件"<<indexFrame++<<endl;

		}
		y = yuv->getY();
	}
	//fclose(pFile);
	syncBuff->disable();
	return 0;

}



int main(int argc,char* argv){
	char* name ="d:\\1video\\seraphim2.mp4";
	vector<SyncBuffer*> buf_v;
	vector<STrackParam*> param_v;
	SyncBuffer * g_buf = new SyncBuffer;
	STrackParam *param = new SVideoTrackParm(90000,352,288,1024 * 500,15,30);
	buf_v.push_back(g_buf);
	param_v.push_back(param);
	pthread_t tid;

	//uint8_t* fristSample;

	//int len = -1;
	//do{
	//	len = g_buf->read(&fristSample);
	//}while(len==0);
	//vector<uint8_t*> naluS;
	//vector<int> naluC;
	//int count = getNaluS(fristSample,len,NALU4H,naluS,naluC);
#ifdef SDEBUG
	//cout<<count<<endl;
#endif
	//uint8_t* pps;
	//int lenPps = -1;
	//uint8_t* sps;
	//int lenSps= -1;
	//for(int i = 0;i<count;i++){
	//	if(isSPS(naluS[i],NALU4H)){
	//		sps = naluS[i];
	//		lenSps = naluC[i];
	//	}else if(isPPS(naluS[i],NALU4H)){
	//		pps = naluS[i];
	//		lenPps =naluC[i];
	//	}else{
	//		delete naluS[i];
	//	}
	//
	//}
#ifdef SDEBUG
	//cout<<"pps len ="<<lenPps<<endl;
	//cout<<"sps len ="<<lenSps<<endl;
#endif



	SMp4Creater creater(name,30,param_v,buf_v);
		pthread_create(&tid,NULL,encodeTask,&creater);
	creater.startEncode();
	pthread_join(tid,NULL);

	int i;
	cin>>i;
return 0;
}