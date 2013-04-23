#include<cstdio>
#include<cstdint>
namespace Seraphim{
	class NaluHelp{
	private :
		FILE* file;
		uint8_t* buf;
		uint8_t* postion;
		uint8_t* end_buf;
		size_t maxSize;
		bool fullBuffer();
		bool checkEmpty();
	public :
		NaluHelp(char* fileName,size_t maxSize=1024*512);
		uint8_t* nextNALU();
		~NaluHelp(){delete[] buf;fclose(file);}
	};

};
