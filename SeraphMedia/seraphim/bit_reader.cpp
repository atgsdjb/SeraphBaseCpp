#include"bit_reader.h"
#include"s_config.h"
namespace Seraphim{
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/

	inline static int getByteNumOfBitNum(int bitNum){
		int len = -1;
		if(bitNum<=8){
			len = 1;
		}else if(bitNum <=16){
			len = 2;
		}else if(bitNum <=32){
			len =4;
		}else if(bitNum <=64){
			len = 8;
		}
		return len;
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	inline static uint8_t getHClearMask(uint8_t num){
		uint8_t i = 0;
		uint8_t result = 0x00;
		for(i;i<num;i++){
			result >>=1;
			result |= 0x80;
		}
		return ~result;
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	inline static uint8_t getLClearMask(uint8_t num){
		uint8_t i = 0;
		uint8_t result = 0x00;
		for(i;i<num;i++){
			result <<=1;
			result |= 0x01;
		}
		return ~result;
	}


	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	inline static uint8_t getHSetMask(uint8_t num){
		uint8_t i = 0;
		uint8_t result = 0x00;
		for(i;i<num;i++){
			result >>=1;
			result |= 0x80;
		}
		return result;
	}
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	inline static uint8_t getLSetMask(uint8_t num){
		uint8_t i = 0;
		uint8_t result = 0x00;
		for(i;i<num;i++){
			result <<=1;
			result |= 0x01;
		}
		return result;
	}
	/************************************************************************/
	/*                default------BIGEND;positionBit from HigBit begin*/
	/************************************************************************/
	int SBitReader::read(uint8_t* dst,int bitNum){
		int l_byteNum = getByteNumOfBitNum(bitNum)-1;
		uint8_t t_byteIndex=0;
		while(bitNum>=8){
			uint8_t t_d = 0x00;
			if(positionBit==0){
					t_d = buf[positionByte++] ;
			}else{
				t_d = (buf[positionByte]<<positionBit) |  (buf[positionByte+1]>>(8-positionBit)  )  ;//& getLClearMask(8-positionBit));
				positionByte++;
			}
			dst[l_byteNum--] = t_d;
			bitNum -=8;
		}
		if(bitNum>0 && 8 >=(bitNum+positionBit)){
			dst[l_byteNum] =  (buf[positionByte]& getHClearMask(positionByte)) >> (8-bitNum-positionBit) ; //_l;// >> (8-bitNum);
			positionBit +=bitNum;
			if(positionBit ==8){
				positionBit =0;
				positionByte++;
			}
		}else if(bitNum>0){
			dst[l_byteNum] =   ( buf[positionByte] & getHClearMask(positionBit) ) << (bitNum -(8 - positionBit)) 
								| ( buf[positionByte+1] >>(16-bitNum-positionBit) );
			positionByte++;
			positionBit = bitNum+positionBit -8;
		}
		ASSERT(positionBit<=7);
		return (len-positionByte)*8-positionBit;
	}
	int SBitReader::readByte(uint8_t* dst){
		return (len-positionByte)*8-positionBit;
	}
	int SBitReader::readInt(uint32_t* dst,bool isBigEnd){
		uint8_t t_b[4]={0,0,0,0};
		int t_d = 0x00000000;
		int i =0;
		read(t_b,sizeof(int));
		if(isBigEnd){
			while(true){
				t_d |= t_b[i++];
				if(i==sizeof(int)){
					break;
				}
				t_d <<=8;
			}
		}else{

		}
		return (len-positionByte)*8-positionBit;
	}
	int SBitReader::readShort(uint16_t* dst,bool isBigEnd){
		uint8_t t_b[2]={0,0};
		int t_d = 0x0000;
		int i =0;
		read(t_b,sizeof(int));
		if(isBigEnd){
			while(true){
				t_d |= t_b[i++];
				if(i==sizeof(int)){
					break;
				}
				t_d <<=8;
			}
		}else{

		}
		return (len-positionByte)*8-positionBit;
	}
	int SBitReader::readLong(uint64_t* dst,bool isBigEnd){
		return (len-positionByte)*8-positionBit;
	}
}