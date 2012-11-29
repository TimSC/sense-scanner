#ifndef __LOCALINTS_H_
#define __LOCALINTS_H_

#ifdef _MSC_VER
	typedef unsigned char uint8_t;
	typedef unsigned __int64 uint64_t;
	typedef __int64 int64_t;
#else
	#include <inttypes.h>
#endif

#endif //__LOCALINTS_H_