#ifndef  __SERAPHIM_SGASpecificConfig_H
#define  __SERAPHIM_SGASpecificConfig_H
#pragma once
#include<stdint.h>
#include"latm_config.h"
#include"../Saac/latm_config.h"
#include"../seraphim/bit_reader.h"
namespace Seraphim{
	class SProgramConfigElement;
	

class SGASpecificConfig
{
private :
	uint8_t  frameLengthFlag; //1
	uint8_t  dependsOnCoreCoder; //1
#if(0) LATM_ONLY(dependsOnCoreCoder) 
		uint16_t coreCoderDelay;//14 
#endif 
	uint8_t  extensionFlag; //1
#if(0) LATM_ONLY(!channelConfiguration)
	SProgramConfigElement* programConfigElement;
#endif
#if(0) LATM_ONLY((audioObjectType == 6) || (audioObjectType == 20)) 
	uint8_t   layerNr; //3 
#endif 
#if(0) LATM_ONLY(extensionFlag && audioObjectType == 22)
	uint8_t    numOfSubFrame ;//5
	uint16_t   layer_length; //11;
#endif 
	
#if(0) LATM_ONLY(extensionFlag && audioObjectType == 17 || audioObjectType == 19 || audioObjectType == 20 || audioObjectType == 23)
	uint8_t aacSectionDataResilienceFlag;  //1  bslbf 
	uint8_t aacScalefactorDataResilienceFlag;//  1  bslbf 
	uint8_t aacSpectralDataResilienceFlag;//  1 
#endif
	uint8_t  extensionFlag3; //1


public:
	SGASpecificConfig(void){}
	~SGASpecificConfig(void){}
};
	//inside
class SProgramConfigElement{
private :

	uint8_t element_instance_tag; // 4   
	uint8_t	object_type;          //2   
	uint8_t	sampling_frequency_index;//  4   
	uint8_t	num_front_channel_elements;//  4   
	uint8_t	num_side_channel_elements; // 4   
	uint8_t	num_back_channel_elements; // 4   
	uint8_t	num_lfe_channel_elements; // 2   
	uint8_t	num_assoc_data_elements;//  3 
	uint8_t	num_valid_cc_elements; // 4  
#if(0) LATM_ONLY(mono_mixdown_present == 1)
	uint8_t mono_mixdown_element_number;//  4 
#endif
	uint8_t  stereo_mixdown_present; // 1
#if(0) LATM_ONLY(stereo_mixdown_present == 1)
	uint8_t stereo_mixdown_element_number;  4 
#endif
	uint8_t  matrix_mixdown_idx_present; //1
	//
	typedef struct{
		uint8_t front_element_is_cpe;//1
		uint8_t front_element_tag_select;//4
	}front_element;
	front_element *front_element_S;  // NUMOF  num_front_channel_elements
	//
	typedef struct{
		uint8_t side_element_is_cpe ;// 1
		uint8_t side_element_tag_select;//4
	}side_element;
	side_element  *side_element_S;   //NUMOF num_side_channel_elements
	//
	typedef struct{
		uint8_t back_element_is_cpe;//1
		uint8_t back_element_tag_select;//4
	}back_element;
	back_element *back_element_S;// NUMOF num_back_channel_elements

	//
	typedef struct{
		uint8_t elf_element_is_cpe;//1
		uint8_t elf_element_tag_select;//4
	}elf_element;
	elf_element *elf_element_S;//NUMOF num_elf_channel_elements;

	//
	uint8_t * assoc_data_element_tag_select;// 4*num_assoc_data_elements

	typedef struct{
		uint8_t cc_element_is_ind_sw;//1
		uint8_t valid_cc_element_tag_select;//4
	}cc_element;
	cc_element *cc_element_S;// NUMOF  num_valid_cc_elements;
#define BYTE_ALING
	uint8_t comment_field_bytes; //8
	uint8_t*  comment_field_data_S// 8 *  comment_field_bytes
};




};
#endif


