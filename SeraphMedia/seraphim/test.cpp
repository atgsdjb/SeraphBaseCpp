//#include"s_config.h" 
//#include<cassert>
//#include<iostream>
//#include<vector>
//#include"pthread.h"
//#include"../Saac/SAudioMuxElement.h"
////
//#include"SMp4Creater.h"
//#include"STrackParam.h"
//#include"sync_buf.h"
//#include"SNaluHelp.h"
//#include"yuv420.h"
//extern"C"{
//#include"x264.h"
//#include"../aac/faac.h"
//};
//
//using namespace std;
//using namespace Seraphim;
//#define AVC_SAMPLE_BUFFER_SIZE  1024 *500
//
//static int g_index =0;
//
//static void initParam(x264_param_t* pX264Param,int width ,int height){
//	pX264Param->i_threads = X264_SYNC_LOOKAHEAD_AUTO;	//* 取空缓冲区继续使用不死锁的保证.//* video Properties
//	pX264Param->i_frame_total = 0;						//* 编码总帧数.不知道用0.
//	pX264Param->i_keyint_max = 10;	
//	pX264Param->i_bframe = 5;
//	pX264Param->b_open_gop = 0;
//	pX264Param->i_bframe_pyramid = 0;
//	pX264Param->i_bframe_adaptive = X264_B_ADAPT_TRELLIS;
//	pX264Param->b_annexb = 1;
//	pX264Param->i_log_level = X264_LOG_NONE;
//	pX264Param->rc.i_bitrate = 1024 * 500;				//* 码率(比特率,单位Kbps), muxing parameters
//	pX264Param->i_fps_den = 1;							//* 帧率分母
//	pX264Param->i_fps_num = 25;							//* 帧率分子
//	pX264Param->i_timebase_den = pX264Param->i_fps_num;
//	pX264Param->i_timebase_num = pX264Param->i_fps_den;
//	pX264Param->i_width = width;
//	pX264Param->i_height=height;
//}
//static void* encodeTask(void* _param){
//	SMp4Creater *handler =(SMp4Creater*)  _param ;
//	SyncBuffer* syncBuff = handler->getBuffer(0);
//	int iResult = 0;
//	int iNal;
//	x264_nal_t* pNals= new x264_nal_t;
//	FILE* pFile= fopen("D:\\1video\\seraphim.h264","wb");
//	//
//	FILE *lFile = fopen("D:\\1video\\seraphim.nalu","wb");
//	x264_t * pX264Handle = NULL;
//	x264_param_t *param= new x264_param_t;
//	x264_param_default(param);
//	initParam(param,352,288);
//	x264_param_apply_profile (param, x264_profile_names[1]);
//	pX264Handle = x264_encoder_open (param);
//	assert (pX264Handle);
//	iResult = x264_encoder_headers (pX264Handle, &pNals, &iNal);
//	int iMaxFrames = x264_encoder_maximum_delayed_frames (pX264Handle);
//	iNal = 0;
//	pNals = NULL;
//	x264_picture_t * pPicIn = new x264_picture_t;
//	x264_picture_t * pPicOut = new x264_picture_t;
//	x264_picture_init (pPicOut);
//	x264_picture_alloc (pPicIn, X264_CSP_I420, param->i_width,param->i_height);
//	pPicIn->img.i_csp = X264_CSP_I420;
//	pPicIn->img.i_plane = 3;
//	Yuv420 *yuv = new Yuv420(352,288,0,"D:\\1video\\test_w352_h288_f2000.yuv");
//	int iDataLen = param->i_width * param->i_height;
//	uint8_t *y = yuv->getY();
//	unsigned int  indexFrame=0;
//	unsigned int  indexNALU = 0;
//	//int lt_index = 0;
//	bool notPPS = true;
//	bool notSPS = true;
//	uint8_t *lAVCBuf = new uint8_t[AVC_SAMPLE_BUFFER_SIZE];
//	while(y!=NULL /*&& lt_index++ < 4*/){
//		pPicIn->img.plane[0]=y;
//		pPicIn->img.plane[1]=yuv->getU();
//		pPicIn->img.plane[2]=yuv->getV();
//		iResult = x264_encoder_encode(pX264Handle,&pNals,&iNal,pPicIn,pPicOut);
//		uint8_t* pps = NULL;
//		uint8_t* sps = NULL;
//		if(iResult<0){
//			cout<<"编码器错误"<<endl;
//			return NULL;
//		}else if(iResult ==0){
//			//cout<<"编码成功,但是被缓存"<<endl;
//			continue;
//		}else{
//			int l_postion = 0;
//			for (int i = 0; i < iNal; ++i)
//			{
//				uint8_t	 naluHead[4]={0};
//				uint32_t sizePayload = pNals[i].i_payload-3-pNals[i].b_long_startcode ;
//				uint32_t naluSize = sizePayload +4 ;
//				naluHead[0] = (sizePayload >> 24) & 0xff;
//				naluHead[1] = (sizePayload >> 16) & 0xff;
//				naluHead[2] = (sizePayload >> 8)  & 0xff;
//				naluHead[3] = sizePayload & 0xff;
//				fwrite (pNals[i].p_payload, 1, pNals[i].i_payload, pFile);
//				fflush(pFile);
//				if(notSPS || notPPS){
//					int len  = pNals[i].i_payload-3- pNals[i].b_long_startcode;
//					uint8_t  *l_bs = new uint8_t[len];
//					memcpy(l_bs,pNals[i].p_payload +3 +pNals[i].b_long_startcode ,len);
//					if(pNals[i].i_type == NAL_PPS){
//						notPPS = false;
//						handler->addPPS(l_bs,len,0);
//					}else if(pNals[i].i_type == NAL_SPS){
//						notSPS = false;
//						handler->addSPS(l_bs,len,0);
//					}
//				}
//				memcpy(lAVCBuf+l_postion,naluHead,4);
//				memcpy(lAVCBuf+l_postion+4,pNals[i].p_payload  + 3+ pNals[i].b_long_startcode,sizePayload);
//				l_postion+=naluSize;
//				//cout<<"编码成功,写入文件"<<"----------------------"<<indexNALU++<<"------NALU-size=-----"<<pNals[i].i_payload<<"------------"<<endl;
//			} 
//			//cout<<"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"<<g_index++<<"~~~~~~~~~~~~"<<"size="<<size<<"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"<<endl;
//			fwrite(lAVCBuf,l_postion,1,lFile);
//			fflush(lFile);
//			syncBuff->write23(lAVCBuf,l_postion);
//			//cout<<"编码成功,写入文件"<<indexFrame++<<endl;
//
//		}
//		y = yuv->getY();
//	}
//	fclose(pFile);
//	fclose(lFile);
//	syncBuff->disable();
//	return 0;
//
//}
///************************************************************************/
///*                                                                      */
///************************************************************************/
//void* aacEncodeThread(void* param){
//	FILE* aacFile = fopen("d:\\1video\\t4.aac","wb+");
//	FILE* pcmFile = fopen("d:\\1video\\t4.pcm","rb");
//	SMp4Creater *handler = (SMp4Creater*)param;
//	STrackParam *p = handler->getTrackParam(1);
//	if(p->getType()!=1){
//#ifdef SDEBUG
//		assert(0);
//#endif
//		exit(1);
//	}
//	SAudioTrackParam *audioParm = (SAudioTrackParam*)p; 
//		//QAudioTrackParam *audioP = (QAudioTrackParam*)trackParam;
//	unsigned long sampleRate = audioParm->sampleRate;
//	unsigned long bitRate = audioParm->bitRate;
//	int numChannels = 1;
//	unsigned long inputBuffSize;
//	unsigned long outBuffSize;
//
//	faacEncHandle aacHandler = faacEncOpen(sampleRate,numChannels,&inputBuffSize,&outBuffSize);
//	faacEncConfigurationPtr conf = faacEncGetCurrentConfiguration(aacHandler);
//	conf->bitRate = bitRate;
//	conf->inputFormat = FAAC_INPUT_16BIT;
//	//conf->channel_map =
//	conf->mpegVersion = MPEG4;
//	faacEncSetConfiguration(aacHandler,conf);
//	uint8_t* pcm=new uint8_t[1024*2];
//	SyncBuffer *aacBuffer = handler->getBuffer(1);
//	int len=-1;
//	uint8_t *l_sample=new uint8_t[1024*2];
//	size_t enc_len;
//	int lenRead=fread(pcm,1,2048,pcmFile);
//	do{
//		
//		/*int pcmOffset;
//		for(pcmOffset=0;pcmOffset<len;pcmOffset+=1024*2){*/
//			enc_len = faacEncEncode(aacHandler,(int32_t*)pcm,1024,l_sample,1024*2);
//			if(enc_len <=0){
//				continue;
//			}
//			uint8_t* l_b = new uint8_t[enc_len];
//			fwrite(l_sample,1,enc_len,aacFile);
//			fflush(aacFile);
//			memcpy(l_b,l_sample,enc_len);
//#ifdef SDEBUG
//#endif
//			aacBuffer->write23(l_b,enc_len);
//			lenRead = fread(pcm,1,2048,pcmFile);
//		//}
//	}while(lenRead==2048);
//	return 0;
//}
//
///************************************************************************/
///*                                                                      */
///************************************************************************/
//static uint8_t getMask(uint8_t num){
//	uint8_t i = 0;
//	uint8_t result = 0x00;
//	for(i;i<num;i++){
//		result >>=1;
//		result |= 0x80;
//	}
//	return result;
//}
//#include<cstdio>
//#include"../Saac/SGASpecificConfig.h"
//#include"SBitReader.h"
//int mainaaaaa(int argc,char* argv){
//	uint8_t buf[]={0x00,0xff,0x92,0x01,0x02};
//	SBitReader *reader = new SBitReader(buf,5);
//	uint8_t a;
//	uint16_t b ;
//	int len = 0;
//	len = reader->read(&a,4);
//	printf("----------%x------%d-------\n",a,len);
//	len = reader->read(&a,4);
//	printf("----------%x------%d-------\n",a,len);
//	len = reader->read(&a,4);
//	printf("----------%x------%d-------\n",a,len);
//	len = reader->read(&a,8);
//	printf("----------%x------%d-------\n",a,len);
//	//len = reader->read(&a,4);
//	//printf("----------%x------%d-------\n",a,len);
//	len = reader->read(&a,4);
//	printf("----------%x------%d-------\n",a,len);
//	len = reader->read(&a,8);
//	printf("----------%x-------%d------\n",a,len);
//	len = reader->read(&a,8);
//	printf("----------%x-------%d------\n",a,len);
//	//len = reader->read((uint8_t*)&b,16);
//	//printf("----------%x-----%d--------\n",b,len);
//	////delete reader;
//	int i ;
//	cin>>i;
//	return 0;
//}
//int main4(int argc,char* argv){
//	char* name ="d:\\1video\\seraphim2.mp4";
//	vector<SyncBuffer*> buf;
//	vector<STrackParam*> param;
//	SyncBuffer * gv_buf = new SyncBuffer;
//	STrackParam *v_param = new SVideoTrackParm(90000,352,288,1024 * 500,25,120*90000);
//	SyncBuffer *ga_buf = new SyncBuffer;
//	STrackParam *a_param = new SAudioTrackParam(44100,128*1024,44100,120 * 44100);
//	buf.push_back(ga_buf);
//	buf.push_back(gv_buf);
//	param.push_back(v_param);
//	param.push_back(a_param);
//	pthread_t tidVEncode;
//	pthread_t tidAEncode;
//	SMp4Creater creater(name,120,param,buf);
//	pthread_create(&tidVEncode,NULL,encodeTask,&creater);
//	pthread_create(&tidAEncode,NULL,aacEncodeThread,(void*)&creater);
//	creater.startEncode();
//	//pthread_join(tidAEncode,NULL);
//	//pthread_join(tidVEncode,NULL);
//
//	int i;
//	//cin>>i;
//return 0;
//}