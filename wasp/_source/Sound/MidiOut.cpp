#include "Sound/MidiOut.h"

#include "Sound/MidiConstants.h"

#include "Logging.h"

namespace wasp::sound::midi {

	MidiOut::MidiOut() {
		openMidiOut();
	}

	void MidiOut::openMidiOut() {
		auto result{
			midiOutOpen(&midiOutHandle, MIDI_MAPPER, 0, 0, CALLBACK_NULL)
		};
		if (result != MMSYSERR_NOERROR) {
			throw std::runtime_error{ "Error opening MIDI Mapper" };
		}
	}

	MidiOut::~MidiOut() {
		//close midi out
		try {
			closeMidiOut();
		}
		catch (const std::exception& error) {
			debug::log(error.what());
		}
		catch (...) {
			//swallow error
		}
	}

	void MidiOut::closeMidiOut() {
		auto result{ midiOutClose(midiOutHandle) };
		if (result != MMSYSERR_NOERROR) {
			throw std::runtime_error{ "Error closing MIDI out" };
		}
	}

	void MidiOut::outputShortMsg(uint32_t output) {
		auto result{
			midiOutShortMsg(midiOutHandle, output)
		};
		if (result != MMSYSERR_NOERROR) {
			throw std::runtime_error{ "Error outputting MIDI short msg" };
		}
	}

	void MidiOut::outputShortMsgOnAllChannels(uint32_t output) {
		//make sure the last 4 bits are empty (it's where we put the channel)
		if (!(output << 28)) {
			for (int i{ 0 }; i <= 0b1111; ++i) {
				outputShortMsg(output + i);
			}
		}
		else {
			throw std::runtime_error{ "Error MIDI short msg must end in 0b0000" };
		}
	}

	void MidiOut::outputControlChangeOnAllChannels(uint32_t data) {
		outputShortMsgOnAllChannels(
			controlChange + (data << 16)
		);
	}

	void MidiOut::outputSystemExclusive(MIDIHDR* midiHDRPointer) {
		auto result{ midiOutPrepareHeader(midiOutHandle, midiHDRPointer, sizeof(MIDIHDR)) };
		if (result != MMSYSERR_NOERROR) {
			throw std::runtime_error{ "Error preparing sysEx" };
		}
		result = midiOutLongMsg(midiOutHandle, midiHDRPointer, sizeof(MIDIHDR));
		if (result != MMSYSERR_NOERROR) {
			throw std::runtime_error{ "Error outputting sysEx" };
		}
		result = midiOutUnprepareHeader(midiOutHandle, midiHDRPointer, sizeof(MIDIHDR));
		if (result != MMSYSERR_NOERROR) {
			throw std::runtime_error{ "Error unpreparing sysEx" };
		}
	}

	void MidiOut::outputReset() {
		outputControlChangeOnAllChannels(allSoundOff);
		auto result{
			midiOutReset(midiOutHandle)
		};
		switch (result) {
			case MMSYSERR_NOERROR:
				break;
			case MMSYSERR_INVALHANDLE:
				throw std::runtime_error{ "Error invalid MIDI out handle" };
			default:
				throw std::runtime_error{ "Error resetting MIDI out: " + result };
		}

		//Terminating a system - exclusive message without sending an 
		//EOX(end - of - exclusive) byte might cause problems for the receiving device.
		//The midiOutReset function does not send an EOX byte when it terminates 
		//a system - exclusive message — applications are responsible for doing this.
		outputShortMsg(systemExclusiveEnd);
	}
}