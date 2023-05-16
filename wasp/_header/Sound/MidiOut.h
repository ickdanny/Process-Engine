#pragma once

#include <cstdint>

#include "windowsInclude.h"
#include "mmeInclude.h"

namespace wasp::sound::midi {
	class MidiOut {
	private:
		HMIDIOUT midiOutHandle {};
	
	public:
		//constructor
		MidiOut();
		
		//destructor
		~MidiOut();
		
		//delete copy and assignment
		MidiOut(const MidiOut& other) = delete;
		
		void operator=(const MidiOut& other) = delete;
		
		//output wrapper functions
		void outputShortMsg(uint32_t output);
		
		void outputShortMsgOnAllChannels(uint32_t output);
		
		void outputControlChangeOnAllChannels(uint32_t data);
		
		void outputSystemExclusive(MIDIHDR* midiHDR);
		
		void outputReset();
	
	private:
		void openMidiOut();
		
		void closeMidiOut();
	};
}