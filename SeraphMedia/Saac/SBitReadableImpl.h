#ifndef __SERAPHIM_SBITREADABLE_IMPL_H
#define __SERAPHIM_SBITREADABLE_IMPL_H
#include<stdint.h>
#include"../seraphim/bit_reader.h"
namespace Seraphim{
class SBitReadableImpl{
private :
	SBitReader *reader;
protected:
	uint8_t readedBitNum;
	uint8_t getByte(uint8_t bitNum){
		uint8_t ld = 0;
		reader->readByte(&ld,bitNum);
		readedBitNum += bitNum;
		return ld;
	}
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	uint16_t getShort(uint8_t bitNum ){
		uint16_t ld = 0;
		reader->readShort(&ld,bitNum);
		readedBitNum += bitNum;
		return ld;
	}
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	uint32_t getInt(uint8_t bitNum){
		uint32_t ld = 0;
		reader->readInt(&ld,bitNum);
		readedBitNum += bitNum;
		return ld;
	}
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	uint8_t* getBits(uint8_t bitNum){
		uint8_t* dst;
		uint8_t byteAddOne = bitNum%8==0?0:1;
		uint8_t byteNum = bitNum /8 +byteAddOne;
		dst = new uint8_t[byteNum];
		reader->read(dst,bitNum);
		readedBitNum += bitNum;
		return dst;
	}
	void byteAlingFull(){
		if(readedBitNum%8 ){
			getByte(8-readedBitNum%8);
		}
	}
	SBitReader * getReader(){return reader;}
public:
	SBitReadableImpl(SBitReader *_reader):reader(_reader),readedBitNum(0){};
};
};
#endif