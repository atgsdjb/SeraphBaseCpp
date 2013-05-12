#ifndef  __SERAPHIM_SGASpecificConfig_H
#define  __SERAPHIM_SGASpecificConfig_H
#pragma once
#include<stdint.h>
#include"latm_config.h"
#include"../seraphim/bit_reader.h"
namespace Seraphim{
	class SProgramConfigElement;
	

class SGASpecificConfig
{
private :
	LATM_BIT_NUM(1)  frameLengthFlag; //1
	LATM_BIT_NUM(1)  dependsOnCoreCoder; //1
#if(1) LATM_ONLY(dependsOnCoreCoder) 
		LATM_BIT_NUM(14) coreCoderDelay;//14 
#endif 
	LATM_BIT_NUM(1)  extensionFlag; //1
#if(1) LATM_ONLY(!channelConfiguration)
	SProgramConfigElement* programConfigElement;
#endif
#if(1) LATM_ONLY((audioObjectType == 6) || (audioObjectType == 20)) 
	LATM_BIT_NUM(3)   layerNr; //3 
#endif 
#if(1) LATM_ONLY(extensionFlag && audioObjectType == 22)
	LATM_BIT_NUM(5)    numOfSubFrame ;//5
	LATM_BIT_NUM(11)   layer_length; //11;
#endif 
	
#if(1) LATM_ONLY(extensionFlag && audioObjectType == 17 || audioObjectType == 19 || audioObjectType == 20 || audioObjectType == 23)
	LATM_BIT_NUM(1) aacSectionDataResilienceFlag;  //1  bslbf 
	LATM_BIT_NUM(1) aacScalefactorDataResilienceFlag;//  1  bslbf 
	LATM_BIT_NUM(1) aacSpectralDataResilienceFlag;//  1 
#endif
	LATM_BIT_NUM(1)  extensionFlag3; //1


public:
	SGASpecificConfig(void){}
	~SGASpecificConfig(void){}
};
	//inside
class SProgramConfigElement{
private :

	LATM_BIT_NUM(4) element_instance_tag; // 4   
	LATM_BIT_NUM(2)	object_type;          //2   
	LATM_BIT_NUM(4)	sampling_frequency_index;//  4   
	LATM_BIT_NUM(4)	num_front_channel_elements;//  4   
	LATM_BIT_NUM(4)	num_side_channel_elements; // 4   
	LATM_BIT_NUM(4)	num_back_channel_elements; // 4   
	LATM_BIT_NUM(2)	num_lfe_channel_elements; // 2   
	LATM_BIT_NUM(3)	num_assoc_data_elements;//  3 
	LATM_BIT_NUM(4)	num_valid_cc_elements; // 4  
#if(0) LATM_ONLY(mono_mixdown_present == 1)
	LATM_BIT_NUM(4) mono_mixdown_element_number;//  4 
#endif
	LATM_BIT_NUM(1)  stereo_mixdown_present; // 1
#if(0) LATM_ONLY(stereo_mixdown_present == 1)
	LATM_BIT_NUM(4) stereo_mixdown_element_number;  4 
#endif
	LATM_BIT_NUM(1)  matrix_mixdown_idx_present; //1
	//
	typedef struct{
		LATM_BIT_NUM(1) front_element_is_cpe;//1
		LATM_BIT_NUM(4) front_element_tag_select;//4
	}front_element;
	front_element *front_element_S;  // NUMOF  num_front_channel_elements
	//
	typedef struct{
		LATM_BIT_NUM(1)side_element_is_cpe ;// 1
		LATM_BIT_NUM(4)side_element_tag_select;//4
	}side_element;
	side_element  *side_element_S;   //NUMOF num_side_channel_elements
	//
	typedef struct{
		LATM_BIT_NUM(1) back_element_is_cpe;//1
		LATM_BIT_NUM(4) back_element_tag_select;//4
	}back_element;
	back_element *back_element_S;// NUMOF num_back_channel_elements

	//
	typedef struct{
		LATM_BIT_NUM(1) elf_element_is_cpe;//1
		LATM_BIT_NUM(4) elf_element_tag_select;//4
	}elf_element;
	elf_element *elf_element_S;//NUMOF num_elf_channel_elements;

	//
	uint8_t * assoc_data_element_tag_select;// 4*num_assoc_data_elements

	typedef struct{
		LATM_BIT_NUM(1) cc_element_is_ind_sw;//1
		LATM_BIT_NUM(4) valid_cc_element_tag_select;//4
	}cc_element;
	cc_element *cc_element_S;// NUMOF  num_valid_cc_elements;
#define BYTE_ALING
	LATM_BIT_NUM(8)   comment_field_bytes; //8
	uint8_t*  comment_field_data_S;// 8 *  comment_field_bytes
};




};
#endif


