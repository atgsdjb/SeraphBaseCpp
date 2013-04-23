#include"nalu_help.h"
#include<cassert>
#include<cstdlib>
#include<cstring>
namespace Seraphim{
	NaluHelp::NaluHelp(char* name,size_t _maxSize):postion(0),end_buf(0),maxSize(_maxSize){
		file = fopen(name,"rb");
		assert(file);
		buf = new uint8_t[maxSize];
		postion = buf;
		end_buf= buf+maxSize;
		
	}
	uint8_t* NaluHelp::nextNALU(){
		int len =4;
		while(true){
			//每次迭代,4个字节
			if(checkEmpty()&& *(postion +len++)==0){
			}
		}

	}
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	bool NaluHelp::checkEmpty(){
		if(postion >= end_buf){
			return fullBuffer();
		}
		return false;
	}
	/**
	
	**/
	bool   NaluHelp::fullBuffer(){
		
		if(postion < end_buf){
		 memcpy(buf,postion,size_t(end_buf - postion));
		}
		postion = buf+(end_buf-postion);
		end_buf = fread(postion,1,maxSize-(postion-buf),file)+postion;
		if( end_buf == postion){
			if(feof(file)){
				return false;
			}
		}
		return true;
	}
};