#ifndef  __SERAPHIM_SGASpecificConfig_H
#define  __SERAPHIM_SGASpecificConfig_H
#pragma once
#include<stdint.h>
#include<iostream>
#include"latm_config.h"
#include"../seraphim/bit_reader.h"
#include"SBitReadableImpl.h"
namespace Seraphim{
/*********************************************
****
**********************************************/
class SProgramConfigElement :public SBitReadableImpl {
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
	LATM_BIT_NUM(1) mono_mixdown_present ;//1



#if(1) LATM_ONLY(mono_mixdown_present == 1)
	LATM_BIT_NUM(4) mono_mixdown_element_number;//  4 
#endif
	LATM_BIT_NUM(1)  stereo_mixdown_present; // 1
#if(1) LATM_ONLY(stereo_mixdown_present == 1)
	LATM_BIT_NUM(4) stereo_mixdown_element_number;//  4 
#endif
	LATM_BIT_NUM(1)  matrix_mixdown_idx_present; //1
#if(1) LATM_ONLY(matrix_mixdown_idx_present)
	LATM_BIT_NUM(2)  matrix_mixdown_idx ; //2
	LATM_BIT_NUM(1)  pseudo_surround_enable; //1
#endif 
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

	LATM_BIT_NUM(4)  *lfe_element_tag_select_S ;//num_assoc_data_elements

	LATM_BIT_NUM(4) * assoc_data_element_tag_select_S;//num_assoc_data_elements

	typedef struct{
		LATM_BIT_NUM(1) cc_element_is_ind_sw;//1
		LATM_BIT_NUM(4) valid_cc_element_tag_select;//4
	}cc_element;
	cc_element *cc_element_S;// NUMOF  num_valid_cc_elements;
#define BYTE_ALING
	LATM_BIT_NUM(8)   comment_field_bytes; //8
	uint8_t*  comment_field_data_S;// 8 *  comment_field_bytes

private:
	void process();
public :
	SProgramConfigElement(SBitReader *_reader):SBitReadableImpl(_reader){
		process();
	
	}	

	friend std::ostream& operator<<(std::ostream& o,SProgramConfigElement& e);

public: //GETTER
	LATM_BIT_NUM(4) getElement_instance_tag(){return element_instance_tag;}
	LATM_BIT_NUM(2)	getObject_type(){return object_type;}
	LATM_BIT_NUM(4)	getSampling_frequency_index(){return sampling_frequency_index;}
	LATM_BIT_NUM(4)	getNum_front_channel_elements(){return num_front_channel_elements;};//  4   
	LATM_BIT_NUM(4)	getNum_side_channel_elements(){return num_side_channel_elements;} // 4   
	LATM_BIT_NUM(4)	getNum_back_channel_elements(){return num_back_channel_elements;} // 4   
	LATM_BIT_NUM(2)	getNum_lfe_channel_elements(){return num_lfe_channel_elements;} // 2   
	LATM_BIT_NUM(3)	getNum_assoc_data_elements(){return num_assoc_data_elements;}//  3 
	LATM_BIT_NUM(4)	getNum_valid_cc_elements(){return num_valid_cc_elements;} // 4  
	LATM_BIT_NUM(1) getMono_mixdown_present(){return mono_mixdown_present ;}//1

	LATM_BIT_NUM(4) getMono_mixdown_element_number(){return mono_mixdown_element_number;}//  4 
	LATM_BIT_NUM(1) getStereo_mixdown_present(){return stereo_mixdown_present;} // 1
	LATM_BIT_NUM(4) getStereo_mixdown_element_number(){return stereo_mixdown_element_number;}//  4 
	LATM_BIT_NUM(1) getMatrix_mixdown_idx_present(){return matrix_mixdown_idx_present;} //1
	LATM_BIT_NUM(2) getMatrix_mixdown_idx (){return matrix_mixdown_idx;}; //2
	LATM_BIT_NUM(1) getPseudo_surround_enable(){return pseudo_surround_enable;} //1b

