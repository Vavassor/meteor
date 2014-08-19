#ifndef MEMORY_FENCES_H
#define MEMORY_FENCES_H

// READ_WRITE_BARRIER
#if _MSC_VER
#include <intrin.h>
#pragma intrinsic(_ReadWriteBarrier)
#define READ_WRITE_BARRIER()	_ReadWriteBarrier()

#elif __GNUC__
#define READ_WRITE_BARRIER()	asm volatile("" ::: "memory")

#elif __INTEL_COMPILER
#define READ_WRITE_BARRIER()	__memory_barrier()

#endif

// MEMORY_FENCE
#if _MSC_VER
#include <WinNT.h>
#define MEMORY_FENCE()	MemoryBarrier()

#elif __GNUC__
#define MEMORY_FENCE()	__sync_synchronize()

#endif

#endif
