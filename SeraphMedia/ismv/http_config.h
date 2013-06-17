#ifndef __HTTP_CONFIG
#define __HTP_CONFIG
#include<pthread.h>
extern pthread_mutex_t  mutex_port;
extern pthread_cond_t   cond_port;
extern int  g_port;
extern char *httppath;
#endif
