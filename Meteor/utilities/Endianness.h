#ifndef ENDIANNESS_H
#define ENDIANNESS_H

// SYSTEM-HEADER MACROS
//----------------------------------------------------------------------------------------------------

#if defined(__APPLE__) && defined(__MACH__)
#include <machine/endian.h>

#elif defined(__linux__)
#include <endian.h>

#elif defined(__FreeBSD__)\
	|| defined(__OpenBSD__)\
	|| defined(__NetBSD__)\
	|| defined(__DragonFly__)
#include <sys/types.h>
#include <sys/endian.h>
#endif

#if defined(__BYTE_ORDER)

#if (__BYTE_ORDER == __BIG_ENDIAN)
#define	ENDIAN_BIG	0x04030201
#elif (__BYTE_ORDER == __LITTLE_ENDIAN)
#define	ENDIAN_LITTLE	0x01020304
#elif (__BYTE_ORDER == __PDP_ENDIAN)
#define	ENDIAN_PDP	0x03040102
#endif

#elif defined(_BYTE_ORDER)

#if (_BYTE_ORDER == _BIG_ENDIAN)
#define	ENDIAN_BIG	0x04030201
#elif (_BYTE_ORDER == _LITTLE_ENDIAN)
#define	ENDIAN_LITTLE	0x01020304
#elif (_BYTE_ORDER == _PDP_ENDIAN)
#define	ENDIAN_PDP	0x03040102
#endif

#elif defined(BYTE_ORDER)

#if (BYTE_ORDER == BIG_ENDIAN)
#define	ENDIAN_BIG	0x04030201
#elif (BYTE_ORDER == LITTLE_ENDIAN)
#define	ENDIAN_LITTLE	0x01020304
#elif (BYTE_ORDER == PDP_ENDIAN)
#define	ENDIAN_PDP	0x03040102
#endif

#endif // defined(BYTE_ORDER)

// PRE-DEFINED COMPILER MACROS
//----------------------------------------------------------------------------------------------------

#if !defined(ENDIAN_BIG)

#if defined(__BIG_ENDIAN__)\
	|| defined(__ARMEB__)\
	|| defined(__THUMBEB__)\
	|| defined(__AARCH64EB__)\
	|| defined(_MIPSEB)\
	|| defined(__MIPSEB)\
	|| defined(__MIPSEB__)
#undef	ENDIAN_BIG
#define ENDIAN_BIG	0x04030201
#endif

#endif // !defined(ENDIAN_BIG)

#if !defined(ENDIAN_LITTLE)

#if defined(__LITTLE_ENDIAN__)\
	|| defined(__ARMEL__)\
	|| defined(__THUMBEL__)\
	|| defined(__AARCH64EL__)\
	|| defined(_MIPSEL)\
	|| defined(__MIPSEL)\
	|| defined(__MIPSEL__)
#undef	ENDIAN_LITTLE
#define ENDIAN_LITTLE	0x01020304
#endif

#endif // !defined(ENDIAN_LITTLE)

#if !defined(ENDIAN_LITTLE) && defined(_WIN32)
#define	ENDIAN_LITTLE 0x01020304
#endif

// RUN-TIME UTILITIES
//----------------------------------------------------------------------------------------------------

enum Endianness
{
	ENDIANNESS_BIG,
	ENDIANNESS_LITTLE,
	ENDIANNESS_PDP,
	ENDIANNESS_UNKNOWN,
};

Endianness check_endianness();

#endif
