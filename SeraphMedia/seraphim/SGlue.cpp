extern"C"{
#include<stdio.h>
#include<stdint.h>
#include"pthread.h"
};
#include<iostream>
#include<map>
#include"STrackParam.h"
#include"context.h"
#include"SMp4Creater.h"
using namespace std;
using namespace Seraphim;

SEncodeContext *context=0;
uint8_t *g_PPS;
uint8_t *g_SPS;
size_t g_lenPPS;
size_t g_lenSPS;
#if(1)
/************************************************************************/
/*                                                                      */
/************************************************************************/
static void* task(void* param){
	int fileIndex = 0;
	char fileName[64]={0};
	sprintf(fileName,context->baseName,fileIndex++);
	//  wait  pps  sps
	do{
	}while(g_PPS == NULL || g_SPS==NULL);
	while(context->runing){
		char fileName[64]={0};
		sprintf(fileName,context->baseName,fileIndex++);
		SMp4Creater *creater = new SMp4Creater(fileName,context->idAndParm,context->idAndBuf);
		creater->addPPS(g_PPS,g_lenPPS,0);
		creater->addSPS(g_SPS,g_lenSPS,0);
		creater->startEncode();
		delete creater;
	}
	return 0;

}
void init(char* baseName,uint8_t countTrack,STrackParam* paramS){
	g_lenPPS=-1;
	g_lenSPS=-1;
	g_PPS = 0;
	g_SPS = 0;
	context = new SEncodeContext;
	//audioId = 1, videoId =  0;
	context->baseName = baseName;
	context->countTrack = countTrack;
	context->runing = true;
	for(int i =0;i<countTrack;i++){
		context->idAndBuf[i] = new SSyncBuffer;
		context->idAndNSample[i] = 0;
		context->idAndParm[i] =(STrackParam*)0;
	}
	pthread_t tid ;
	pthread_create(&tid,NULL,task,NULL);
}
void addSample(uint8_t trackId,uint8_t* sample,size_t len){

}


int main(int argc,char** argv){
	printf("-----seraphim--------\n");
	std::map<int,int> *m = new map<int,int>;
	(*m)[1] = 10;
	std::cout<<(*m)[1]<<std::endl;
	pthread_t tid ;
	pthread_create(&tid,NULL,task,NULL);
	int i;

	cin>>i;
	return 0;
}
#endif