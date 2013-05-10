#ifndef __SERAPHIM_SAudioSpecificConfig_H
#define __SERAPHIM_SAudioSpecificConfig_H
namespace Seraphim{

class SGASpecificConfig;	
class SAudioSpecificConfig{
private:
	typedef struct {
		uint8_t audioObjectType ;//5;
if(0) LATM_ONLY(audioType==31)
		uint8_t audioObjectTypeExt ;//6  audioObjectType = 32 + audioObjectTypeExt; 
#endif
	}AuidoObjectType;
	AuidoObjectType *audioObjectType;
	uint8_t  samplingFrequencyIndex; //4
	uint32_t  samplingFrequency;//24 .may not have .only if(samplingFrequencyIndex == 0xf)
	uint8_t  channelConfiguration; //4
	
	
	//  audioObjectType=={1,3,6,7,17,19,20,21,22,23}
	SGASpecificConfig *gacSpecificConfig;
public:
	SGASpecificConfig(){};
	
};

};
#endif