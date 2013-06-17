/*
 *  us_log.c
 *  uuseedown
 *
 *  Created by wuwg on 10-12-30.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "us_log.h"
#include <sys/time.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <assert.h>
//#include "UUSeeCastAppDelegate.h"
#include <arpa/inet.h>


#ifdef SHOW_UUSEE_LOG

void getCurTime(char *_cur_time)
{
	struct tm *tt2;
	struct timeval tstruct1;
	struct timezone tzp;
	gettimeofday(&tstruct1,&tzp);
	tt2 = localtime(&tstruct1.tv_sec);
	sprintf(_cur_time,"%04d%02d%02d %02d:%02d:%02d.%06d",
			tt2->tm_year+1900,tt2->tm_mon+1,tt2->tm_mday,tt2->tm_hour,tt2->tm_min,tt2->tm_sec,tstruct1.tv_usec);
}


void ExportLog(const char* str)
{
	
	FILE *myLog = NULL;
	myLog = fopen("./appLog","a+");
	
	if (myLog != NULL) 
	{		
		char tmp[50] = {0};
		getCurTime((char *)tmp);
		
		int thepid = (int)getpid();
		char  buf[18];
		memset( buf, 0x0, sizeof( buf ) );
		snprintf(buf,sizeof(buf),"%d",thepid );
		fputs("[PID:",myLog);
		fputs(buf,myLog);
		fputs("]",myLog);
		
		pthread_t tid = pthread_self();
		memset( buf, 0x0, sizeof( buf ) );
		snprintf(buf,sizeof(buf),"%d", (unsigned int)tid);
		fputs("[TID:",myLog);
		fputs(buf,myLog);
		fputs("]",myLog);
		
		fputs(tmp,myLog);
		fputs("  ",myLog);
		fputs(str,myLog);
		//fputs("\n",myLog);
	}
	fclose(myLog);
	
}

void UUSee_Printf_Hex(const char *fragment, int length, const char *name)
{

    const unsigned char *s;
    const unsigned short data_per_line=16;
    int i, j;
    unsigned char *c1, *c2, buf[256];
	
    if (fragment==NULL)
		return;
	
    UUSee_Printf("  %s at 0x%x.",name, (unsigned int)fragment);
    UUSee_Printf("      length: %d byte%s\r\n", length, (length>0)?"s":"");
	
    s=fragment;
    j=length;
    while (j>0) {
		memset(buf, ' ',256);
		/* c1+=sprintf(c1=buf, "    data: "); */
		memcpy(buf, "    data: ", 11);
		c1=(buf+10);
		c2=c1+(3*data_per_line)+1;
		for (i=((j>data_per_line)?data_per_line:j); i>0; i--, j--) {
			*c1=(*s>>4); *(c1)+=(*c1<0x0a)?'0':('a'-0x0a); c1++;
			*c1=(*s&0x0f); *(c1)+=(*c1<0x0a)?'0':('a'-0x0a); c1++;
			*c1++=' ';
			if (isprint(*s))
				*c2++=*s;
			else
				*c2++='.';
			s++;
		}
		*c2=0;
		UUSee_Printf("%s\r\n",buf);
    }
}


