#ifndef  __SERAPHIM_SGLUE_H
#define  __SERAPHIM_SGLUE_H
extern"C"{
#include<stdint.h>
};
#include<map>
#include"STrackParam.h"
#include"SSyncBuf.h"
using namespace  Seraphim;

typedef struct{
	char* baseName;
	uint8_t countTrack;
	uint64_t duration;
	std::map<uint8_t,STrackParam*> idAndParm;
	std::map<uint8_t,uint64_t>     idAndNSample;
	std::map<uint8_t,SSyncBuffer*> idAndBuf;
	bool runing;
}SEncoderContext;
extern size_t g_lenPPS;
extern size_t g_lenSPS;
extern uint8_t* g_PPS;
extern uint8_t* g_SPS;
extern SEncoderContext* context;

void* workTask(void*);
void* aacTask(void*);
void* avcTask(void*);

#endif