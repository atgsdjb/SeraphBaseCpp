//
//#include<stdio.h>
//#include<stdlib.h>
//#include"us_down.h"
//#include<string.h>
//#include<pthread.h>
//#include"us_task.h"
//#include<stdlib.h>
//#include<string.h>
//#include <signal.h>
//#include <sys/wait.h>
//#include <unistd.h>
//#include <sys/time.h>
//#include <ctype.h>
//#include <stdarg.h>
//#include<sys/time.h>
//#include<sys/resource.h>
//
////typedef unsigned int uint32_t;
//
//typedef struct  TPackageData
//{
//    uint32_t type;    //vod data
//    uint32_t OffSet;  //vod file offset (out) live sendtime
//    char * Data;      //data buffer pointer(in)
//    uint32_t MaxLen;  //buffer max len(in)
//    uint32_t ActLen;  //actual len (out)
//    uint32_t ID;
//}TPackageData,* PPackageData;
//static int exit_flag;
//static char gtime[50] = {0};
//char* getCurTime()
//{
//	struct tm *tt2;
//	struct timeval tstruct1;
//	struct timezone tzp;
//	gettimeofday(&tstruct1,&tzp);
//	tt2 = localtime(&tstruct1.tv_sec);
//	memset(gtime,0,50);
//	sprintf(gtime,"%04d%02d%02d %02d:%02d:%02d.%06d",
//			tt2->tm_year+1900,tt2->tm_mon+1,tt2->tm_mday,tt2->tm_hour,tt2->tm_min,tt2->tm_sec,tstruct1.tv_usec);
//	return gtime;
//}
//
//void UUSee_Printf(const char* fmt, ... )
//{
//    const int MAX_DBG_STR = 2048;
//
//    int written = 0;
//    char buffer[MAX_DBG_STR];
//	memset(buffer,0,MAX_DBG_STR);
//
//    va_list va;
//    va_start( va, fmt );
//    written = vsprintf( &buffer[0], fmt, va);
//    va_end(va);
//
//	if(strlen(buffer))
//		printf("[%s]%s",getCurTime(),buffer);
//}
//
//static void signal_handler(int sig_num)
//{
//	if (sig_num == SIGCHLD) {
//		do {
//		} while (waitpid(-1, &sig_num, WNOHANG) > 0);
//	} else
//	{
//		exit_flag = sig_num;
//	}
//}
//
//void* run_uusee_p2p(void* param)
//{
//	TPackageData t;
//	char info[1024]={0};
//	char status[1024] = {0};
//
//	int count = 0;
//	int count2 = 0;
//	char* g_pbuffer = (char*)malloc(500*1024);
//
//	strcpy(info,"|UdpTaskStep=36|BufferSegmentCount=20|PeerMaxCount=30|TrackerCount=3|TrackerIP1=60.191.221.143|TrackerPort1=8888|TrackerIP2=119.161.149.138|TrackerPort2=8888|TrackerIP3=125.39.159.122|TrackerPort3=8888|HttpServerCount=4|HttpServerUrl1=l1.51v8.cn:8088/[/1|HttpServerUrl2=l2.51v8.cn:8088/[/1|HttpServerUrl3=l3.51v8.cn:8088/[/1|HttpServerUrl4=l4.51v8.cn:8088/[/1|");
//
//	//��ʼ������ģ��
//	CORE_init(0);
//
//
//	//CORE_play(1,"{A3818557-0000-0000-FFFF-FFFFFFFFFF14}",38, info, 0, "",0);
//	CORE_play(1,"{C171F740-0000-0000-FFFF-FFFFFFFFFF14}",38, info, 0, "",0);
//	while(1)
//	{
//		memset(g_pbuffer,0,500*1024);
//		memset(&t,0,sizeof(TPackageData));
//
//		t.Data = (char*)g_pbuffer;
//		t.MaxLen = 500*1024;
//		count++;
//		if (count%15 == 0) {
//			count2++;
//			UUSee_Printf("==========CORE_stop====in========\n");
//			CORE_stop();
//			UUSee_Printf("==========CORE_stop====out========\n");
//			sleep(1);
//			if (count2%2 == 0) {
//				CORE_play(1,"{C171F740-0000-0000-FFFF-FFFFFFFFFF14}",38, info, 0, "",0);
//			} else {
//				CORE_play(1,"{A3818557-0000-0000-FFFF-FFFFFFFFFF14}",38, info, 0, "",0);
//			}
//			UUSee_Printf("==========CORE_play====out========\n");
//		}
//		CORE_getdata(&t);
//		if(t.ActLen > 0)
//		{
//			UUSee_Printf("---------------------get %d \n",t.ID);
//		}
//
//		CORE_getstatusEx(status,1024);
//		UUSee_Printf("%s\n",status);
//
//		sleep(1);
//	}
//
//	return NULL;
//}
//int main(int argc, char *argv[]) {
//	pthread_t id;
//	int ret;
//	struct rlimit coredump;
//	memset(&coredump, 0, sizeof(struct rlimit));
//	coredump.rlim_cur = RLIM_INFINITY;
//	coredump.rlim_max = RLIM_INFINITY;
//	setrlimit(RLIMIT_CORE, &coredump);
//
//	signal(SIGCHLD, signal_handler);
//	signal(SIGTERM, signal_handler);
//	signal(SIGINT, signal_handler);
//	signal(SIGHUP, signal_handler);
//	signal(SIGILL, signal_handler);
//
//	ret=pthread_create(&id,NULL,run_uusee_p2p,NULL);
//
//	if(ret!=0){
//		UUSee_Printf ("Create pthread error!\n");
//		exit (1);
//	}
//	pthread_detach(id);
//
//	while (exit_flag == 0) {
//		sleep(1);
//	}
//	UUSee_Printf ("exit_flg = %d\n",exit_flag);
//	return 0;
//}
//
#include"Patch264.h"
#include<stdio.h>
int main(int argc,char** argv){
	
	segment_main(argv[1],"/home/root/video/test.ismv");
	
	return 0;
}
