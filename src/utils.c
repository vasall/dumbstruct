#include "utils.h"

#include <stdlib.h>


DBS_API u64 dbs_hash(u8 *buf, s32 len)
{
	u64 hash = 5381;
	s32 i;

	for(i = 0; i < len; i++) {
		hash = ((hash << 5) + hash) + buf[i];
	}

	return hash;
}
