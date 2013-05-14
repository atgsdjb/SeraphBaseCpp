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
	 return o;
   }



};