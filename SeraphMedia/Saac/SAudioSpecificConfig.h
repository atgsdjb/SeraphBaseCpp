#ifndef __SERAPHIM_SAudioSpecificConfig_H
#define __SERAPHIM_SAudioSpecificConfig_H
#include<stdint.h>
#include"SGASpecificConfig.h"
#include"../Seraphim/bit_reader.h"
#include"SBitReadableImpl.h"
namespace Seraphim{
/************************************************************************/
/*                                                                      */
/************************************************************************/
class SAudioSpecificConfig : public SBitReadableImpl{
private:
	typedef struct{
	LATM_BIT_NUM(5) audioObjectType ;//5;
#if(1) LATM_ONLY(audioType==31)
	LATM_BIT_NUM(6) audioObjectTypeExt ;//6  audioObjectType = 32 + audioObjectTypeExt; 
#endif
	}_AudioObjectType;
	_AudioObjectType *objectType;
	LATM_BIT_NUM(4)  samplingFrequencyIndex; //4
#if(1) LATM_ONLY(samplingFrequencyIndex == 0xf)
	LATM_BIT_NUM(24)  samplingFrequency;//24 .may not have .only if(samplingFrequencyIndex == 0xf)
#endif
	LATM_BIT_NUM(4)  channelConfiguration; //4
	//  audioObjectType=={1,3,6,7,17,19,20,21,22,23}
	SGASpecificConfig *gacSpecificConfig;
private:
	SBitReader *reader;
	void process();
public:
	SAudioSpecificConfig(SBitReader* _reader):SBitReadableImpl(_reader){process();};
	

public:  //GETTER
	LATM_BIT_NUM(4)  getSamplingFrequencyIndex(){return samplingFrequencyIndex;};
	LATM_BIT_NUM(24) getSamplingFrequency(){return samplingFrequency;};
	LATM_BIT_NUM(4)  getChannelConfiguration(){return channelConfiguration;}
	SGASpecificConfig *getGacSpecificConfig(){return gacSpecificConfig;};
	LATM_BIT_NUM(5) getAudioObjectType(){return objectType->audioObjectType;} ;//5;
};
/*class _AudioObjectType{
}*/;
};
#endif