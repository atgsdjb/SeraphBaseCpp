/*
 *  us_crc.c
 *  uuseedown
 *
 *  Created by wuwg on 09-11-26.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "us_crc.h"
#include "us_log.h"


static uLong crc_table[256];

/*
 * This routine writes each crc_table entry exactly once,
 * with the ccorrect final value.  Thus, it is safe to call
 * even on a table that someone else is using concurrently.
 */
void UUSee_crc32_init ()
{
	unsigned int i, j;
	uLong h = 1;
	crc_table[0] = 0;
	for (i = 128; i; i >>= 1)
	{
		h = (h >> 1) ^ ((h & 1) ? POLYNOMIAL : 0);
		/* h is now crc_table[i] */
		for (j = 0; j < 256; j += 2 * i)
			crc_table[i + j] = crc_table[j] ^ h;
	}
}

/*
 * This computes the standard preset and inverted CRC, as used
 * by most networking standards.  Start by passing in an initial
 * chaining value of 0, and then pass in the return value from the
 * previous crc32() call.  The final return value is the CRC.
 * Note that this is a little-endian CRC, which is best used with
 * data transmitted lsbit-first, and it should, itself, be appended
 * to data in little-endian byte and bit order to preserve the
 * property of detecting all burst errors of length 32 bits or less.
 */
uLong UUSee_crc32 (uLong crc, const char *buf, unsigned int len)
{
	UUSee_Assert(crc_table[255] != 0);
	//crc ^= 0xffffffff;
	crc = 0xffffffff;
	while (len--)
		crc = (crc >> 8) ^ crc_table[(crc ^ *buf++) & 0xff];
	return crc ^ 0xffffffff;
}


/**
 * Compute the CRC32 checksum for the first len bytes of the buffer.
 *
 * @param buf the data over which we're taking the CRC
 * @param len the length of the buffer
 * @return the resulting CRC32 checksum
 */
#if 0
static int UUSee_crc32_n (const void *buf, size_t len)
{
	uLong crc;
	crc = crc32 (0L, Z_NULL, 0);
	crc = crc32 (crc, (char *) buf, len);
	return crc;
}
#endif
/* end of crc32.c */

