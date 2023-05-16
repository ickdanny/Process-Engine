#pragma once

#include "Game/Resources/DirectoryStorage.h"
#include "Game/Resources/ManifestStorage.h"
#include "Game/Resources/MidiSequenceStorage.h"
#include "Game/Resources/DialogueStorage.h"

namespace process::game::resources {
	struct ResourceMasterStorage {
		////BitmapStorage bitmapStorage{}; //todo: bitmap resource storage
		game::resources::DirectoryStorage directoryStorage {};
		game::resources::ManifestStorage manifestStorage {};
		game::resources::MidiSequenceStorage midiSequenceStorage {};
		game::resources::DialogueStorage dialogueStorage {};
	};
}