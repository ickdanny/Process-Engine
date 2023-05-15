#pragma once

#include <cstdint>
#include <stdexcept>

namespace wasp::sound::midi {
	//all midi files should begin with "MThd"
	constexpr uint32_t requiredHeaderID{ 0x4d546864 };
	//currently all midi files have a header size of 6
	constexpr uint32_t minimumHeaderSize{ 0x00000006 };

	enum fileFormat : std::uint16_t {
		formatSingleTrack =		0x0000,
		formatMultiTrackSync =	0x0001,
		formatMultiTrackAsync = 0x0002
	};

	//all midi tracks should begin with "MTrk"
	constexpr uint32_t requiredTrackHeaderID{ 0x4d54726b };

	//status codes
	constexpr uint8_t statusMask{ 0b1111'0000 };
	//midi event last 4 bits are channel #
	enum midiEvents : uint8_t {
		noteOff =						0b1000'0000,
		noteOn =						0b1001'0000,
		polyphonicKeyPressure =			0b1010'0000,
		controlChange =					0b1011'0000,
		programChange =					0b1100'0000,
		channelPressure =				0b1101'0000,
		pitchBendChange =				0b1110'0000,
		metaEventOrSystemExclusive =	0b1111'0000
	};
	//meta events should not be sent to the synth
	constexpr uint8_t metaEvent{ 0b1111'1111 };
	//system exclusive events should be sent to the synth including the byte F0
	constexpr uint8_t systemExclusiveStart{ 0b1111'0000 };
	//used to end sysex
	//also starts both continuation events as well as escape sequences
	constexpr uint8_t systemExclusiveEnd{ 0b1111'0111 };

	//meta events have a secondary status code
	enum metaEvents : uint8_t {
		sequenceNumber =		0b0000'0000,
		text =					0b0000'0001,
		copyright =				0b0000'0010,
		sequenceOrTrackName =	0b0000'0011,
		instrumentName =		0b0000'0100,
		lyric =					0b0000'0101,
		marker =				0b0000'0110,
		cuePoint =				0b0000'0111,
		programName =			0b0000'1000,
		deviceName =			0b0000'1001,
		midiChannelPrefix =		0b0010'0000,
		midiPort =				0b0010'0001,
		endOfTrack =			0b0010'1111,
		tempo =					0b0101'0001,
		smpteOffset =			0b0101'0100,
		timeSignature =			0b0101'1000,
		keySignature =			0b0101'1001,
		sequencerSpecific =		0b0101'1001
	};

	enum controllerCodes : uint8_t {
		bankSelectMSB =						0,
		modulationWheelMSB =				1,
		breathControllerMSB =				2,
		//undefined							//undefined	3
		footPedalMSB =						4,
		portamentoTimeMSB =					5,
		dataEntryMSB =						6,
		channelVolumeMSB =					7,  mainVolumeMSB =		7,
		balanceMSB =						8,
		//undefined							//undefined 9
		panMSB =							10,
		expressionMSB =						11,
		effectControl1MSB =					12,
		effectControl2MSB =					13,
		//undefined							//undefined 14
		//undefined							//undefined 15
		generalPurposeController1MSB =		16,
		generalPurposeController2MSB =		17,
		generalPurposeController3MSB =		18,
		generalPurposeController4MSB =		19,
		//undefined							//undefined 20
		//.									//.
		//.									//.
		//.									//.
		//undefined							//undefined 31
		bankSelectLSB =						32,
		modulationWheelLSB =				33,
		breathControllerLSB =				34,
		//undefined							//undefined 35
		footPedalLSB =						36,
		portamentoTimeLSB =					37,
		dataEntryLSB =						38,
		channelVolumeLSB =					39,  mainVolumeLSB =		39,
		balanceLSB =						40,
		//undefined							//undefined 41
		panLSB =							42,
		expressionLSB =						43,
		effectControl1LSB =					44,
		effectControl2LSB =					45,
		//undefined							//undefined 46,
		//undefined							//undefined 47,
		generalPurposeController1LSB =		48,
		generalPurposeController2LSB =		49,
		generalPurposeController3LSB =		50,
		generalPurposeController4LSB =		51,
		//undefined							//undefined 52,
		//.									//.
		//.									//.
		//.									//.
		//undefined							//undefined 63,
		sustainSwitch =						64,
		portamentoSwitch =					65,
		sostenutoSwitch =					66,
		softPedalSwitch =					67,
		legatoSwitch =						68,
		hold2Switch =						69,
		soundController1 =					70,  soundVariation =		70,
		soundController2 =					71,  soundTimbre =			71,
		soundController3 =					72,  releaseTime =			72,
		soundController4 =					73,  attackTime =			73,
		soundController5 =					74,	 brightness =			74,
		soundController6 =					75,  decayTime =			75,
		soundController7 =					76,  vibratoRate =			76,
		soundController8 =					77,  vibratoDepth =			77,
		soundController9 =					78,  vibratoDelay =			78,
		soundController0 =					79, 
		generalPurposeController5 =			80,
		generalPurposeController6 =			81,
		generalPurposeController7 =			82,
		generalPurposeController8 =			83,
		portamentoControl =					84,
		//undefined							//undefined 85
		//undefined							//undefined 86
		//undefined							//undefined 87
		highResolutionVelocityPrefix =		88,
		//undefined							//undefined 89
		//undefined							//undefined 90
		effects1Depth =						91,  reverb =				91,
		effects2Depth =						92,  tremelo =				92,
		effects3Depth =						93,  chorus =				93,
		effects4Depth =						94,  detune = 94, delay =	94,
		effects5Depth =						95,  phaser =				95,
		dataIncrement =						96,
		dataDecrement =						97,
		nonRegisteredParameterNumberLSB =	98,
		nonRegisteredParameterNumberMSB =	99,
		registeredParameterNumberLSB =		100,
		registeredParameterNumberMSB =		101
	};
		//undefined							//undefined 102,
		//.									//.
		//.									//.
		//.									//.
		//undefined							//undefined 119,
	enum channelModeMessages : uint8_t {
		allSoundOff =						120,
		resetAllControllers =				121,
		localControl =						122,
		allNotesOff =						123,
		omniModeOff =						124,
		omniModeOn =						125,
		monoModeOn =						126,
		polyModeOn =						127
	};

	//midi defaults to 120 BPM
	constexpr uint32_t defaultMicrosecondsPerBeat{ 500'000 };

	constexpr uint8_t smpteFpsEncode(uint32_t fps) {
		switch (fps) {
			case 24:
				return 0xE8;
			case 25:
				return 0xE7;
			case 29:
				return 0xE3;
			case 30:
				return 0xE2;
			default:
				throw std::runtime_error{ "Error invalid SMPTE fps: " + fps };
		}
	}

	constexpr uint32_t smpteFpsDecode(uint8_t code) {
		switch (code) {
			case 0xE8:
				return 24;
			case 0xE7:
				return 25;
			case 0xE3:
				return 29;
			case 0xE2:
				return 30;
			default:
				throw std::runtime_error{ "Error invalid SMPTE code: " + code };
		}
	}
}