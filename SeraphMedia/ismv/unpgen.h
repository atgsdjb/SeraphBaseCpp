#ifndef _US_UNP_H_
#define _US_UNP_H_

#ifdef __cplusplus
extern "C" {
#endif
	
#include "unpdef.h"

int init();
bool genunp(int obj,BYTE type,int subtype,char * buf,unsigned int len,char * unpbuf,unsigned int * unplen);
int stop(int obj);

#ifdef __cplusplus
}
#endif

#endif