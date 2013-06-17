/*
 *  us_log.h
 *  uuseedown
 *
 *  Created by wuwg on 10-12-30.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _US_LOG_H_
#define _US_LOG_H_

#define SHOW_UUSEE_LOG

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SHOW_UUSEE_LOG
	
	#define UUSee_Assert(cOND) \
	{                                                \
	if (!(cOND))                                     \
	{                                                \
	}                                                \
	}
	
	#define UUSee_Printf(...)
	#define UUSee_Printf_Hex(fragment,length,name)
	#define UUSee_SendLog(...)
	void UUSeeLog(const char* fmt, ... )
	
#else 
	void UUSeeLog(const char* fmt, ... );
	void UUSee_AssertFail(const char *cond, const char *file, int line);
	void UUSee_Printf(const char* fmt, ... );
	void UUSee_Printf_Hex(const char *fragment, int length, const char *name);
	void UUSee_SendLog(const char* fmt, ... );
	
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