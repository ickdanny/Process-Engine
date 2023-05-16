#include "Sound\MidiSequencer.h"

#include <stdexcept>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

#include "Utility\ByteUtil.h"
#include "Utility\Scheduling.h"

#include "Logging.h"

namespace wasp::sound::midi {
	
	constexpr uint64_t ratio100nsToSeconds { 10'000'000ull };
	
	using wasp::utility::byteSwap32;
	using wasp::utility::getByte;
	using wasp::utility::sleep100ns;
	
	//helper functions
	namespace {
		uint32_t convertMicrosecondsPerBeatToTimePerTick100ns(
			uint32_t microsecondsPerBeat,
			uint16_t ticks
		) {
			return (10 * microsecondsPerBeat) / ticks;
		}
		
		uint32_t calculateInitialTimePerTick100ns(uint16_t ticks) {
			//a leading 1 means SMPTE FPS time
			if( ticks & 0b1000'0000'0000'0000 ) {
				uint32_t fps { smpteFpsDecode(getByte(ticks, 2)) };
				uint8_t subframeResolution { static_cast<uint8_t>(ticks) };
				auto subframesPerSecond { fps * subframeResolution };
				return ratio100nsToSeconds / subframesPerSecond;
			}
				//a leading 0 means ticks per beat
			else {
				return convertMicrosecondsPerBeatToTimePerTick100ns(
					defaultMicrosecondsPerBeat,
					ticks
				);
			}
		}
	}
	
	MidiSequencer::~MidiSequencer() {
		//try to stop and turn off all notes
		try {
			stop();
		}
		catch( const std::exception& error ) {
			//swallow error
			debug::log(error.what());
		}
		catch( ... ) {
			//swallow error
			debug::log("Exception caught of unknown type in MidiSequencer destructor");
		}
	}
	
	void MidiSequencer::start(std::shared_ptr<MidiSequence> midiSequencePointer) {
		stopPlaybackThread();
		this->midiSequencePointer = midiSequencePointer;
		running.store(true);
		playbackThread = std::thread {
			[&] {
				try {
					this->playback();
				}
				catch( const std::exception& exception ) {
					debug::log(exception.what());
					std::exit(1);
				}
				catch( ... ) {
					debug::log("Exception of unknown type caught on midi playback thread");
					std::exit(1);
				}
			}
		};
	}
	
	void MidiSequencer::playback() {
		using ratioTimePointTo100ns = std::ratio<
			clockType::period::num * ratio100nsToSeconds,
			clockType::period::den
		>;
		
		midiOutPointer->outputReset();
		
		//timing variables
		microsecondsPerBeat = defaultMicrosecondsPerBeat;
		timePerTick100ns = calculateInitialTimePerTick100ns(
			midiSequencePointer->ticks
		);
		
		timePointType prevTimeStamp { getCurrentTime() };
		timePointType currentTimeStamp {};
		
		long long previousSleepDuration100ns { 0 };
		
		//loop variables
		iter = midiSequencePointer->compiledTrack.begin();
		auto endIter { midiSequencePointer->compiledTrack.end() };
		loopPointIter = endIter;
		
		//helper function for calculating sleep duration
		auto calculateSleepDuration100ns {
			[&]() {
				//calculate how long we should wait
				long long sleepDuration100ns {
					static_cast<long long>(iter->deltaTime) * timePerTick100ns
				};
				
				//account for the time we spent already
				currentTimeStamp = getCurrentTime();
				long long timeElapsed {
					((currentTimeStamp - prevTimeStamp).count()
						* ratioTimePointTo100ns::num)
						/ ratioTimePointTo100ns::den
				};
				long long timeLost { timeElapsed - previousSleepDuration100ns };
				sleepDuration100ns -= timeLost;
				
				prevTimeStamp = currentTimeStamp;
				return sleepDuration100ns;
			}
		};
		
		//begin playback
		while( iter != endIter ) {
			//sleep for delta time
			if( iter->deltaTime != 0 ) {
				long long sleepDuration100ns {
					calculateSleepDuration100ns()
				};
				previousSleepDuration100ns = sleepDuration100ns;
				
				//check to see if need to exit before sleep
				if( !running.load() ) {
					//do not output reset in this case
					resetPlaybackFields();
					return;
				}
				
				sleep100ns(sleepDuration100ns);
				
				//also check after sleep
				if( !running.load() ) {
					//do not output reset in this case
					resetPlaybackFields();
					return;
				}
			}
			
			//handle event
			uint8_t status { getByte(iter->event, 1) };
			//midi event case
			if( (status & statusMask) != metaEventOrSystemExclusive ) {
				outputMidiEvent();
			}
				//meta event or system exclusive cases
			else {
				switch( status ) {
					case metaEvent:
						//may change tempo or loop back
						handleMetaEvent();
						break;
						//continuation events and escape sequences start with sysEx end
						//the data is encoded the same way
					case systemExclusiveStart:
					case systemExclusiveEnd: outputSystemExclusiveEvent();
						break;
					default: throw std::runtime_error { "Error unrecognized MIDI status" };
				}
			}
		}
		
		//output reset if we finish naturally
		midiOutPointer->outputReset();
		resetPlaybackFields();
		running.store(false);
	}
	
