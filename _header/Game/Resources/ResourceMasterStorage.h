#pragma once

#include "Game/Resources/DirectoryStorage.h"
#include "Game/Resources/ManifestStorage.h"
#include "Game/Resources/MidiSequenceStorage.h"
#include "Game/Resources/DialogueStorage.h"
#include "Game/Resources/SpriteStorage.h"

namespace process::game::resources {
	struct ResourceMasterStorage {
		DirectoryStorage directoryStorage {};
		ManifestStorage manifestStorage {};
		SpriteStorage spriteStorage{};
		MidiSequenceStorage midiSequenceStorage {};
		DialogueStorage dialogueStorage {};
	};
}