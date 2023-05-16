#pragma once

#include <iostream>

namespace wasp::game {
	
	#pragma pack(push, 1)
	struct Settings {
		bool muted {};            //true = sound off, false = sound on
		bool fullscreen {};        //true = fullscreen, false = windowed
	};
	#pragma pack(pop)
	
	template <typename CharT, typename Traits = std::char_traits<CharT>>
	std::basic_ostream<CharT, Traits>& operator<<(
		std::basic_ostream<CharT, Traits>& outStream, const Settings& settings
	) {
		outStream.write(
			reinterpret_cast<const char*>(&settings),
			sizeof(Settings)
		);
		return outStream;
	}
	
	template <typename CharT, typename Traits = std::char_traits<CharT>>
	std::basic_istream<CharT, Traits>& operator>>(
		std::basic_istream<CharT, Traits>& inStream, Settings& settings
	) {
		inStream.read(
			reinterpret_cast<char*>(&settings),
			sizeof(Settings)
		);
		return inStream;
	}
	
	namespace settings {
		//utility functions
		Settings readOrCreateSettingsFromFile(const std::string& fileName);
		
		void writeSettingsToFile(
			const Settings& settings,
			const std::string& fileName
		);
	}
}