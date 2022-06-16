#ifndef _DBS_UTILS_H
#define _DBS_UTILS_H

#include "define.h"


/*
 * Hash a buffer.
 *
 * @buf: A buffer containing the data
 * @len: The number of bytes to hash from the buffer
 *
 * Returns: The hash value
 */
DBS_API unsigned long dbs_hash(unsigned char *buf, int len);

#endif
