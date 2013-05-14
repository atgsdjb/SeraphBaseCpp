#ifndef __SAUDIO_MUX_ELEMENT_H
#define __SAUDIO_MUX_ELEMENT_H
#include<stdint.h>
#include"SStreamMuxConfig.h"
#include"latm_config.h"
#include"SBitReadableImpl.h"
namespace Seraphim{

class PayloadLengthInfo;
class PayloadMux;
/************************************************************************/
/*                                                                      */
/************************************************************************/
class SAudioMuxElement  : public  SBitReadableImpl{
#if(1)  LATM_ONLY(muxConfigPresent)
LATM_BIT_NUM(1) useSameStreamMux ;
#if(1) LATM_ONLY(useSameStreamMux == 0)
SStreamMuxConfig *streamMuxConfig;
#endif
#endif
typedef struct{
PayloadLengthInfo *paylengthInfo;
PayloadMux  *payloadMux;
}PayLoad;

PayLoad *payLoad_S;// NUMOF numSubFrames
#if(1)  LATM_ONLY(otherDataPresent)
LATM_BIT_NUM(1) *otherDataBit_S;// NUMOF otherDataLenBits

#endif

private:
	SBitReader *reader;
	void process();
	uint8_t muxConfigPresent;

public :
	SAudioMuxElement(SBitReader *_reader,uint8_t _muxConfigPresent=1):SBitReadableImpl(_reader),muxConfigPresent(_muxConfigPresent){process();};
public:  //GETER
	LATM_BIT_NUM(1) getUseSameStream(){return useSameStreamMux;};
	SStreamMuxConfig * getStreamMuxConfig(){return streamMuxConfig;};
	PayLoad* getPayLoad_S(){return payLoad_S;};
};
/************************************************************************/
/*                                                                      */
/************************************************************************/
class PayloadLengthInfo{

};
/************************************************************************/
/*                                                                      */
/************************************************************************/
class PayloadMux{

};
};
#endif