	void MidiSequencer::stop() {
		stopPlaybackThread();
		midiOutPointer->outputReset();
	}
	
	void MidiSequencer::outputMidiEvent() {
		midiOutPointer->outputShortMsg((iter++)->event);
	}
	
	void MidiSequencer::outputSystemExclusiveEvent() {
		++iter;
		//index now points to the length block
		
		uint32_t byteLength { iter->deltaTime };
		uint32_t indexLength { iter->event };
		++iter;
		//index now points to the first data entry
		
		//prepare data to be output from MIDIHDR
		MIDIHDR midiHDR {};
		midiHDR.lpData = reinterpret_cast<char*>(&(*iter)); //probably UB
		midiHDR.dwBufferLength = byteLength;
		midiHDR.dwBytesRecorded = byteLength;
		
		iter += indexLength;
		
		midiOutPointer->outputSystemExclusive(&midiHDR);
		
		//index now points to 1 past the last data entry
	}
	
	void MidiSequencer::handleMetaEvent() {
		//the status is the second byte (following 0xFF)
		uint8_t metaEventStatus { getByte(iter->event, 2) };
		++iter;
		//index now points to the length block
		
		uint32_t byteLength { iter->deltaTime };
		uint32_t indexLength { iter->event };
		++iter;
		//index now points to the first data entry
		
		if( metaEventStatus == tempo ) {
			//test if this is actually a loop event
			if(
				(byteLength == MidiSequence::loopEncoding.deltaTime)
					&& (indexLength == MidiSequence::loopEncoding.event)
				) {
				//if this is the loop start, set it
				if( loopPointIter == midiSequencePointer->compiledTrack.end() ) {
					loopPointIter = iter;
				}
					//if this is the loop end, bring us back to the loop start
				else {
					iter = loopPointIter;
				}
			}
				//otherwise this is a real tempo event
			else {
				microsecondsPerBeat = byteSwap32(iter->deltaTime) >> 8;
				timePerTick100ns = convertMicrosecondsPerBeatToTimePerTick100ns(
					microsecondsPerBeat,
					midiSequencePointer->ticks
				);
			}
		}
		
		iter += indexLength;
		//index now points to 1 past the last data entry
	}
	
	void MidiSequencer::stopPlaybackThread() {
		running.store(false);
		if( playbackThread.joinable() ) {
			wakeupSwitch.signal();
			playbackThread.join();
			wakeupSwitch.unsignal();
		}
	}
	
	void MidiSequencer::resetPlaybackFields() {
		midiSequencePointer = {};
		iter = {};
		loopPointIter = {};
		microsecondsPerBeat = defaultMicrosecondsPerBeat;
		timePerTick100ns = 0;
	}
	
	//static
	MidiSequencer::timePointType MidiSequencer::getCurrentTime() {
		return clockType::now();
	}
}