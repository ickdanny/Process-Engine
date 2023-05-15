#pragma once

#include <cstdint>
#include <istream>
#include <vector>

namespace wasp::sound::midi {
	struct MidiSequence {

		uint16_t ticks{};

		#pragma pack(push, 1)
		struct EventUnit {
			uint32_t deltaTime{};
			uint32_t event{};
		};
		#pragma pack(pop)

		using EventUnitTrack = std::vector<EventUnit>;
		EventUnitTrack compiledTrack{};

		//we encode a loop point as a tempo metaevent followed by this block
		//the literal "0x6C6F6F70" spells out "loop" in ascii
		static constexpr EventUnit loopEncoding{ 0x6C6F6F70, 0 };
	};
}