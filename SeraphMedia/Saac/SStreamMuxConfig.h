#ifndef __SSTREAM_MUX_CONFIG_H
#define __SSTREAM_MUX_CONFIG_H
#include<stdint.h>
#include"latm_config.h"
#include"SAudioSpecificConfig.h"
namespace Seraphim{
class SStreamMuxConfig_Prog_Layer;
class SStreamMuxConfig_Prog;
class SStreamMuxConfig{
private :
	LATM_BIT_NUM(1) audioMuxVersion;//1
#if(0)  LATM_ONLY(audioMuxVersion)
	LATM_BIT_NUM(1) audioMuxVersionA; //1  may not have. 
#endif
#if(0) LATM_ONLY(audioMuxVersion==1 && androidMuxVersionA==0)
LATM_DEL_SLatmGetValue
#endif
	LATM_BIT_NUM(1)  allStreamsSameTimeFraming; //1
	LATM_BIT_NUM(6)  numSubFrames; //6
	LATM_BIT_NUM(4)  numProgram; //4
	SStreamMuxConfig_Prog *prog_S;// NUMOF numProgram
};
/************************************************************************/
/*                                                                      */
/************************************************************************/
class SStreamMuxConfig_Prog{
	LATM_BIT_NUM(3) numLayer; //3
	SStreamMuxConfig_Prog_Layer *layer_S;// NUMOF numLayer
};

/************************************************************************/
/*                                                                      */
/************************************************************************/
class SStreamMuxConfig_Prog_Layer{
#if(1)  LATM_ONLY(prog != 0 || lay != 0)
	LATM_BIT_NUM(1) useSameConfig ;//1  if( prog 1= 0 || lay == 0)
#endif


#if(1) LATM_ONLY(useSameConfig != 0)
#if(1) LATM_ONLY(audioMuxVersion == 0)
SSAudioSpecificConfig* SAudioSpecificConfig;
#else
LATM_DEL_SLatmGetValue
SSAudioSpecificConfig *audioSPecificConfig; 
uint8_t *fillBits // len = getVleue() - AudioSPecificConfig
#endif
#endif

LATM_BIT_NUM(3)	frameLengthType;//3
#if(0) LATM_SELETE(1,frameLengthType) LATM_ONLY(0)
 latmBufferFullness;
...........
#endif
#if(1) LATM_SELETE(1,frameLengthType) LATM_ONLY(1)
LATM_BIT_NUM(9) frameLength;//  frameLength[streamID[prog][lay]]; 
#endif
#if(0) LATM_SELETE(1,frameLengthType) LATM_ONLY(4) LATM_ONLY(5)  LATM_ONLY(3)
.....
#endif
#if(0) LATM_SELETE(1,frameLengthType) LATM_ONLY(6) LATM_ONLY(7) 
....
#endif
};
};

#endif