#include "mpeg4ip.h"

int gettimeofday(struct timeval *t, void *)
{
	SYSTEMTIME systm;
	GetSystemTime(&systm); 

	t->tv_sec = systm.wSecond;
	t->tv_usec = systm.wMilliseconds * 1000;

	return 0;
}
