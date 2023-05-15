#pragma once

#include <cstdint>
#include <type_traits>

namespace wasp::utility {
	uint16_t byteSwap16(uint16_t i);
	uint32_t byteSwap32(uint32_t i);
	uint64_t byteSwap64(uint64_t i);

	//not zero indexed!
	template <typename IntType>
	uint8_t getByte(IntType i, int byteNumber) {
		static_assert(std::is_integral<IntType>::value, "Integral required!");
		//right shift by (byteNumber - 1) bytes, then truncate
		return static_cast<uint8_t>(i >> ((byteNumber - 1) * 8));
	}
}