#include <iostream>
#include <string>
#include"yuv420.h"
using namespace std;
using namespace Seraphim;
#include"sync_buf.h"
#include"../mp4/mp4.h"

#include<list>
extern "C"
{
#include<stdio.h>
#include"../aac/faac.h"
#include<stdint.h>
#include "x264.h"
};
static void initParam(x264_param_t* pX264Param,int width ,int height){
	pX264Param->i_threads = X264_SYNC_LOOKAHEAD_AUTO;	//* 取空缓冲区继续使用不死锁的保证.//* video Properties
	pX264Param->i_frame_total = 0;						//* 编码总帧数.不知道用0.
	pX264Param->i_keyint_max = 10;	
	pX264Param->i_bframe = 5;
	pX264Param->b_open_gop = 0;
	pX264Param->i_bframe_pyramid = 0;
	pX264Param->i_bframe_adaptive = X264_B_ADAPT_TRELLIS;
	pX264Param->b_annexb = 1;
	pX264Param->i_log_level = X264_LOG_DEBUG;
	pX264Param->rc.i_bitrate = 1024 * 10;				//* 码率(比特率,单位Kbps), muxing parameters
	pX264Param->i_fps_den = 1;							//* 帧率分母
	pX264Param->i_fps_num = 25;							//* 帧率分子
	pX264Param->i_timebase_den = pX264Param->i_fps_num;
	pX264Param->i_timebase_num = pX264Param->i_fps_den;
	pX264Param->i_width = width;
	pX264Param->i_height=height;
}
//static  int  encode(){
//	int iResult = 0;
//	int iNal;
//	x264_nal_t* pNals= new x264_nal_t;
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
//	MP4FileHandle file = MP4CreateEx("D:\\1video\\test_w352_h288_f2000a.mp4", MP4_DETAILS_ALL, 0, 1, 1, 0, 0, 0, 0);//创建mp4文件
//	assert(file !=MP4_INVALID_FILE_HANDLE);
//	MP4SetTimeScale(file,90000);
//	MP4TrackId video = MP4AddH264VideoTrack(file,90000,90000/25,176,144,0x64,0x00,0x1f,3);
//	assert(video !=MP4_INVALID_TRACK_ID);
//    MP4SetVideoProfileLevel(file, 0x7F);
//	Yuv420 *yuv = new Yuv420(352,288,0,"D:\\1video\\test_w352_h288_f2000.yuv");
//	int iDataLen = param->i_width * param->i_height;
//	uint8_t *y = yuv->getY();
//	uint8_t* sampleBuffer = new uint8_t[1024*1024*100];
//	unsigned int  indexFrame=0;
//	unsigned int  indexNALU = 0;
//	while(y!=NULL){
//		pPicIn->img.plane[0]=y;
//		pPicIn->img.plane[1]=yuv->getU();
//		pPicIn->img.plane[2]=yuv->getV();
//		iResult = x264_encoder_encode(pX264Handle,&pNals,&iNal,pPicIn,pPicOut);
//		if(iResult<0){
//			cout<<"编码器错误"<<endl;
//			return -1;
//		}else if(iResult =0){
//			//cout<<"编码成功,但是被缓存"<<endl;
//		}else{
//			int l_postion = 0;
//			for (int i = 0; i < iNal; ++i)
//			{
//				//fwrite (pNals[i].p_payload, 1, pNals[i].i_payload, pFile);
//				memcpy(sampleBuffer+l_postion,pNals[i].p_payload,pNals[i].i_payload);
//				l_postion+=pNals[i].i_payload;
//			cout<<"编码成功,写入文件"<<"----------------------"<<indexNALU++<<"------NALU-size=-----"<<pNals[i].i_payload<<"------------"<<endl;
//			} 
//			MP4WriteSample(file,video,sampleBuffer,l_postion);
//			cout<<"编码成功,写入文件"<<indexFrame++<<endl;
//
//		}
//		y = yuv->getY();
//	}
//	MP4Close(file);
//	//fflush(pFile);
//	//fclose(pFile);
//	return 0;
//}
static void testEncoder(){
	//int width  = 352;
	//int height = 288;
	//int sizePreFrame = width*height*3/2;
	//int startU = width * height;
	//int startV = width*height*5/4;
	//uint8_t* buffer = new uint8_t[sizePreFrame*10];
	//FILE  *file = fopen("D:\\1video\\test_w352_h288_f2000.yuv","rb");
	////FILE  *pFile = fopen("D:\\1video\\test_w352_h288_f2000.h264","wb+");
	//assert(file);
	////assert(pFile);
	//fread(buffer,1,sizePreFrame*10,file);

	//int iResult = 0;
	//int iNal;
	//x264_nal_t* pNals= new x264_nal_t;
	//x264_t * pX264Handle = NULL;
	//x264_param_t *param= new x264_param_t;
	//x264_param_default(param);
	//initParam(param,352,288);
	//x264_param_apply_profile (param, x264_profile_names[1]);
	//pX264Handle = x264_encoder_open (param);
	//assert (pX264Handle);
	//iResult = x264_encoder_headers (pX264Handle, &pNals, &iNal);
	//int iMaxFrames = x264_encoder_maximum_delayed_frames (pX264Handle);
	//iNal = 0;
	//pNals = NULL;
	//x264_picture_t * pPicIn = new x264_picture_t;
	//x264_picture_t * pPicOut = new x264_picture_t;
	//x264_picture_init (pPicOut);
	//x264_picture_alloc (pPicIn, X264_CSP_I420, param->i_width,param->i_height);
	//pPicIn->img.i_csp = X264_CSP_I420;
	//pPicIn->img.i_plane = 3;
	//unsigned int  index=0;
	//while(index<300){
	//	index++;
	//	uint8_t * start = buffer+ sizePreFrame*(index%10);
	//	pPicIn->img.plane[0]=start;
	//	pPicIn->img.plane[1]=start+startU;
	//	pPicIn->img.plane[2]=start+startV;
	//	iResult = x264_encoder_encode(pX264Handle,&pNals,&iNal,pPicIn,pPicOut);
	//	if(iResult<0){
	//		return ;
	//	}else if(iResult =0){
	//	}else{
	//		int l_postion = 0;
	//		for (int i = 0; i < iNal; ++i)
	//		{
	//			//fwrite (pNals[i].p_payload, 1, pNals[i].i_payload, pFile);
	//		} 

	//	}

	//}
	//fflush(pFile);
	//fclose(pFile);
	//fclose(file);
};
uint8_t g_buffer[]={
	//7
	0x00,0x00,0x00,0x01,0xa2,0x23,0x12,
	//20----O7
	0x00,0x00,0x00,0x01,0x25,0x35,0x89,0x25,0x00,0x00,0x00,0x10,0x02,0x12,0x00,0x12,0x00,0x00,0x23,0x0f,
	//11-O27
	0x00,0x00,0x00,0x01,0x23,0x54,0x12,0x59,0x88,0x89,0x00,
	//26-O38
	0x00,0x00,0x00,0x01,0x85,0x55,0x66,0x48,0x22,0x11,0x89,0x00,0x12,0x00,0x12,0x00,0x00,0x00,0x10,0x02,0x12,0x00,0x12,0x00,0x00,0x13,
	//5-O64
	0x00,0x00,0x00,0x01,0x56

};
/**
从缓冲区中COPY 1个NAU

**/
//                                        0   1  2  3 
//static const  unsigned int NALU_HEAD=0X 00 00 00 01;
static  int  readHALU(uint8_t* srcBuffer,uint8_t* &distBuffer,unsigned int offset,unsigned int sizeBuffer ){
	int len =4;// 4;
	uint8_t *pI = srcBuffer+offset;
	uint8_t  data;
	while(sizeBuffer >(len+offset)){
		/*if(*(pI+len) != 0x00){
			len++;
			continue;
		*/
		data = *(pI+len);
	   if(*(pI+len)==0x00){
		   data = *(pI+len+1);
		if(*(pI+len+1)==0x00){
			data = *(pI+len+2);
			if(*(pI+len+2)==0x00){
				data = *(pI+len+3);
				if(*(pI+len+3)==0x01){
					break;
				}else{
					data = *(pI+len+3);
					len +=4;
				}
			}else{
				data = *(pI+len+2);
				len +=3;
			}
		}else{
			data = *(pI+len+1);		
			len +=2;
		}
	   }else{
		   data = *pI;
		   len++;
	   }
	}
	distBuffer = new uint8_t[len];
	memcpy(distBuffer,pI,len);
	return len;
}
static void createMp4FormH264(char* h264Name,char* mp4File,int countFrame,int width,int ){
     MP4FileHandle file = MP4CreateEx(mp4File, MP4_DETAILS_ALL, 0, 1, 1, 0, 0, 0, 0);//创建mp4文件
	 assert(file !=MP4_INVALID_FILE_HANDLE);
	 MP4SetTimeScale(file,90000);
	 MP4TrackId video = MP4AddH264VideoTrack(file,90000,90000/25,176,144,0x64,0x00,0x1f,3);
	 assert(video !=MP4_INVALID_TRACK_ID);
     MP4SetVideoProfileLevel(file, 0x7F);
	 FILE *h264File = fopen(h264Name,"rb");
	 uint8_t* buffer = new uint8_t[1024*1024*25];
	 size_t  sizeBuffer =fread(buffer,1,1024*1024,h264File);
	 cout<<"read data size="<<sizeBuffer;
	 uint8_t *dp;
	 int offset = 0;
	 int len = 0;
	 int  indexFrame = 0;
	 while(offset<sizeBuffer){
		 len = readHALU(buffer,dp,offset,sizeBuffer);
		 cout<<"------indexFrame=-----"<<indexFrame++<<"----len-------"<<len<<"len"<<endl;
			MP4WriteSample(file,video,dp,len);
			delete[] dp;
			offset += len;
		 offset +=len;
	 }
	 MP4Close(file);
  };

