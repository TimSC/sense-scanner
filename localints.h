#ifndef __LOCALINTS_H_
#define __LOCALINTS_H_

#ifdef _MSC_VER
	#define uint8_t unsigned char
	#define uint64_t unsigned __int64
	#define int64_t __int64
#else
	#include <inttypes.h>
#endif

#endif //__LOCALINTS_H_