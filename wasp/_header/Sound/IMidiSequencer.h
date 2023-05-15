#pragma once

#include "MidiSequence.h"

namespace wasp::sound::midi {
	class IMidiSequencer {
	public:
		virtual void start(std::shared_ptr<MidiSequence> midiSequencePointer) = 0;
		virtual void stop() = 0;
	};
}