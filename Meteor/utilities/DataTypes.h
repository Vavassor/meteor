#ifndef __DATA_TYPES_H__
#define __DATA_TYPES_H__

#if _MSC_VER >= 1600 || __GNUC__ >= 3
	#include <cstdint>
#else
    typedef __int8              int8_t;
    typedef __int16             int16_t;
    typedef __int32             int32_t;
    typedef __int64             int64_t;
	typedef __int64				intptr_t;
    typedef unsigned __int8     uint8_t;
    typedef unsigned __int16    uint16_t;
    typedef unsigned __int32    uint32_t;
    typedef unsigned __int64    uint64_t;
	typedef unsigned __int64	uintptr_t;
#endif

#endif
