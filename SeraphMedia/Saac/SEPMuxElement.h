#ifndef  __SERAPH_EPMuxElement_H
#define	 __SERAPH_EPMuxElement_H
#include<stdint.h>
#include<iostream>
#include"SGASpecificConfig.h"
#include"../Seraphim/SBitReader.h"
#include"SBitReadableImpl.h"
#include"latm_config.h"
#include"SErrorProtectionSpecificConfig.h"
namespace Seraphim{
class SEPMuxElement:public SBitReadableImpl{
private :
	LATM_BIT_NUM(1)  epUsePreviousMuxConfig; 
	LATM_BIT_NUM(2)  epUsePreviousMuxConfigParity; 
#if(1) LATM_ONLY(!epUsePreviousMuxConfig)) 
	LATM_BIT_NUM(10)    epSpecificConfigLength; 
	LATM_BIT_NUM(11)    epSpecificConfigLengthParity; 
	SErrorProtectionSpecificConfig* errorProtectionSpecificConfig;
	ErrorProtectionSpecificConfigParity* errorProtectionSpecificConfigParity;
#endif;
BYTE_ALING

};
};

#endif