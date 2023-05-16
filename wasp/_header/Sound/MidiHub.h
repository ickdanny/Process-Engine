#pragma once

#include "MidiSequencer.h"

namespace wasp::sound::midi {
	
	class MidiHub {
	private:
		//fields
		MidiOut midiOut {};
		MidiSequencer midiSequencer;    //not initialized!
		bool muted {};
	
	public:
		MidiHub(bool muted = false);
		
		//delete copy constructor and assignment operator
		MidiHub(const MidiHub& other) = delete;
		
		void operator=(const MidiHub& other) = delete;
		
		void start(std::shared_ptr<MidiSequence> midiSequencePointer);
		
		void stop();
		
		void toggleMute();
		
		bool isMuted() {
			return muted;
		}
	};
}