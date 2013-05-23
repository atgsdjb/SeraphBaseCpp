#include"context.h" 
#include"SMp4Creater.h"
#include"../aac/faac.h"
using namespace Seraphim;
void* workTask(void* param){
	int fileIndex = 0;
	char fileName[64]={0};
	sprintf(fileName,context->baseName,fileIndex++);
	//  wait  pps  sps
	do{
	}while(g_PPS == NULL || g_SPS==NULL);
	while(context->runing){
		char fileName[64]={0};
		sprintf(fileName,context->baseName,fileIndex++);
		SMp4Creater creater(fileName,context->idAndParm,context->idAndBuf);
		creater.addPPS(g_PPS,g_lenPPS,0);
		creater.addSPS(g_SPS,g_lenSPS,0);
		creater.startEncode();
	}
	return 0;

}
 void* aacTask(void *param){

	return 0;
}

 void* avcTask(void* param){
	return 0;
}