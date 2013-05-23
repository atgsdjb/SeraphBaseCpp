#ifndef __SSTREAM_MUX_CONFIG_H
#define __SSTREAM_MUX_CONFIG_H
#include<stdint.h>
#include<iostream>
#include"latm_config.h"
#include"SAudioSpecificConfig.h"
#include"SBitReadableImpl.h"
namespace Seraphim{

/************************************************************************/
/*                                                                      */
/************************************************************************/
struct SStreamMuxConfig_Prog_Layer{
	LATM_BIT_NUM(1) useSameConfig;
	SAudioSpecificConfig* sAudioSpecificConfig;
	//LATM_BIT_NUM(8) *fillBits;
	uint8_t *fillBits;
	LATM_BIT_NUM(3)	frameLengthType;
#if(1) LATM_SELETE(1) LATM_ONLY(0)
	LATM_BIT_NUM(8)  latmBufferFullness;
	/*if (frameLengthType[streamID[prog][lay] == 0) {           
		latmBufferFullness[streamID[prog][ lay]];  8       uimsbf 
			if (! allStreamsSameTimeFraming) {           
				if ((AudioObjectType[lay] == 6 || 
					AudioObjectType[lay] == 20) && 
					(AudioObjectType[lay-1] == 8 || 
					AudioObjectType[lay-1] == 24))*/
	LATM_BIT_NUM(6)  coreFrameOffset;
#endif
#if(1) LATM_SELETE(1,frameLengthType) LATM_ONLY(1)
	   LATM_BIT_NUM(9) frameLength;//  frameLength[streamID[prog][lay]]; 
#endif
#if(1) LATM_SELETE(1,frameLengthType) LATM_ONLY(4) LATM_ONLY(5)  LATM_ONLY(3)
	//	.....
#endif
#if(1) LATM_SELETE(1,frameLengthType) LATM_ONLY(6) LATM_ONLY(7) 
//			....
#endif
			friend std::ofstream& operator<<(std::ofstream& o,SStreamMuxConfig_Prog_Layer& l);
			friend std::ofstream& operator<<(std::ofstream& o,SStreamMuxConfig_Prog_Layer* pc){
				operator<<(o,*pc);
				return o;
			};
};

/************************************************************************/
/*                                                                      */
/************************************************************************/
struct SStreamMuxConfig_Prog{
	LATM_BIT_NUM(3) numLayer; //3
	SStreamMuxConfig_Prog_Layer *layer_S;// NUMOF numLayer
	friend std::ofstream& operator<<(std::ofstream &o,SStreamMuxConfig_Prog& p);
	friend std::ofstream& operator<<(std::ofstream& o,SStreamMuxConfig_Prog* pc){
		operator<<(o,*pc);
		return o;
	};
};

/************************************************************************/
/*                                                                      */
/************************************************************************/
class SStreamMuxConfig : public SBitReadableImpl{
private :
	LATM_BIT_NUM(1) audioMuxVersion;//1
#if(1)  LATM_ONLY(audioMuxVersion)
	LATM_BIT_NUM(1) audioMuxVersionA; //1  may not have. 
#endif
#if(0) LATM_ONLY(audioMuxVersion==1 && adudioMuxVersionA==0)
LATM_DEL_SLatmGetValue
#endif
LATM_BIT_NUM(1)  allStreamsSameTimeFraming; //1
LATM_BIT_NUM(6)  numSubFrames; //6
LATM_BIT_NUM(4)  numProgram; //4
SStreamMuxConfig_Prog *prog_S;// NUMOF numProgram
LATM_BIT_NUM(1) otherDataPresent;//
#if(1) LATM_ONLY(otherDataPresent)
#if(0) LATM_ONLY( audioMuxVersion == 1 )
	  otherDataLenBits = LatmGetValue(); 
#else
	typedef struct{
		LATM_BIT_NUM(1) otherDataLenEsc;
		LATM_BIT_NUM(8) otherDataLenTmp; 
	}OtherDataLen;
	OtherDataLen *otherDataLen_S;
#endif
#endif

	LATM_BIT_NUM(1) crcCheckPresent;
#if(1) LATM_ONLY(crcCheckPresent)
	LATM_BIT_NUM(8)  crcCheckSum; 
#endif 
private :
	
public:

	void process();
	SStreamMuxConfig(SBitReader *_reader):SBitReadableImpl(_reader){process();};
	friend std::ofstream& operator<<(std::ofstream& o,SStreamMuxConfig c);
	friend std::ofstream& operator<<(std::ofstream& o,SStreamMuxConfig* pc){
			operator<<(o,*pc);	
			return o;
	};
public:   //GETTER
	LATM_BIT_NUM(1) getAudioMuxVersion(){return audioMuxVersion;};
	LATM_BIT_NUM(1) getAudioMuxVersionA(){return audioMuxVersionA;};
	LATM_BIT_NUM(1) getAllStreamsSameTimeFraming(){return allStreamsSameTimeFraming;};
	LATM_BIT_NUM(6) getNumSubFrames(){return numSubFrames;};
	LATM_BIT_NUM(4) getNumProgram(){return numProgram;};
	// SStreamMuxConfig_Prog
	LATM_BIT_NUM(3) getNumLayer(uint8_t programIndex){return prog_S[programIndex].numLayer;};
	// SStreamMuxConfig_Prog_Layer
	LATM_BIT_NUM(1) getUseSameConfig(uint8_t programIndex,uint8_t layerIndex){return prog_S[programIndex].layer_S[layerIndex].useSameConfig;};
	SAudioSpecificConfig* getSAudioSpecificConfig(uint8_t programIndex,uint8_t layerIndex){return prog_S[programIndex].layer_S[layerIndex].sAudioSpecificConfig;};;						
	LATM_BIT_NUM(3)	getFrameLengthType(uint8_t programIndex,uint8_t layerIndex){
		return prog_S[programIndex].layer_S[layerIndex].frameLengthType;
	};
	LATM_BIT_NUM(9) getFrameLength(uint8_t programIndex,uint8_t layerIndex){
		return prog_S[programIndex].layer_S[layerIndex].frameLength;
	};



};
};

#endif