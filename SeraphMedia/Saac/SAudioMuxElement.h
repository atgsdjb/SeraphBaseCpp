#ifndef __SAUDIO_MUX_ELEMENT_H
#define __SAUDIO_MUX_ELEMENT_H
#include<stdint.h>
#include"SAudioSpecificConfig.h"
#include"SStreamMuxConfig.h"
#include"latm_config.h"
namespace Seraphim{

class PayloadLengthInfo;
class PayloadMux;
class SAudioMuxElement{
#if(1)  LATM_ONLY(muxConfigPresent)
LATM_BIT_NUM(1) useSameStreamMux ;
#if(1) LATM_ONLY(useSameStreamMux)
SStreamMuxConfig *streamMuxConfig;
#endif
#endif
typedef struct{
PayloadLengthInfo *paylengthInfo;
PayloadMux  *payloadMux;
}payLoad;

payLoad payLoad_S;// NUMOF numSubFrames
#if(0)  LATM_ONLY(otherDataPresent)
LATM_BIT_NUM(1) *otherDataBit_S;// NUMOF otherDataLenBits

#endif

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