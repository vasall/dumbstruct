#include "utils.h"

#include <stdlib.h>


DBS_API unsigned long dbs_hash(unsigned char *buf, int len)
{
	unsigned long hash = 5381;
	int c;
	int i;

	for(i = 0; i < len; i++) {
		hash = ((hash << 5) + hash) + buf[i];
	}

	return hash;
}
