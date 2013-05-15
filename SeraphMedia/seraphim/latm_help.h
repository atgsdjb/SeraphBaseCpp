#ifndef __SERAPHIM_LATM_HELP_H
#define __SERAPHIM_LATM_HELP_H
#include<stdint.h>
namespace Seraphim{
#define BYTE_ALING 

	//class  SAudioSpecificConfig;
	//class  SStreamMuxConfig;
	//class  SPayloadLengthInfo;
	//class  SPayloadMux;
	//class  SProgram;
	//class  SLayer; 
	//class SLatmHelp{
	//private:
	//	uint8_t useSameStreamMux; // 1 may not have , if muxConfigPresent ==  1;
	//	SStreamMuxConfig *streamMuxConfig;
	//	
	//	typedef struct {
	//		SPayloadLengthInfo*	   payloadLengthInfo;
	//		SPayloadMux*		   plaloadMux;
	//	}Payload;


	//	Payload* payLoad_S;//  *  numSubFrames
	//    BYTE_ALING
	//};

	//class SStreamMuxConfig{
	//private :
	//	uint8_t audioMuxVersion;//1
	//	uint8_t audioMuxVersionA; //1  may not have. if  audioMuxVersion == 1,else audioMuxVersion = 0
	//	uint8_t  allStreamsSameTimeFraming; //1
	//	uint8_t  numSubFrames; //6
	//	uint8_t  numProgram; //4
	//};

	//class SPayloadLengthInfo{
	//	 
	//};
	//class SPayloadMux{

	//};
	//class SLayer{
	//	uint8_t useSameConfig ;//1  if( prog 1= 0 || lay == 0)
	//	
	//	
	//		
	//	//
	//	uint8_t frameLengthType;//3
	//	typedef struct{
	//		uint8_t latmBufferFullness;//8
	//		uint8_t coreFrameOffset;   //6   may not have   if ((AudioObjectType[lay] == 6 || AudioObjectType[lay] == 20) &&
	//		//  (AudioObjectType[lay-1] == 8 || AudioObjectType[lay-1] == 24))

	//	}frameLengthTypeEqu0;
	//	frameLengthTypeEqu0  type0 ;// if frameLengthType == 0;
	//	uint16_t  frameLength;//9  frameLengthType ==1
	//	uint8_t   CELPframeLengthTableIndex;//6   frameLengthType =={3,4,5}
	//	uint8_t   HVXCframeLengthTableIndex; //1  frameLengthType =={6,7}
	//};

	//class SProgram{
	//private:
	//	uint8_t numLayer; //3
	//	SLayer *layer_S;
	//	
	//};

	

};
#endif