	LATM_BIT_NUM(1) getFront_element_is_cpe(uint8_t frontIndex){return front_element_S[frontIndex].front_element_is_cpe;}//1
	LATM_BIT_NUM(4) getFront_element_tag_select(uint8_t frontIndex){return front_element_S[frontIndex].front_element_tag_select;}//4

	LATM_BIT_NUM(1) getSide_element_is_cpe(uint8_t sideIndex){return side_element_S[sideIndex].side_element_is_cpe ;}// 1
	LATM_BIT_NUM(4) getSide_element_tag_select(uint8_t sideIndex){return side_element_S[sideIndex].side_element_tag_select;}//4

	LATM_BIT_NUM(1) getBack_element_is_cpe(uint8_t backIndex){return back_element_S[backIndex].back_element_is_cpe;}//1
	LATM_BIT_NUM(4) getBack_element_tag_select(uint8_t backIndex){return back_element_S[backIndex].back_element_tag_select;}//4

	LATM_BIT_NUM(4) * getLfe_element_tag_select_S(){return lfe_element_tag_select_S;};//num_assoc_data_elements
	LATM_BIT_NUM(4) * getAssoc_data_element_tag_select_S(){return assoc_data_element_tag_select_S;}//num_assoc_data_elements


	LATM_BIT_NUM(1) getCc_element_is_ind_sw(uint8_t validCcIndex){return cc_element_S[validCcIndex].cc_element_is_ind_sw;}//1
	LATM_BIT_NUM(4) getValid_cc_element_tag_select(uint8_t validCcIndex){return cc_element_S[validCcIndex].valid_cc_element_tag_select;}//4

	LATM_BIT_NUM(8)   getComment_field_bytes(){return comment_field_bytes;}
	LATM_BIT_NUM(8) *  getComment_field_data_S(){return comment_field_data_S;} 
};

/************************************************************************/
/*                                                                      */
/************************************************************************/
class SGASpecificConfig :public SBitReadableImpl
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

private:
	SBitReader *reader;
	uint8_t samplingFrequencyIndex;
	uint8_t channelConfiguration;
	uint8_t audioObjectType;
	void process();

public:
	SGASpecificConfig(SBitReader* _reader,uint8_t _samplingFrequencyIndex,uint8_t _channelConfiguration):
	SBitReadableImpl(_reader),
	samplingFrequencyIndex(_samplingFrequencyIndex),
	channelConfiguration(_channelConfiguration){process();
	}

	~SGASpecificConfig(void){}



	friend std::ostream&  operator<<(std::ostream&,SGASpecificConfig&);
public:  //GETTER
	LATM_BIT_NUM(1)  getFrameLengthFlag(){return frameLengthFlag;}; //1
	LATM_BIT_NUM(1)  getDependsOnCoreCoder(){return dependsOnCoreCoder;} //1
	LATM_BIT_NUM(14) getCoreCoderDelay(){return coreCoderDelay;}//14 
	LATM_BIT_NUM(1)  getExtensionFlag(){return extensionFlag;}
	SProgramConfigElement* getProgramConfigElement(/*uint8_t programConfigElementIndex*/){return programConfigElement;};
	LATM_BIT_NUM(3)  getLayerNr(){return layerNr;}
	LATM_BIT_NUM(5)  getNumOfSubFrame(){return numOfSubFrame;}//5
	LATM_BIT_NUM(11) getLayer_length(){return layer_length;} //11;
	LATM_BIT_NUM(1) getAacSectionDataResilienceFlag(){return aacSectionDataResilienceFlag;}
	LATM_BIT_NUM(1) getAacScalefactorDataResilienceFlag(){return aacScalefactorDataResilienceFlag;}//  1  bslbf 
	LATM_BIT_NUM(1) getAacSpectralDataResilienceFlag(){return aacSpectralDataResilienceFlag;}
	LATM_BIT_NUM(1) getextensionFlag3(){return extensionFlag3;} //1
};







};
#endif


