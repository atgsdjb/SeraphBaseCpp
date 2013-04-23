#ifndef _SERAPHIM_SYNC_BUFFER_H
#define _SERAPHIM_SYNC_BUFFER_H
extern"C"{
#include"pthread.h""
#include<stdint.h>
};
#include<queue>
using namespace std;
namespace Seraphim{
	class SyncBuffer{
	private:
		pthread_mutex_t mutex;
		pthread_cond_t  cond;
		pthread_mutex_t condMutex;
		queue<uint8_t*> d_buf;
		queue<int>		size_buf;
		bool endFlg;
	public:
		void write23(uint8_t* src,size_t size);
		int	 read(uint8_t** dst); 
		void disable();
		SyncBuffer();
		~SyncBuffer();		
	};
};
#endif