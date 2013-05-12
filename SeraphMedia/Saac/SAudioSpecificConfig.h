#ifndef __SERAPHIM_SAudioSpecificConfig_H
#define __SERAPHIM_SAudioSpecificConfig_H
#include<stdint.h>
#include"SGASpecificConfig.h"
namespace Seraphim{
class SSAudioSpecificConfig_AudioObjectType;
class SSAudioSpecificConfig{
private:
	SSAudioSpecificConfig_AudioObjectType *audioObjectType;
	LATM_BIT_NUM(4)  samplingFrequencyIndex; //4
	LATM_BIT_NUM(24)  samplingFrequency;//24 .may not have .only if(samplingFrequencyIndex == 0xf)
	LATM_BIT_NUM(4)  channelConfiguration; //4
	//  audioObjectType=={1,3,6,7,17,19,20,21,22,23}
	SGASpecificConfig *gacSpecificConfig;
public:
	//SGASpecificConfig(){};
	
};
class SSAudioSpecificConfig_AudioObjectType{
	LATM_BIT_NUM(5) audioObjectType ;//5;
#if(0) LATM_ONLY(audioType==31)
	LATM_BIT_NUM(6) audioObjectTypeExt ;//6  audioObjectType = 32 + audioObjectTypeExt; 
#endif
};
};
#endif