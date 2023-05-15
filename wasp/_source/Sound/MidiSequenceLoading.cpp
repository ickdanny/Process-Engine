#include "Sound/MidiSequenceLoading.h"

#include <fstream>

#include "Utility\ByteUtil.h"
#include "Math\MathUtil.h"
#include "Sound\MidiConstants.h"

namespace wasp::sound::midi {

	constexpr int trackSizeByteMultiplier = 4;

	using wasp::utility::byteSwap16;
	using wasp::utility::byteSwap32;
	using wasp::math::ceilingIntegerDivide;

	using EventUnit = MidiSequence::EventUnit;
	using EventUnitTrack = MidiSequence::EventUnitTrack;

	#pragma pack(push, 1)
	struct MidiFileHeader {
		uint32_t id{};		// identifier "MThd"
		uint32_t size{};	// always 6 in big-endian format
		uint16_t format{};	// big-endian format
		uint16_t tracks{};	// number of tracks, big-endian
		uint16_t ticks{};	// number of ticks per quarter note, big-endian
	};
	#pragma pack(pop)

	#pragma pack(push, 1)
	struct MidiTrackHeader {
		uint32_t id{};		// identifier "MTrk"
		uint32_t length{};	// track length, big-endian
	};
	#pragma pack(pop)

	MidiFileHeader readFileHeader(std::istream& inStream) {
		MidiFileHeader header{};
		//read in file header
		inStream.read(
			reinterpret_cast<char*>(&header),
			sizeof(MidiFileHeader)
		);

		//swap file header endianness
		header.id = byteSwap32(header.id);
		header.size = byteSwap32(header.size);
		header.format = byteSwap16(header.format);
		header.tracks = byteSwap16(header.tracks);
		header.ticks = byteSwap16(header.ticks);

		//check file header validity
		if (header.id != requiredHeaderID) {
			throw new std::runtime_error{ "Error MIDI file invalid header identifier" };
		}
		if (header.size < minimumHeaderSize) {
			throw new std::runtime_error{ "Error MIDI file header too small" };
		}

		//handle the case when the file header is longer than expected
		if (header.size > minimumHeaderSize) {
			inStream.ignore(header.size - minimumHeaderSize);
		}

		return header;
	}

	MidiTrackHeader readTrackHeader(std::istream& inStream) {
		MidiTrackHeader header{};
		//read in header
		inStream.read(
			reinterpret_cast<char*>(&header),
			sizeof(MidiTrackHeader)
		);

		//swap track header endianness
		header.id = byteSwap32(header.id);
		header.length = byteSwap32(header.length);

		//check track header validity
		if (header.id != requiredTrackHeaderID) {
			throw new std::runtime_error{ "Error MIDI file invalid track identifier" };
		}

		return header;
	}

