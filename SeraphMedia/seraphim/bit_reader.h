#ifndef __SERAPHIM_BIT_READER_H
#define  __SERAPHIM_BIT_READER_H
#include<stdint.h>
namespace Seraphim{
	class SBitReader{
	private:
		uint8_t* buf;
		size_t   len;
		size_t   positionByte;
		uint8_t  positionBit;
		bool isBigEnd;
	public:
		SBitReader(uint8_t* _buf,size_t _len,bool _isBigEnd=true):buf(_buf),len(_len),positionByte(0),positionBit(0),isBigEnd(_isBigEnd){};
		int read(uint8_t* dst,int bitNum);
		int readByte(uint8_t* dst);
		int readInt(uint32_t* dst);
		int readShort(uint16_t* dst);
		int readLong(uint64_t* dst);
		~SBitReader(){delete[] buf;};
	};
};



#endif