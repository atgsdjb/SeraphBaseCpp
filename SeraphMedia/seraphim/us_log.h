/*
 *  us_log.h
 *  uuseedown
 *
 *  Created by wuwg on 09-11-19.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _US_LOG_H_
#define _US_LOG_H_

#ifdef __cplusplus
extern "C" {
#endif
	
//#define SHOW_UUSEE_LOG
	
#if !defined(__IPHONE__)
	#define UUSEE_LOG_PATH "/var/mobile/appLog"
#else
	#define UUSEE_LOG_PATH "/Users/wuwg/appLog"
#endif

#ifndef SHOW_UUSEE_LOG
	
	#define UUSee_Assert(cOND)
	#define UUSee_Printf(...)
	#define UUSee_Printf_Hex(fragment,length,name)
	#define td_printf(fmt,...)
#else 
	
	void td_printf(char* fmt,...);
	#define UUSee_Assert(cOND)			\
	{                                                \
	if (!(cOND))                                     \
	{                                                \
	UUSee_AssertFail(#cOND, __FILE__, __LINE__);   \
	}                                                \
	}
	
#endif 	
	
#ifdef __cplusplus
}
#endif


#endif