	uint32_t readVariableLength(std::istream& inStream) {
		uint32_t toRet{};
		uint8_t byte{};

		//read variable length loop
		do {
			inStream.read(reinterpret_cast<char*>(&byte), sizeof(byte));
			toRet = (toRet << 7) + (byte & 0b0111'1111);
		} while (byte & 0b1000'0000);

		//DON'T SWAP ENDIANNESS

		return toRet;
	}

	size_t calculateInitialTrackLength(uint32_t trackHeaderLength) {
		return (trackHeaderLength * trackSizeByteMultiplier) / sizeof(EventUnit);
	}

	//encoded as byteLength / indexLength
	EventUnit encodeLength(uint32_t byteLength) {
		EventUnit eventUnit{};
		eventUnit.deltaTime = byteLength;
		uint32_t indexLength{ ceilingIntegerDivide(
			byteLength,
			static_cast<uint32_t>(sizeof(EventUnit))
		) };
		eventUnit.event = indexLength;
		return eventUnit;
	}

	void throwIfInvalidLoopPoints(int64_t loopStart, int64_t loopEnd) {
		if (loopStart < 0 || (loopEnd != -1 && loopEnd <= loopStart)) {
			throw std::runtime_error{ "Error invalid loop points given" };
		}
	}

	uint8_t readByteFromIStream(std::istream& inStream) {
		return static_cast<uint8_t>(inStream.get());
	}

	//helper functions
	void loadMidiEvent(
		std::istream& inStream, 
		EventUnitTrack& track,
		uint32_t& index,
		uint8_t& lastStatus,
		uint8_t status,
		uint8_t maskedStatus
	) {
		//read in first byte
		uint8_t temp{ readByteFromIStream(inStream) };
		track[index].event = static_cast<uint32_t>(status) |
			(static_cast<uint32_t>(temp) << 8);

		//read second byte if has one
		if (maskedStatus != programChange && maskedStatus != channelPressure) {
			temp = inStream.get();
			track[index].event |= (static_cast<uint32_t>(temp) << 16);
		}

		//DON'T SWAP ENDIANNESS

		lastStatus = status;
		//advance index by 1
		++index;
	}

	void loadMetaEvent(
		std::istream& inStream,
		EventUnitTrack& track,
		uint32_t& index,
		uint8_t& lastStatus,
		bool& encounteredEndOfTrack,
		uint8_t status
	) {
		//read in the meta event
		uint8_t metaEventStatus{ readByteFromIStream(inStream) };

		//read in length 
		uint32_t length = readVariableLength(inStream);

		//do not insert end of track events into our translated track
		if (metaEventStatus == endOfTrack) {
			//remove the delta time we set at the start
			track[index] = {};
			encounteredEndOfTrack = true;
			//don't advance index
		}

		//insert everything else
		else {
			//first block = deltaTime / 00 - 00 - event - FF
			track[index++].event = (metaEventStatus << 8) | status;

			//second block = length / index length
			track[index] = encodeLength(length);
			uint32_t indexLength{ track[index++].event };

			if (length > 0) {
				//read binary data into our translated track
				inStream.read(
					reinterpret_cast<char*>(&track[index]),
					length
				);
				//advance index by necessary amount
				index += indexLength;
			}
		}
		lastStatus = 0;
	}

	void loadSystemExclusiveEvent(
		std::istream& inStream, 
		EventUnitTrack& track,
		uint32_t& index,
		uint8_t& lastStatus,
		uint8_t status,
		bool insertSystemExclusiveStart
	) {
		//read in length
		uint32_t length = readVariableLength(inStream);

		//first block = deltaTime / 00 - 00 - 00 - F0
		track[index++].event = status;

		//second block = length / index length
		if (insertSystemExclusiveStart) {
			//add 1 to length for our inserted F0 byte if length > 0
			track[index] = encodeLength(length > 0 ? length + 1 : 0);
		}
		else {
			track[index] = encodeLength(length);
		}
		uint32_t indexLength{ track[index++].event };

		if (length > 0) {
			if (insertSystemExclusiveStart) {
				//insert F0 at beginning of data dump
				track[index].deltaTime = systemExclusiveStart;
				//read binary data into our translated track
				inStream.read(
					reinterpret_cast<char*>(&track[index]) + 1,
					length
				);
			}
			else {
				inStream.read(
					reinterpret_cast<char*>(&track[index]),
					length
				);
			}
			//advance index by necessary amount
			index += indexLength;
		}
		lastStatus = 0;
	}

	void insertLoopEvent(
		EventUnitTrack& track,
		uint32_t& index,
		uint32_t deltaTime
	) {
		track[index++] = { deltaTime, (static_cast<uint32_t>(tempo) << 8) | metaEvent };
		//we encode the loop points as tempo events with length block 0
		track[index++] = MidiSequence::loopEncoding;
	}

	bool insertLoopEventIfNecessary(
		EventUnitTrack& track,
		uint32_t& index,
		int64_t loopTime,
		int64_t& realTime,
		uint32_t& deltaTime
	) {
		//check if this event we've read occurs after loop start
		if ((realTime + deltaTime) >= loopTime) {
			//calculate our loop deltaTime and our new event deltaTime
			uint32_t loopPointDeltaTime{
				static_cast<uint32_t>(loopTime - realTime)
			};
			deltaTime -= loopPointDeltaTime;
			insertLoopEvent(track, index, loopPointDeltaTime);
			realTime = loopTime;
			//realTime is pointing at our loop point
			return true;
		}
		return false;
	}

	void doFinalLoopCheck(
		EventUnitTrack& track,
		uint32_t& index,
		bool placedLoopStart,
		bool placedLoopEnd,
		int64_t loopEnd,
		int64_t realTime
	) {
		//handle bizarre loop cases
		//(don't worry about setting index in this function)
		if (!placedLoopStart) {
			throw std::runtime_error{ "Error loop start too high " };
		}
		if (loopEnd == -1) {
			insertLoopEvent(track, index, 0);
		}
		else if (!placedLoopEnd) {
			uint32_t loopEndDeltaTime{ static_cast<uint32_t>(loopEnd - realTime) };
			insertLoopEvent(track, index, loopEndDeltaTime);
		}
	}

	EventUnitTrack loadTrack(std::istream& inStream) {
		//read in track header
		MidiTrackHeader trackHeader{ readTrackHeader(inStream) };

		//prepare track
		EventUnitTrack track(
			calculateInitialTrackLength(trackHeader.length)
		);

		uint8_t lastStatus{ 0 };
		uint32_t index{ 0 };
		bool encounteredEndOfTrack{ false };

		//begin loading track
		while (!encounteredEndOfTrack) {
			//read in delta time
			track[index].deltaTime = readVariableLength(inStream);

			//grab command byte
			uint8_t status{ readByteFromIStream(inStream) };

			//handle running status
			if (status < 0b1000'0000) {
				status = lastStatus;
				inStream.seekg(-1, std::ios_base::cur);
			}

			//handle midi events
			uint8_t maskedStatus{ static_cast<uint8_t>(status & statusMask) };	//cast??
			if (maskedStatus != metaEventOrSystemExclusive) {
				loadMidiEvent(
					inStream, 
					track, 
					index,
					lastStatus, 
					status, 
					maskedStatus
				);
			}
			//handle meta events
			else if (status == metaEvent) {
				loadMetaEvent(
					inStream,
					track,
					index,
					lastStatus,
					encounteredEndOfTrack,
					status
				);
			}
			//handle system exclusive events
			else if (status == systemExclusiveStart) {
				loadSystemExclusiveEvent(
					inStream,
					track,
					index,
					lastStatus,
					status,
					true			//insert F0 at start
				);
			}
			//irrelevant whether continuation packet or escape sequence
			else if (status == systemExclusiveEnd) {
				loadSystemExclusiveEvent(
					inStream,
					track,
					index,
					lastStatus,
					status,
					false			//do not insert anything at start
				);
			}
		}

		//index points to the first empty space
		track.erase(track.begin() += index, track.end());
		track.shrink_to_fit();

		return track;
	}

	EventUnitTrack loadTrackLooped(
		std::istream& inStream,
		int64_t loopStart,
		int64_t loopEnd
	) {

		throwIfInvalidLoopPoints(loopStart, loopEnd);

		//read in track header
		MidiTrackHeader trackHeader{ readTrackHeader(inStream) };

		//prepare track
		EventUnitTrack track(
			calculateInitialTrackLength(trackHeader.length)
		);

		bool placedLoopStart{ false };
		bool placedLoopEnd{ false };
		int64_t realTime{ 0 };

		uint8_t lastStatus{ 0 };
		uint32_t index{ 0 };
		bool encounteredEndOfTrack{ false };

		if (loopStart == 0) {
			insertLoopEvent(track, index, 0);
			placedLoopStart = true;
		}

		//begin loading track
		while (!encounteredEndOfTrack) {
			
			//read in delta time and handle loop events
			uint32_t deltaTime{ readVariableLength(inStream) };

			if (!placedLoopStart) {
				placedLoopStart = insertLoopEventIfNecessary(
					track,
					index,
					loopStart,
					realTime,
					deltaTime
				);
			}
			if (loopEnd != -1 && !placedLoopEnd) {
				//case where loop end is directly after loop start is handled
				placedLoopEnd = insertLoopEventIfNecessary(
					track,
					index,
					loopEnd,
					realTime,
					deltaTime
				);
			}

			track[index].deltaTime = deltaTime;
			realTime += deltaTime;

			//grab command byte
			uint8_t status{ readByteFromIStream(inStream) };

			//handle running status
			if (status < 0b1000'0000) {
				status = lastStatus;
				inStream.seekg(-1, std::ios_base::cur);
			}

			//handle midi events
			uint8_t maskedStatus{ static_cast<uint8_t>(status & statusMask) }; //cast??
			if (maskedStatus != metaEventOrSystemExclusive) {
				loadMidiEvent(
					inStream,
					track,
					index,
					lastStatus,
					status,
					maskedStatus
				);
			}
			//handle meta events
			else if (status == metaEvent) {
				loadMetaEvent(
					inStream,
					track,
					index,
					lastStatus,
					encounteredEndOfTrack,
					status
				);
			}
			//handle system exclusive events
			else if (status == systemExclusiveStart) {
				loadSystemExclusiveEvent(
					inStream,
					track,
					index,
					lastStatus,
					status,
					true	//insert F0 at start
				);
			}
			//irrelevant whether continuation packet or escape sequence
			else if (status == systemExclusiveEnd) {
				loadSystemExclusiveEvent(
					inStream,
					track,
					index,
					lastStatus,
					status,
					false	//do not insert anything at start
				);
			}
		}

		doFinalLoopCheck(track, index, placedLoopStart, placedLoopEnd, loopEnd, realTime);

		//index points to the first empty space
		track.erase(track.begin() += index, track.end());
		track.shrink_to_fit();

		return track;
	}

	static EventUnitTrack compileTracks(
		std::vector<EventUnitTrack>& individualTracks
	) {
		//case where we only have 1 track
		if (individualTracks.size() == 1) {
			return std::move(individualTracks[0]);
		}

		//create track to hold all elements in individualTracks
		size_t totalSize{ 0 };
		for (const auto& track : individualTracks) {
			totalSize += track.size();
		}
		EventUnitTrack compiledTrack(totalSize);

		//delta time corresponding to each track
		std::vector<uint32_t> deltaTimes(individualTracks.size());
		for (size_t index{ 0 }; index < individualTracks.size(); ++index) {
			deltaTimes[index] = individualTracks[index][0].deltaTime;
		}
		//where we are along each individual track
		std::vector<size_t> indices(individualTracks.size());
		//where we are on our compiled track
		size_t compiledIndex{ 0 };

		//find and insert events by chronological order
		while (individualTracks.size() > 1) {
			//find next event to insert
			uint32_t lowestDeltaTime{ std::numeric_limits<uint32_t>::max() };
			size_t lowestDeltaTimeIndex{ 0 };

			for (size_t index{ 0 }; index < individualTracks.size(); ++index) {
				if (deltaTimes[index] < lowestDeltaTime) {
					lowestDeltaTime = deltaTimes[index];
					lowestDeltaTimeIndex = index;
				}
			}

			//handle deltaTimes and realTime
			for (auto& deltaTime : deltaTimes) {
				deltaTime -= lowestDeltaTime;
			}

			auto& trackIndex{ indices[lowestDeltaTimeIndex] };
			auto& individualTrack{ individualTracks[lowestDeltaTimeIndex] };

			//truncate to just the status byte
			uint8_t status{ static_cast<uint8_t>(individualTrack[trackIndex].event) };

			//handle normal midi message case
			if ((status & statusMask) != metaEventOrSystemExclusive) {
				//copy into our compiledTrack and advance indices
				compiledTrack[compiledIndex].deltaTime = lowestDeltaTime;
				compiledTrack[compiledIndex++].event
					= individualTrack[trackIndex++].event;
			}
			//handle meta and sysex i.e. message with length
			else {
				//copy status block into our compiledTrack and advance indices
				compiledTrack[compiledIndex].deltaTime = lowestDeltaTime;
				compiledTrack[compiledIndex++].event
					= individualTrack[trackIndex++].event;
				//trackIndex is now pointing to the length block
				//copy the length block, store index length, and advance indices
				uint32_t indexLength{
					(compiledTrack[compiledIndex++] = individualTrack[trackIndex++])
					.event
				};
				//trackIndex is now pointing to the first block after the length
				//copy as many blocks as required by indexLength
				for (uint32_t i{ 0 }; i < indexLength; ++i) {
					compiledTrack[compiledIndex++] = individualTrack[trackIndex++];
				}
			}

			//either track is over, in which case remove
			if (trackIndex >= individualTrack.size()) {
				deltaTimes.erase(deltaTimes.begin() + lowestDeltaTimeIndex);
				indices.erase(indices.begin() + lowestDeltaTimeIndex);
				individualTracks.erase(
					individualTracks.begin() + lowestDeltaTimeIndex
				);
			}
			//or update new delta time for next event on the track
			else {
				deltaTimes[lowestDeltaTimeIndex]
					= individualTrack[trackIndex].deltaTime;
			}
		}
		//1 track left, append all into our compiled track
		auto& individualTrack{ individualTracks[0] };
		while (indices[0] < individualTrack.size()) {
			compiledTrack[compiledIndex++] = individualTrack[indices[0]++];
		}

		return compiledTrack;
	}

	EventUnitTrack compileTracksLooped(
		std::vector<EventUnitTrack>& individualTracks,
		int64_t loopStart,
		int64_t loopEnd
	) {
		throwIfInvalidLoopPoints(loopStart, loopEnd);

		//create track to hold all elements in individualTracks
		//add 4 slots to buffer for inserted loop events
		size_t totalSize{ 4 };
		for (const auto& track : individualTracks) {
			totalSize += track.size();
		}
		EventUnitTrack compiledTrack(totalSize);

		//delta time corresponding to each track
		std::vector<uint32_t> deltaTimes(individualTracks.size());
		for (size_t index{ 0 }; index < individualTracks.size(); ++index) {
			deltaTimes[index] = individualTracks[index][0].deltaTime;
		}
		//where we are along each individual track
		std::vector<size_t> indices(individualTracks.size());
		//where we are on our compiled track
		size_t compiledIndex{ 0 };

		//find and insert events by chronological order
		bool placedLoopStart{ false };
		bool placedLoopEnd{ false };
		int64_t realTime{ 0 };

		while (individualTracks.size() > 0) {
			//find next event to insert
			uint32_t lowestDeltaTime{ std::numeric_limits<uint32_t>::max() };
			size_t lowestDeltaTimeIndex{ 0 };

			for (size_t index{ 0 }; index < individualTracks.size(); ++index) {
				if (deltaTimes[index] < lowestDeltaTime) {
					lowestDeltaTime = deltaTimes[index];
					lowestDeltaTimeIndex = index;
				}
			}

			//handle deltaTimes and realTime
			for (auto& deltaTime : deltaTimes) {
				deltaTime -= lowestDeltaTime;
			}

			if (!placedLoopStart) {
				placedLoopStart = insertLoopEventIfNecessary(
					compiledTrack,
					compiledIndex,
					loopStart,
					realTime,
					lowestDeltaTime
				);
			}
			if (loopEnd != -1 && !placedLoopEnd) {
				//case where loop end is directly after loop start is handled
				placedLoopStart = insertLoopEventIfNecessary(
					compiledTrack,
					compiledIndex,
					loopEnd,
					realTime,
					lowestDeltaTime
				);
			}

			realTime += lowestDeltaTime;

			auto& trackIndex{ indices[lowestDeltaTimeIndex] };
			auto& individualTrack{ individualTracks[lowestDeltaTimeIndex] };

			//truncate to just the status byte
			uint8_t status{ static_cast<uint8_t>(individualTrack[trackIndex].event) };

			//handle normal midi message case
			if ((status & statusMask) != metaEventOrSystemExclusive) {
				//copy into our compiledTrack and advance indices
				compiledTrack[compiledIndex].deltaTime = lowestDeltaTime;
				compiledTrack[compiledIndex++].event
					= individualTrack[trackIndex++].event;
			}
			//handle meta and sysex i.e. message with length
			else {
				//copy status block into our compiledTrack and advance indices
				compiledTrack[compiledIndex].deltaTime = lowestDeltaTime;
				compiledTrack[compiledIndex++].event
					= individualTrack[trackIndex++].event;
				//trackIndex is now pointing to the length block
				//copy the length block, store index length, and advance indices
				uint32_t indexLength{
					(
						compiledTrack[compiledIndex++] = individualTrack[trackIndex++]
					).event
				};
				//trackIndex is now pointing to the first block after the length
				//copy as many blocks as required by indexLength
				for (uint32_t i{ 0 }; i < indexLength; ++i) {
					compiledTrack[compiledIndex++] = individualTrack[trackIndex++];
				}
			}

			//either track is over, in which case remove
			//or update new delta time for next event on the track
			if (trackIndex >= individualTrack.size()) {
				deltaTimes.erase(deltaTimes.begin() + lowestDeltaTimeIndex);
				indices.erase(indices.begin() + lowestDeltaTimeIndex);
				individualTracks.erase(
					individualTracks.begin() + lowestDeltaTimeIndex
				);
			}
			else {
				deltaTimes[lowestDeltaTimeIndex]
					= individualTrack[trackIndex].deltaTime;
			}
		}

		doFinalLoopCheck(
			compiledTrack, 
			compiledIndex, 
			placedLoopStart, 
			placedLoopEnd, 
			loopEnd, 
			realTime
		);

		compiledTrack.erase(compiledTrack.begin() += compiledIndex, compiledTrack.end());
		compiledTrack.shrink_to_fit();

		return compiledTrack;
	}

	MidiSequence parseMidiFile(const std::wstring& fileName) {
		std::ifstream inStream{ fileName, std::ios::binary };
		MidiSequence midiSequence{};

		//read in header file
		MidiFileHeader header{ readFileHeader(inStream) };
		midiSequence.ticks = header.ticks;

		if (header.format == formatSingleTrack) {
			midiSequence.compiledTrack = loadTrack(inStream);
		}
		else if (header.format == formatMultiTrackSync) {
			std::vector<MidiSequence::EventUnitTrack> individualTracks(header.tracks);

			for (uint16_t i{ 0 }; i < header.tracks; ++i) {
				individualTracks[i] = loadTrack(inStream);
			}

			midiSequence.compiledTrack = compileTracks(individualTracks);
		}
		else {
			throw std::runtime_error{ "Error unsupported MIDI format" };
		}

		return midiSequence;
	}

	MidiSequence parseLoopedMidiFile(
		const std::wstring& fileName,
		int64_t loopStart,
		int64_t loopEnd
	) {
		throwIfInvalidLoopPoints(loopStart, loopEnd);

		std::ifstream inStream{ fileName, std::ios::binary };
		MidiSequence midiSequence{};

		//read in header file
		MidiFileHeader header{ readFileHeader(inStream) };
		midiSequence.ticks = header.ticks;

		if (header.format == formatSingleTrack) {
			midiSequence.compiledTrack = loadTrackLooped(inStream, loopStart, loopEnd);
		}
		else if (header.format == formatMultiTrackSync) {
			std::vector<MidiSequence::EventUnitTrack> individualTracks(header.tracks);

			for (uint16_t i{ 0 }; i < header.tracks; ++i) {
				individualTracks[i] = loadTrack(inStream);
			}

			midiSequence.compiledTrack = compileTracksLooped(
				individualTracks,
				loopStart,
				loopEnd
			);
		}
		else {
			throw std::runtime_error{ "Error unsupported MIDI format" };
		}

		return midiSequence;
	}
}