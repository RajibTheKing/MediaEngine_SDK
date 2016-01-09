
#ifndef _TYPE_DEFS_H_
#define _TYPE_DEFS_H_

#include <limits.h>
#include <stddef.h>

#ifndef  _MSC_VER

#define __STDC_FORMAT_MACROS
#include <stdint.h>
#include <inttypes.h>

#ifdef __LP64__
typedef int64_t intX_t;
#else
typedef int32_t intX_t;
#endif

#else

typedef signed char      int8_t  ;
typedef unsigned char    uint8_t ;
typedef short            int16_t ;
typedef unsigned short   uint16_t;
typedef int              int32_t ;
typedef unsigned int     uint32_t;
typedef __int64          int64_t ;
typedef unsigned __int64 uint64_t;
#define PRId64 "I64d"

#ifdef _WIN64
typedef int64_t intX_t;
#else
typedef int32_t intX_t;
#endif

#endif 

#ifdef EPSN
#undef EPSN
#endif
#define EPSN (0.000001f) 

#endif 
