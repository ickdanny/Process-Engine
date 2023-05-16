#include "Sound/MidiHub.h"

namespace wasp::sound::midi {
	
	MidiHub::MidiHub(bool muted)
		: midiOut {}, midiSequencer { &midiOut }, muted { muted } {
	}
	
	void MidiHub::start(std::shared_ptr<MidiSequence> midiSequencePointer) {
		if( !muted ) {
			midiSequencer.start(midiSequencePointer);
		}
	}
	
	void MidiHub::stop() {
		midiSequencer.stop();
	}
	
	void MidiHub::toggleMute() {
		//if we are now muted
		if( muted = !muted ) {
			stop();
		}
	}
}