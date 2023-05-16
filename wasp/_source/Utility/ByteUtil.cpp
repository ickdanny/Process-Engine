#include "Utility\ByteUtil.h"

#include <intrin.h>

namespace wasp::utility {
	uint16_t byteSwap16(uint16_t i) {
		return _byteswap_ushort(i);
	}
	
	uint32_t byteSwap32(uint32_t i) {
		return _byteswap_ulong(i);
	}
	
	uint64_t byteSwap64(uint64_t i) {
		return _byteswap_uint64(i);
	}
}