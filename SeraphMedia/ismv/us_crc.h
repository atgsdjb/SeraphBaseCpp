/*
 *  us_crc.h
 *  uuseedown
 *
 *  Created by wuwg on 09-11-26.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _US_CRC_H_
#define _US_CRC_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Avoid wasting space on 8-byte longs. */
//#if UINT_MAX >= 0xffffffff
//	typedef unsigned int uLong;
//#elif ULONG_MAX >= 0xffffffff
//	typedef unsigned long uLong;
//#else
//#error This compiler is not ANSI-compliant!
//#endif

typedef unsigned long uLong;

#define Z_NULL  0
#define POLYNOMIAL (uLong)0xedb88320


uLong UUSee_crc32 (uLong crc, const char *buf, unsigned int len);
void UUSee_crc32_init ();

#ifdef __cplusplus
}
#endif


#endif
