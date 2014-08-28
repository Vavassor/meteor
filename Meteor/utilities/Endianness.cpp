#include "Endianness.h"

#include "DataTypes.h"

Endianness check_endianness()
{
	union endian_check
	{
		uint32_t n;
		uint8_t b[4];
	}
	checker = { 0x01020304 };

	return (checker.b[0] == 0x04) ? ENDIANNESS_LITTLE 
		: (checker.b[0] == 0x01) ? ENDIANNESS_BIG 
		: (checker.b[0] == 0x02) ? ENDIANNESS_PDP 
		: ENDIANNESS_UNKNOWN;
}
