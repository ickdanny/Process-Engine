#include "Settings.h"

#include <fstream>

namespace wasp::game::settings {
	
	Settings readOrCreateSettingsFromFile(const std::string& fileName) {
		Settings settings {};
		std::ifstream inStream { fileName, std::ios::binary };
		inStream >> settings;
		if( !inStream ) {     //this checks bad and fail but not eof
			inStream.close();
			settings = {};
			writeSettingsToFile(settings, fileName);
		}
		return settings;
	}
	
	void writeSettingsToFile(
		const Settings& settings,
		const std::string& fileName
	) {
		std::ofstream outStream { fileName, std::ios::binary };
		outStream << settings;
		if( !outStream ) {
			throw std::runtime_error { "setting write error" };
		}
	}
}