static  void*  encode(SyncBuffer* _param){
		SyncBuffer* syncBuff = _param;
		int iResult = 0;
		int iNal;
		x264_nal_t* pNals= new x264_nal_t;
		FILE* pFile= fopen("D:\\1video\\temp.h264","wb");
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
		while(y!=NULL /*&& lt_index++ < 4*/){
			pPicIn->img.plane[0]=y;
			pPicIn->img.plane[1]=yuv->getU();
			pPicIn->img.plane[2]=yuv->getV();
			iResult = x264_encoder_encode(pX264Handle,&pNals,&iNal,pPicIn,pPicOut);
			if(iResult<0){
				cout<<"编码器错误"<<endl;
				return NULL;
			}else if(iResult =0){
				//cout<<"编码成功,但是被缓存"<<endl;
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
					fwrite (pNals[i].p_payload, 1, pNals[i].i_payload, pFile);
					fflush(pFile);
					memcpy(sampleBuffer+l_postion,pNals[i].p_payload,pNals[i].i_payload);
					l_postion+=pNals[i].i_payload;
				//cout<<"编码成功,写入文件"<<"----------------------"<<indexNALU++<<"------NALU-size=-----"<<pNals[i].i_payload<<"------------"<<endl;
				} 
				syncBuff->write23(sampleBuffer,size);
				//cout<<"编码成功,写入文件"<<indexFrame++<<endl;
	
			}
			y = yuv->getY();
		}
		fclose(pFile);
		syncBuff->disable();
		return 0;
}
typedef struct _T{
	int index;
	char* name;
}T;
char name_list[][3]={
	"1d","2b","3c"
};
void test(){
	FILE* file = fopen("D:\\1video\\seraphim00001.mp4","rb");
	uint8_t t_buf[128]={0};
	size_t len = fread(t_buf,1,128,file);
	cout<<"read len "<<len<<endl;
	uint8_t type= t_buf[4];
	cout<<"type"<<type<<endl;
	if(type==0x67)
		cout<<"PPS"<<endl;
	else
		cout<<"not!!1"<<endl;
}
typedef struct {
	SyncBuffer* buff;
	const char* mp4BaseName;
	int framePreFile;
}ThreadParam;
void* create_mp4(void* param){
	ThreadParam *p = (ThreadParam *)param;
	SyncBuffer *syncBuff = p->buff;
	const int sizeFile = p->framePreFile;
	const char* baseName = p->mp4BaseName;
	char name[256]={0};
	uint8_t* frame = NULL;
	int indexFile = 1;
	int len = -1;//读到的数据大小,-1为结束标志
	while(true){
		int indexFramed = 0;
		memset(name,0,256);
		sprintf(name,baseName,indexFile++);
		MP4FileHandle file = MP4CreateEx(name, MP4_DETAILS_ALL, 0, 1, 1, 0, 0, 0, 0);//创建mp4文件
		assert(file !=MP4_INVALID_FILE_HANDLE);
		MP4SetTimeScale(file,90000);
		MP4TrackId video = MP4AddH264VideoTrack(file,90000,90000/25,352,288,0x64,0x00,0x1f,3);
		assert(video !=MP4_INVALID_TRACK_ID);
		MP4SetVideoProfileLevel(file, 0x7F);
		while(((len = syncBuff->read(&frame)) != -1) && (indexFramed++ < sizeFile)){
			MP4WriteSample(file,video,frame,len);
			delete frame;
		};
		MP4Close(file);
		//cout<<"compile a file-----"<<endl;
		if(len == -1)
			break;
	};
	//cout<<"-----finish----"<<endl;
	return NULL;
}


