#pragma once

#include "MidiSequence.h"

namespace wasp::sound::midi {

	MidiSequence parseMidiFile(
		const std::wstring& fileName
	);

	MidiSequence parseLoopedMidiFile(
		const std::wstring& fileName,
		int64_t loopStart = 0,
		int64_t loopEnd = -1
	);
}