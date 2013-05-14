#include"SStreamMuxConfig.h"
#include<string>
using namespace  std;
namespace Seraphim{
	const char* SStreamMuxConfig::toString(){
		char* str = new char[1024];
		sprintf(str,"SStream:\
					{AudioMuxVersion=%d}\
					{AudioMuxVersionA=%d}\
					{AllStreamsSameTimeFraming=%d}\
					{NumSubFrames=%d}\
					{NumProgram=%d}\
					{NumLayer=%d}\
					{UseSameConfig=%d}\
					");
		return 0;
	}
	std::ostream& operator<<(std::ostream& o,SStreamMuxConfig& c){
		o<<"SStream:"<<"{";
		o<<"[AudioMuxVersion = "<<c.audioMuxVersion<<"]";
		o<<"[AudioMuxVersionA = "<<c.audioMuxVersionA<<"]";
		o<<"[AllStreamsSameTimeFraming = "<<c.allStreamsSameTimeFraming<<"]";
		o<<"[NumSubFrames = "<<c.numSubFrames<<"]";
		o<<"[NumProgram = "<<c.numProgram<<"]";
		for(int i=0;i<c.getNumProgram();i++){
			o<<c.prog_S[i];
		}	
		o<<"}";
		return o;
	};
	std::ostream& operator<<(std::ostream& o,SStreamMuxConfig_Prog& p){
		o<<"SStream_Prog:{";
		o<<"[numLayer = " <<p.numLayer<<"]";
		for(int i =0;i<p.numLayer;i++){
			o<<p.layer_S[i];
		}
		o<<"}";
		return o;
	}

	std::ostream& operator<<(std::ostream& o, SStreamMuxConfig_Prog_Layer& l){
		o<<"SStream_Prog_Layer:{";
		o<<"[useSameConfig = " <<l.useSameConfig<<"]";
		o<<"[audioSpecificConfig="<<l.sAudioSpecificConfig<<"]";
		o<<"[frameLengthType = "<<l.frameLengthType<<"]";
		o<<"[frameLength = "<<l.frameLength<<"]";
		o<<"}";
		return o;
	}

};