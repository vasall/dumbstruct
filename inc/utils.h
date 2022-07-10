#ifndef _DBS_UTILS_H
#define _DBS_UTILS_H

#include "define.h"
#include "imports.h"

/*
 * Hash a buffer.
 *
 * @buf: A buffer containing the data
 * @len: The number of bytes to hash from the buffer
 *
 * Returns: The hash value
 */
DBS_API u64 dbs_hash(u8 *buf, s32 len);

#endif
