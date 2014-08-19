#ifndef INTERLOCKED_H
#define INTERLOCKED_H

#if _MSC_VER
#include <intrin.h>

# pragma intrinsic(_InterlockedIncrement)
# pragma intrinsic(_InterlockedDecrement)
# pragma intrinsic(_InterlockedCompareExchange)

#define INTERLOCKED_INCREMENT(int32_pointer)	_InterlockedIncrement(int32_pointer)
#define INTERLOCKED_DECREMENT(int32_pointer)	_InterlockedDecrement(int32_pointer)
#define INTERLOCKED_COMPARE_AND_SWAP(dest,comperand,value)	_InterlockedCompareExchange(dest,value,comperand)

#elif __GNUC__
#define INTERLOCKED_INCREMENT(int32_pointer)	__sync_add_and_fetch(int32_pointer, 1)
#define INTERLOCKED_DECREMENT(int32_pointer)	__sync_sub_and_fetch(int32_pointer, 1)
#define INTERLOCKED_COMPARE_AND_SWAP(dest,comperand,value)	__sync_val_compare_and_swap(dest,comperand,value)

#endif

#endif
