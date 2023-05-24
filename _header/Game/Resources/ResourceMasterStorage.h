#pragma once

#include "Game/Resources/DirectoryStorage.h"
#include "Game/Resources/ManifestStorage.h"
#include "Game/Resources/MidiSequenceStorage.h"
#include "Game/Resources/DialogueStorage.h"
#include "Game/Resources/TextureStorage.h"

namespace process::game::resources {
	struct ResourceMasterStorage {
		DirectoryStorage directoryStorage {};
		ManifestStorage manifestStorage {};
		TextureStorage textureStorage{};
		MidiSequenceStorage midiSequenceStorage {};
		DialogueStorage dialogueStorage {};
	};
}