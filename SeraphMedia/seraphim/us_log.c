/*
 *  us_log.c
 *  uuseedown
 *
 *  Created by wuwg on 09-11-19.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "us_log.h"
#include <time.h>
#include <stdio.h>
#include "pthread.h"
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <assert.h>

#ifdef SHOW_UUSEE_LOG
extern char* g_path;
void getCurTime(char *_cur_time)
{
	/*struct tm *tt2;
	struct timeval tstruct1;
	struct timezone tzp;
	gettimeofday(&tstruct1,&tzp);
	tt2 = localtime(&tstruct1.tv_sec);
	sprintf(_cur_time,"%04d%02d%02d %02d:%02d:%02d.%06d",
			tt2->tm_year+1900,tt2->tm_mon+1,tt2->tm_mday,tt2->tm_hour,tt2->tm_min,tt2->tm_sec,tstruct1.tv_usec);*/
}



void td_printf(const char* fmt,...){
	const int MAX_DBG_STR = 1024;
	char buffer[1024]={0};
	char  buf[18];
	int thepid;
	int tid;
	FILE *myLog = NULL;
	    va_list va;
	    va_start( va, fmt );
	    vsprintf( &buffer[0], fmt, va);
	    va_end(va);
		
		
		memset(buffer,0,MAX_DBG_STR);

		myLog = fopen("seraph.log","a+");
		if (myLog != NULL)
		{
			char tmp[50] = {0};
			getCurTime((char *)tmp);

			 thepid= 110;
			
			memset( buf, 0x0, sizeof( buf ) );
			sprintf(buf,sizeof(buf),"%d",thepid );
			fputs("[PID:",myLog);
			fputs(buf,myLog);
			fputs("]",myLog);
			tid = 110;//pthread_self();
			memset( buf, 0x0, sizeof( buf ) );
			snprintf(buf,sizeof(buf),"%d", (unsigned int)tid);
			fputs("[TID:",myLog);
			fputs(buf,myLog);
			fputs("]",myLog);
			fputs(tmp,myLog);
			fputs("  ",myLog);
			fputs(buffer,myLog);
		}else{
		}
		fclose(myLog);

}
#endif
