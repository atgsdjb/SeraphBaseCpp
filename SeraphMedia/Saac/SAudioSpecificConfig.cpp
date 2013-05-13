#include"SAudioMuxElement.h"
namespace Seraphim{
void SAudioMuxElement::process(){
	if(muxConfigPresent){
		useSameStreamMux = getByte(1);
	}
	if(useSameStreamMux==0){
		streamMuxConfig = new SStreamMuxConfig(reader);
	}
}

/***************
**
****************/
 void	SAudioSpecificConfig::process(){
	 objectType->audioObjectType = getByte(5);
	 if(objectType->audioObjectType==31){
	 this->objectType->audioObjectTypeExt = getByte(6) ;
	 objectType->audioObjectType =32 + objectType->audioObjectTypeExt; 
	 }
	 
	 samplingFrequencyIndex = getByte(4);
	 if(samplingFrequencyIndex == 0xf){
		samplingFrequency = getShort(24);
	 }

	 channelConfiguration = getByte(4);

	 gacSpecificConfig = new SGASpecificConfig(reader,samplingFrequencyIndex,channelConfiguration);

 }

 

void SGASpecificConfig::process(){
	frameLengthFlag = getByte(1);

	dependsOnCoreCoder = getByte(1);

	if(dependsOnCoreCoder){
		coreCoderDelay = getShort(14);
	}

	extensionFlag = getByte(1);

	if(channelConfiguration){
		programConfigElement = new SProgramConfigElement(reader);
	}
	
	if(audioObjectType == 6 || audioObjectType == 20 ){
		layerNr = getByte(3);
	}

	if(extensionFlag && audioObjectType==22){
		numOfSubFrame = getByte(5);
		layer_length = getShort(11);
	}

	if(extensionFlag && (audioObjectType == 17 || 
						 audioObjectType == 19 ||
						 audioObjectType == 20 ||
						 audioObjectType ==23)){
			aacSectionDataResilienceFlag = getByte(1);
			aacScalefactorDataResilienceFlag = getByte(1);
			aacSpectralDataResilienceFlag = getByte(1);
	}
	extensionFlag3 = getByte(1);
	if(extensionFlag3){
		//...............
	}



}



void SProgramConfigElement::process(){
	element_instance_tag = getByte(4);
	object_type = getByte(2);
	sampling_frequency_index = getByte(4);
	num_front_channel_elements = getByte(4);
	num_side_channel_elements = getByte(4);
	num_back_channel_elements = getByte(4);
	num_lfe_channel_elements = getByte(2);
	num_assoc_data_elements = getByte(3);
	num_valid_cc_elements = getByte(4);
	mono_mixdown_present = getByte(1);

	if(mono_mixdown_present){
		mono_mixdown_element_number = getByte(4);
	}
	
	stereo_mixdown_present = getByte(1);
	if(stereo_mixdown_present){
		stereo_mixdown_element_number = getByte(4);
	}

	matrix_mixdown_idx_present = getByte(1);

	if(num_front_channel_elements){
		front_element_S = new front_element[num_front_channel_elements];
		for(int i = 0 ;i < num_front_channel_elements; i++) { 
			front_element_S[i].front_element_is_cpe = getByte(1);
			front_element_S[i].front_element_tag_select = getByte(4);
		}
	}

	if(num_side_channel_elements){
		side_element_S = new side_element[num_side_channel_elements];
		for(int i =0; i < num_side_channel_elements ;i++){
			side_element_S[i].side_element_is_cpe = getByte(1);
			side_element_S[i].side_element_tag_select = getByte(4);
		}
	}

	if(num_back_channel_elements){
		back_element_S = new back_element[num_back_channel_elements];
		for(int i = 0;i< num_back_channel_elements ; i++){
			back_element_S[i].back_element_is_cpe = getByte(1);
			back_element_S[i].back_element_tag_select = getByte(4);
		}
	}

	if(num_lfe_channel_elements){
		lfe_element_tag_select_S = new uint8_t[num_lfe_channel_elements];
		for(int i =0;i<num_lfe_channel_elements;i++){
			lfe_element_tag_select_S[i] = getByte(4);
		}
	}

	if(num_assoc_data_elements){
		assoc_data_element_tag_select_S = new uint8_t[num_assoc_data_elements];
		for(int i = 0;i< num_assoc_data_elements;i++){
			assoc_data_element_tag_select_S[i] = getByte(4);
		}
	}

	if( num_valid_cc_elements){
		cc_element_S = new cc_element[num_valid_cc_elements];
		for(int i=0;i<num_valid_cc_elements;i++){
			cc_element_S[i].cc_element_is_ind_sw = getByte(1);
			cc_element_S[i].valid_cc_element_tag_select = getByte(4);
		}
	}
	//8-BIT  ALING
	if(readedBitNum %8){
		getByte(8- readedBitNum %8);
	}
	  comment_field_bytes = getByte(8);
	  if(comment_field_bytes){
		comment_field_data_S = new uint8_t[comment_field_bytes];
		for(int i = 0;i< comment_field_bytes;i++){
			comment_field_data_S[i] = getByte(8);
		}
	 }
}

};