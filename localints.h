#ifndef __LOCALINTS_H_
#define __LOCALINTS_H_

/*!
* This file provides platform specific aliases for various
* integer conversions and types
*/

#ifdef _MSC_VER
	typedef unsigned char uint8_t;
	typedef unsigned __int64 uint64_t;
	typedef __int64 int64_t;
#else
	#include <inttypes.h>
#endif

#ifdef _MSC_VER
#define STR_TO_ULL _strtoui64
#else
#define STR_TO_ULL std::strtoull
#endif

#define STR_TO_ULL_SIMPLE atoi //TODO change this to avoid loss of precision
//STR_TO_ULL(ev->data.c_str(),NULL,10);
#endif //__LOCALINTS_H_
