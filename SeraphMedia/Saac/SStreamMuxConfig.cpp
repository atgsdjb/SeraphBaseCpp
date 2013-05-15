#include"SStreamMuxConfig.h"
#include<string>
using namespace  std;
namespace Seraphim{
/************************************************************************/
/*                                                                      */
/************************************************************************/
void SStreamMuxConfig::process(){
	audioMuxVersion = getByte(1);
	if(audioMuxVersion){
		audioMuxVersionA = getByte(1);
	}else{
		audioMuxVersionA = 0;
	}

	if(audioMuxVersionA==0){
		if(audioMuxVersion !=0){
			//  taraBufferFullness = LatmGetValue(); 
		}
		allStreamsSameTimeFraming = getByte(1);
		numSubFrames = getByte(6);
		numProgram = getByte(4);
		prog_S = new SStreamMuxConfig_Prog[numProgram];
		for(int i =0;i<numProgram;i++){
			prog_S[i].numLayer = getByte(3);
			prog_S[i].layer_S = new SStreamMuxConfig_Prog_Layer[prog_S[i].numLayer];
			for(int l = 0;l<prog_S[i].numLayer;l++){
				SStreamMuxConfig_Prog_Layer *layer = &(prog_S[i].layer_S[l]);

				if(l==0 && i == 0){
					layer->useSameConfig =0;
				}else{
					layer->useSameConfig = getByte(1);
				}
				if(layer->useSameConfig ==0){
					if(audioMuxVersion ==0){
						layer->sAudioSpecificConfig = new SAudioSpecificConfig(getReader());
					}else{
						
					}
				}else{
					layer->sAudioSpecificConfig = 0;
				}
				layer->frameLengthType = getByte(3);
				switch(layer->frameLengthType){
				case 0:break;
				case 1:
					layer->frameLength = getShort(9);
					break;
				case 2:break;
				case 3:case 4:case 5:break;
				case 6:case 7:break;
				}
			}
		}
		crcCheckPresent = getByte(1);
		if(crcCheckPresent){
			crcCheckSum = getByte(8);
		}
	}else{

	}
}
/************************************************************************/
/*                                                                      */
/************************************************************************/
	std::ofstream& operator<<(std::ofstream& o,SStreamMuxConfig c){
		o<<"SStream:"<<"{";
		o<<"[AudioMuxVersion = "<<(int)c.getAudioMuxVersion()<<"]";
		o<<"[AudioMuxVersionA = "<<(int)c.getAudioMuxVersionA()<<"]";
		o<<"[AllStreamsSameTimeFraming = "<<(int)c.getAllStreamsSameTimeFraming()<<"]";
		o<<"[NumSubFrames = "<<(int)c.getNumSubFrames()<<"]";
		o<<"[NumProgram = "<<(int)c.getNumProgram()<<"]";
		for(int i=0;i<c.getNumProgram();i++){
			o<<c.prog_S[i];
			o<<endl;
		}	
		o<<"}";
		return o;
	};
	std::ofstream& operator<<(std::ofstream& o,SStreamMuxConfig_Prog& p){
		o<<"SStream_Prog:{";
		o<<"[numLayer = " <<(int)p.numLayer<<"]";
		for(int i =0;i<p.numLayer;i++){
			o<<p.layer_S[i];
			o<<endl;
		}
		o<<"}";
		return o;
	}

	std::ofstream& operator<<(std::ofstream& o, SStreamMuxConfig_Prog_Layer& l){
		o<<"SStream_Prog_Layer:{";
		o<<"[useSameConfig = " <<(int)l.useSameConfig<<"]";
	/*	SAudioSpecificConfig l_c = (*l.sAudioSpecificConfig);
		
		int i=0;
		i++;*/
		if(l.sAudioSpecificConfig){
			o<<"[audioSpecificConfig=";
			o<<l.sAudioSpecificConfig;
			o<<"]";
		}
		o<<"[frameLengthType = "<<(int)l.frameLengthType<<"]";
		o<<"[frameLength = "<<(int)l.frameLength<<"]";
		o<<"}";
		return o;
	}

};