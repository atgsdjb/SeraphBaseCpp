#include"SGASpecificConfig.h"
namespace Seraphim{
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/ 
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

	std::ostream& operator<<(std::ostream& o,SGASpecificConfig& c){
		o<<"SGASpecificConfig:{";
		o<<"[AacScalefactorDataResilienceFlag = "<<c.getAacScalefactorDataResilienceFlag()<<"]";
		o<<"[AacSectionDataResilienceFlag="<<c.getAacSectionDataResilienceFlag()<<"]";
		o<<"[AacSpectralDataResilienceFlag="<<c.getAacSpectralDataResilienceFlag()<<"]";
		o<<"[CoreCoderDelay="<<c.getCoreCoderDelay()<<"]";
		o<<"[DependsOnCoreCoder="<<c.getDependsOnCoreCoder()<<"]";
		o<<"[ExtensionFlag="<<c.getExtensionFlag()<<"]";
		o<<"[extensionFlag3="<<c.getextensionFlag3()<<"]";
		o<<"[FrameLengthFlag="<<c.getFrameLengthFlag()<<"]";
		o<<"[Layer_length="<<c.getLayer_length()<<"]";
		o<<"[LayerNr="<<c.getLayerNr()<<"]";
		o<<"[ProgramConfigElement="<<c.getProgramConfigElement()<<"]";
		o<<"[NumOfSubFrame="<<c.getNumOfSubFrame()<<"]";
		return o;
	}
 std::ostream& operator<<(std::ostream& o,SProgramConfigElement& e){
	 o<<"SProgramConfigElement:";
	o<<"[="<<e.getElement_instance_tag() <<"]";
	o<<"[="<<e.getObject_type() <<"]";
	o<<"[="<<e.getSampling_frequency_index()<<"]";
	o<<"[="<<e.	getNum_front_channel_elements()<<"]";
	o<<"[="<<e.getNum_side_channel_elements()<<"]";
	o<<"[="<<e.getNum_back_channel_elements()<<"]";
	o<<"[="<<e.getNum_lfe_channel_elements()<<"]";
	o<<"[="<<e.getNum_assoc_data_elements()<<"]";
	o<<"[="<<e.getNum_valid_cc_elements()<<"]";
	o<<"[="<<e.getMono_mixdown_present()<<"]";
	o<<"[="<<e.getMono_mixdown_element_number()<<"]";
	o<<"[="<<e.getStereo_mixdown_present()<<"]";
	o<<"[="<<e.getStereo_mixdown_element_number()<<"]";
	o<<"[="<<e.getMatrix_mixdown_idx_present()<<"]";
	o<<"[="<<e.getMatrix_mixdown_idx ()<<"]";
	o<<"[="<<e.getPseudo_surround_enable()<<"]";
	for(int i = 0;i<e.getNum_front_channel_elements();i++){
		o<<"[="<<e.getFront_element_is_cpe(i)<<"]";
		o<<"[="<<e.getFront_element_tag_select(i)<<"]";
	}
	for(int i = 0;i<e.getNum_side_channel_elements();i++){
	o<<"[="<< e.getSide_element_is_cpe(i)<<"]";
	o<<"[="<< e.getSide_element_tag_select(i)<<"]";
	}
	for(int i=0;i<e.getNum_back_channel_elements();i++){
		o<<"[="<<e.getBack_element_is_cpe(i)<<"]";
		o<<"[="<<e.getBack_element_tag_select(i) <<"]";
	}
	for(int i=0;i<e.getNum_lfe_channel_elements();i++){

		o<<"[="<<e.getLfe_element_tag_select_S()[i] <<"]";
	}
	for(int i =0;i<e.getNum_assoc_data_elements();i++){
		o<<"[="<<e.getAssoc_data_element_tag_select_S()[i] <<"]";
	}
	for(int i=0;i<e.getNum_valid_cc_elements();i++){
		o<<"[="<<e.getCc_element_is_ind_sw(i)<<"]";
		o<<"[="<<e.getValid_cc_element_tag_select(i)<<"]";
	}
	for(int i=0;i<e.comment_field_bytes;i++){
		o<<"[="<<e.getComment_field_data_S()[i]<<"]";
	}
	 return o;
   }


};