void test_sync_buf(){
	ThreadParam *param = new ThreadParam;
	SyncBuffer *syncBuff = new SyncBuffer;
	param->buff = syncBuff;
	param->framePreFile = 200;
	param->mp4BaseName="D:\\1video\\se%04d.mp4";
	pthread_t tid;
	pthread_create(&tid,NULL,create_mp4,(void*)param);
	encode(syncBuff);
	pthread_join(tid,NULL);
	delete syncBuff;
	delete param;
}
typedef struct seraphim{
	int f;
	int s;
}seraphim,*seraphimP;
int main34()
{	int result ;
	seraphim s;
	s.f =1;
	s.s = 2;
	seraphimP sp = new seraphim;
	sp->s=1;
	sp->f =2;
	cout<<"-----------equ-----"<<strcmp("seraph","seraph")<<endl;
	cout<<"-----not---equ-----"<<strcmp("1seraph","seraph")<<endl;
	int js;
	cin >>js;
	cout<<sp->s<<endl;
	cout<<sp->f<<endl;
	return result;

}
#include"../mp4/mp4common.h"
#include<vector>
class F{
public:
	F(int _old):old(_old){};
	int old;
};


typedef struct {
	int count;
	vector<F*> *people;
}HandlerType;
HandlerType* handler;
/************************************************************************/
/*                                                                      */
/************************************************************************/
void put_bits(uint8_t* buf,size_t size,uint32_t data){

}
//int adts_write_frame_header(uint8_t *sample, int size, int pce_size,uint8_t profile_objecttype,)
//{
//	uint8_t pb;
//	//PutBitContext pb;
//
//	//init_put_bits(&pb, buf, ADTS_HEADER_SIZE);
//
//	/* adts_fixed_header */
//	put_bits(sample, 12, 0xfff);   /* syncword */
//	put_bits(sample, 1, 0);        /* ID */
//	put_bits(sample, 2, 0);        /* layer */
//	put_bits(sample, 1, 1);        /* protection_absent */
//	put_bits(sample, 2, ctx->objecttype); /* profile_objecttype */
//	put_bits(sample, 4, ctx->sample_rate_index);
//	put_bits(sample, 1, 0);        /* private_bit */
//	put_bits(sample, 3, ctx->channel_conf); /* channel_configuration */
//	put_bits(sample, 1, 0);        /* original_copy */
//	put_bits(sample, 1, 0);        /* home */
//
//	/* adts_variable_header */
//	put_bits(sample, 1, 0);        /* copyright_identification_bit */
//	put_bits(sample, 1, 0);        /* copyright_identification_start */
//	put_bits(sample, 13, ADTS_HEADER_SIZE + size + pce_size); /* aac_frame_length */
//	put_bits(sample, 11, 0x7ff);   /* adts_buffer_fullness */
//	put_bits(sample, 2, 0);        /* number_of_raw_data_blocks_in_frame */
//
//	//flush_put_bits(&pb);
//
//	return 0;
//}
/************************************************************************/
/*                                                                      */
/************************************************************************/
#include"aac_adts.h"
uint8_t g_data[]={0xaa,0x55,0xaa,0x55,0xaa,0x55,0xaa,0x55,0xaa,0x55,0xaa,0x55,0xaa,0x55,0xaa,0x55,0xaa,0x55,0xaa,0x55,0xaa,0x55,
				  0xaa,0x55,0xaa,0x55,0xaa,0x55,0xaa,0x55,0xaa,0x55,0xaa,0x55};
void testADTS(){
	uint8_t *d = new uint8_t[256];
		size_t len = sizeof(g_data);
	AdtsHelp* h=new AdtsHelp();
	FILE *f = fopen("d:\\1video\\test23.aac","wb+");

	assert(f);

	int i =0;
	uint8_t* t_d;	
	 h->adts_write_frame_header(&t_d,d,len);
	for(i;i<1024*1024;i++){
		fwrite(t_d,1,len+ADTS_HEADER_SIZE,f);
	}
	fflush(f);
	fclose(f);
}
#include<map>
class SS{
private: int id ;
		 map<int,int> m;
public :
	SS(int _id):id(_id){

	};
	SS(){id=123;};
	int getId(){return id;};
	void inster(int id,int v){m[id]=v;};
	int  get(int id){return m[id];};
};

#include"mp4_creater.h"
int main23(int argc, char ** argv){
	//
	//SMp4Creater creater(0,0,2,0,0);
	//creater.startEncode();

	//SS ss(123);
	//ss.inster(1,100);
	//ss.inster(2,200);
	//cout<<ss.get(1)<<endl;
	//cout<<ss.get(2)<<endl;
	//int k;
	//cin>>k;
	return 0;

}