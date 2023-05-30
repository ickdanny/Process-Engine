#pragma once

#include "Resources/ResourceMasterStorage.h"
#include "Scenes.h"
#include "Topics.h"
#include "SceneUpdater.h"
#include "SceneRenderer.h"
#include "Sound/MidiHub.h"
#include "Settings.h"

#pragma warning(suppress : 4068) //suppress unknown pragma
#pragma clang diagnostic push
#pragma warning(suppress : 4068) //suppress unknown pragma
#pragma clang diagnostic ignored "-Wshadow"

namespace process::game {
	
	class Game {
	private:
		//typedefs
		using ChannelSet = wasp::channel::ChannelSet;

		//fields
		SceneList sceneList;			//not initialized!
		ChannelSet globalChannelSet{};

		SceneUpdater sceneUpdater;		//not initialized!
		SceneRenderer sceneRenderer;	//not initialized!

		wasp::game::Settings* settingsPointer{};
		resources::ResourceMasterStorage* resourceMasterStoragePointer{};
		window::GraphicsWrapper* graphicsWrapperPointer{};
		wasp::input::IKeyInputTable* keyInputTablePointer{};
		wasp::sound::midi::MidiHub* midiHubPointer{};

		std::function<void()> exitCallback{};
		std::function<void()> updateFullscreenCallback{};
		std::function<void()> writeSettingsCallback{};
		
	public:
		//constructor
		Game(
			wasp::game::Settings* settingsPointer,
			resources::ResourceMasterStorage* resourceMasterStoragePointer,
			window::GraphicsWrapper* graphicsWrapperPointer,
			wasp::input::IKeyInputTable* keyInputTablePointer,
			wasp::sound::midi::MidiHub* midiHubPointer
		);

		void update();

		void render();

		void setExitCallback(const std::function<void()>& exitCallback) {
			this->exitCallback = exitCallback;
		}

		void setUpdateFullscreenCallback(
			const std::function<void()>& updateFullscreenCallback
		) {
			this->updateFullscreenCallback = updateFullscreenCallback;
		}

		void setWriteSettingsCallback(
			const std::function<void()>& writeSettingsCallback
		) {
			this->writeSettingsCallback = writeSettingsCallback;
		}

	private:
		bool wasExitFlagRaised();
		void updateSceneList();
		void updateInput();
		void updateMusic();
		void updateSettings();

		void recursiveRenderHelper(const SceneList::ReverseIterator& itr);
	};
}
#pragma warning(suppress : 4068) //suppress unknown pragma
#pragma clang diagnostic pop