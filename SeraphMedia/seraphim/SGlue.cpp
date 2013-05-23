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

SEncoderContext *context=0;
uint8_t *g_PPS;
uint8_t *g_SPS;
size_t g_lenPPS;
size_t g_lenSPS;

/************************************************************************/
/*                                                                      */
/************************************************************************/


void init(char* baseName,uint8_t countTrack,STrackParam* paramS){
	g_lenPPS=-1;
	g_lenSPS=-1;
	g_PPS = 0;
	g_SPS = 0;
	context = new SEncoderContext;
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
	pthread_create(&tid,NULL,workTask,NULL);
}
void addSample(uint8_t trackId,uint8_t* sample,size_t len){

}


int main(int argc,char** argv){
	printf("-----seraphim--------\n");
	std::map<int,int> *m = new map<int,int>;
	(*m)[1] = 10;
	std::cout<<(*m)[1]<<std::endl;
	pthread_t tid ;
	pthread_create(&tid,NULL,workTask,NULL);
	int i;

	cin>>i;
	return 0;
}