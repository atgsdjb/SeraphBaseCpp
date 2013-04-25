#include"s_config.h"
#include"nalu_help.h"

#include<cstdlib>
#include<vector>
#include<cstring>
#ifdef SDEBUG
#include<iostream>
#include<cassert>
#endif
using namespace std;
namespace Seraphim{
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	int getNALUFormStream(uint8_t* stm,int lenStm,uint8_t** dist,nalu_head head,bool isCopy){
		
		if(head >= lenStm)
		return -2;
		if(head==NALU4H){
			if(stm[0]!=0 || stm[1]!= 0 || stm[2] !=0 || stm[3]!= 1 )
				return -1;
		}else{
			if(stm[0]!=0 || stm[1]!= 0 || stm[2] !=0 || stm[3]!= 0 || stm[4] != 1 )
				return -1;
		}
		int offset = head;
		while(offset < lenStm-head){
			if((stm[offset]==0 && stm[offset+1]==0 && stm[offset+2]==0) 
				&& ((head==NALU4H && stm[offset+3]==1 ) || (stm[offset+3]==0 && stm[offset+4]==1))){
				break;
			}
			offset++;
		}
		if(offset >= lenStm-head)
			offset = lenStm;
		if(isCopy){
			*dist =  new uint8_t[offset];
			memcpy(*dist,stm,offset);
		}else{
			*dist = stm;
		}
		return offset;
	}
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	int getNaluS(uint8_t* stm,int size,nalu_head head,vector<uint8_t*>& naluS,vector<int>& naluC){
	
		int sum = 0;
		uint8_t* dist;
		int len = 0;
		for(sum;sum<size;){
			len = getNALUFormStream(stm+sum,size-sum,&dist,head);
		#ifdef SDEBUG
			cout<<"len====="<<len<<"------head = "<<head<<"------------"<<endl;
			assert(len > head);
		#endif
			sum +=len;
			naluS.push_back(dist);
			naluC.push_back(len);
		}
		#ifdef SDEBUG
		assert(sum==size /*&& naluS.size() == naluC.size() */);
		#endif
		return naluS.size();
	}
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	bool isPPS(uint8_t* nalu,nalu_head head){
		uint8_t b = nalu[head];
		return (b & 0x1f)==8;
	}
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	bool isSPS(uint8_t* nalu,nalu_head head){
		uint8_t b = nalu[head];
		return (b & 0x1f)==7;
	}
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	NaluHelp::NaluHelp(char* name,size_t _maxSize):postion(0),end_buf(0),maxSize(_maxSize){
		file = fopen(name,"rb");
		//assert(file);
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