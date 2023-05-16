#include "Game/Resources/Dialogue.h"

#include <fstream>
#include <sstream>
#include <unordered_map>

namespace wasp::game::resources {
	
	DialogueCommand::Commands parseCommandString(const std::wstring& commandString) {
		static const std::unordered_map<std::wstring, DialogueCommand::Commands> map {
			{ L"left_image",  DialogueCommand::Commands::setLeftImage },
			{ L"right_image", DialogueCommand::Commands::setRightImage },
			{ L"text",        DialogueCommand::Commands::setText },
			{ L"track",       DialogueCommand::Commands::setTrack },
			{ L"stop",        DialogueCommand::Commands::stop },
			{ L"end",         DialogueCommand::Commands::end }
		};
		
		if( map.find(commandString) != map.end() ) {
			return map.find(commandString)->second;
		}
		else {
			return DialogueCommand::Commands::error;
		}
	}
	
	DialogueCommand parseDialogueLine(const std::wstring& line) {
		std::wstringstream stringStream { line };
		
		std::wstring commandString {};
		std::getline(stringStream, commandString, L']');
		
		std::wstring data {};
		std::getline(stringStream, data, L']');
		
		return { parseCommandString(commandString), data };
	}
	
	Dialogue parseDialogueFile(const std::wstring& fileName) {
		
		Dialogue dialogue {};
		
		std::wifstream inStream { fileName };
		std::wstring line {};
		
		while( std::getline(inStream, line) ) {
			dialogue.push_back(parseDialogueLine(line));
		}
		
		inStream.close();
		return dialogue;
	}
}