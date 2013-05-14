#ifndef __SSTREAM_MUX_CONFIG_H
#define __SSTREAM_MUX_CONFIG_H
#include<stdint.h>
#include"latm_config.h"
#include"SAudioSpecificConfig.h"
#include"SBitReadableImpl.h"
namespace Seraphim{

/************************************************************************/
/*                                                                      */
/************************************************************************/
struct SStreamMuxConfig_Prog_Layer{
#if(1)  LATM_ONLY(prog != 0 || lay != 0)
		LATM_BIT_NUM(1) useSameConfig ;//1  if( prog 1= 0 || lay == 0)
#endif


#if(1) LATM_ONLY(useSameConfig != 0)
#if(1) LATM_ONLY(audioMuxVersion == 0)
		SAudioSpecificConfig* sAudioSpecificConfig;
#else
		LATM_DEL_SLatmGetValue
			SSAudioSpecificConfig *audioSPecificConfig; 
		uint8_t *fillBits // len = getVleue() - AudioSPecificConfig
#endif
#endif

			LATM_BIT_NUM(3)	frameLengthType;//3
#if(1) LATM_SELETE(1,frameLengthType) LATM_ONLY(0)
	//	latmBufferFullness;
	//	...........
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
};

/************************************************************************/
/*                                                                      */
/************************************************************************/
struct SStreamMuxConfig_Prog{
	LATM_BIT_NUM(3) numLayer; //3
	SStreamMuxConfig_Prog_Layer *layer_S;// NUMOF numLayer
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
#if(0) LATM_ONLY(otherDataPresent)
#if(0)
	  otherDataLenBits = LatmGetValue(); 
#else
	otherDataLenBits = 0; /* helper variable 32bit */           
	do {           
		otherDataLenBits *= 2^8;           
		otherDataLenEsc;  1       uimsbf 
			otherDataLenTmp;  8       uimsbf 
			otherDataLenBits += otherDataLenTmp;           
	} while (otherDataLenEsc); 
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