void UUSee_Printf(const char* fmt, ... )
{	
    const int MAX_DBG_STR = 1024;
	int thepid = (int)getpid();
	pthread_t tid = pthread_self();
	
    int written = 0;
    char buffer[MAX_DBG_STR];
	memset(buffer,0,MAX_DBG_STR);
	
	char tmp[50] = {0};
	getCurTime((char *)tmp);
//	memcpy(buffer, tmp, strlen(tmp));
//	memcpy(buffer + strlen(tmp), " ", 1);
	sprintf(buffer, "%s[%d:%d]",tmp,thepid,tid);
	
    va_list va;
    va_start( va, fmt );
    //written = vsprintf( &buffer[strlen(tmp)+1], fmt, va);
	written = vsprintf( &buffer[strlen(buffer)], fmt, va);
    va_end(va);
	
	
	
//	if(strlen(buffer))
//		ExportLog(buffer);
	if(strlen(buffer)) {
		printf(buffer);
	}
	
//	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
//	UUSeeCastAppDelegate *dele = (UUSeeCastAppDelegate *)[[UIApplication sharedApplication] delegate];
//	NSString *newStr = [[[NSString alloc] initWithCString:buffer encoding:NSASCIIStringEncoding] autorelease];
//	[dele performSelectorOnMainThread:@selector(UUSEE_LOG:) withObject:newStr waitUntilDone:NO];
//	[pool release];
	
}
void UUSeeLog(const char* fmt, ... )
{	
    
	const int MAX_DBG_STR = 1024;

	    int written = 0;
	    char buffer[MAX_DBG_STR];
		memset(buffer,0,MAX_DBG_STR);

	    va_list va;
	    va_start( va, fmt );
	    written = vsprintf( &buffer[0], fmt, va);
	    va_end(va);
		FILE *myLog = NULL;
		myLog = fopen("/mnt/sdcard/seraphim/seraphim_Upload.log","a+");
		if (myLog != NULL)
		{
			char tmp[50] = {0};
			getCurTime((char *)tmp);

			int thepid = (int)getpid();
			char  buf[18];
			memset( buf, 0x0, sizeof( buf ) );
			snprintf(buf,sizeof(buf),"%d",thepid );
			fputs("[PID:",myLog);
			fputs(buf,myLog);
			fputs("]",myLog);
			pthread_t tid = pthread_self();
			memset( buf, 0x0, sizeof( buf ) );
			snprintf(buf,sizeof(buf),"%d", (unsigned int)tid);
			fputs("[TID:",myLog);
			fputs(buf,myLog);
			fputs("]",myLog);
			fputs(tmp,myLog);
			fputs("  ",myLog);
			fputs(buffer,myLog);
		}else{
			//__android_log_write(1,"PORT","LOGError");
		}
		fclose(myLog);
	
}

void UUSee_AssertFail(const char *cond, const char *file, int line)
{
	printf("Assert Fail: %s, #%d, (%s)\n",file, line, cond);
	UUSee_Printf("Assert Fail: %s, #%d, (%s)\n",file, line, cond);
	assert(0);
}

static void sendLog (char* param) {
//	socklen_t addrlen = 0;
//	//连接server
//	struct sockaddr_in server_sockaddr;
//	server_sockaddr.sin_family = AF_INET;
//	server_sockaddr.sin_port = htons(5050);
//	server_sockaddr.sin_addr.s_addr = inet_addr("219.237.222.3");
//	bzero(&(server_sockaddr.sin_zero), 8);
//	addrlen =sizeof(server_sockaddr);
//
//	//超时
//	struct timeval sndtimeout;
//    sndtimeout.tv_sec = 0;
//    sndtimeout.tv_usec = 200 * 1000;   //200ms
//
//	//创建socket
//	int sc = socket( AF_INET, SOCK_DGRAM, 0 );
//	if (sc == -1) {
//		return;
//	}
//
//	//设置成非阻塞模式
//	//	fcntl( sc, F_SETFL, O_NONBLOCK );
//	//    fcntl( sc, F_GETFL, 0 );
//	//	int ret = setsockopt(sc,SOL_SOCKET,SO_SNDTIMEO,(BYTE *)(&sndtimeout),sizeof(struct timeval));
//	//printf("len:%d",strlen(param));
//	sendto(sc, param,strlen(param), 0, (struct sockaddr* )&server_sockaddr, addrlen);
//
//	close(sc);
}

void UUSee_SendLog(const char* fmt, ... )
{	
	
    const int MAX_DBG_STR = 1024;
	//	int thepid = (int)getpid();
	//	pthread_t tid = pthread_self();
	
    int written = 0;
    char buffer[MAX_DBG_STR];
	memset(buffer,0,MAX_DBG_STR);
	
	char tmp[50] = {0};
	getCurTime((char *)tmp);
	//	memcpy(buffer, tmp, strlen(tmp));
	//	memcpy(buffer + strlen(tmp), " ", 1);
	//sprintf(buffer, "%s[%d:%d]",tmp,thepid,tid);
	sprintf(buffer, "[%s]",tmp);
	
    va_list va;
    va_start( va, fmt );
    //written = vsprintf( &buffer[strlen(tmp)+1], fmt, va);
	written = vsprintf( &buffer[strlen(buffer)], fmt, va);
    va_end(va);
	
	sendLog(buffer);
	
}


#endif
