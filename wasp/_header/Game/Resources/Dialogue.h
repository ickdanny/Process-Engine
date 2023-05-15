#pragma once

#include <string>
#include <vector>

//not sure where to put this
namespace wasp::game::resources {
	
	struct DialogueCommand {
		enum class Commands {
			error,			//crashes if seen
			setLeftImage,	//sets image on the left
			setRightImage,	//sets image on the right
			setText,		//sets the text
			setTrack,		//starts a new music track
			stop,			//stops dialogue (waits for next input)
			end				//ends dialogue
		} command{};

		std::wstring data{};
	};

	using Dialogue = std::vector<DialogueCommand>;

	Dialogue parseDialogueFile(const std::wstring& fileName);
}