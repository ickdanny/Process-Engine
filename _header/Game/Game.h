#pragma once

#include "Resources/ResourceMasterStorage.h"
#include "Scenes.h"
#include "Topics.h"
#include "SceneUpdater.h"
#include "SceneRenderer.h"
#include "Sound/MidiHub.h"
#include "Settings.h"

namespace process::game {

	//stores and acts upon the logical state of the game
	class Game {
	private:
		//typedefs
		using ChannelSet = wasp::channel::ChannelSet;

		//fields
		SceneList sceneList;			//not initialized!
		ChannelSet globalChannelSet{};

		SceneUpdater sceneUpdater;		//not initialized!
		SceneRenderer sceneRenderer;	//not initialized!

		Settings* settingsPointer{};
		resources::ResourceMasterStorage* resourceMasterStoragePointer{};
		input::IKeyInputTable* keyInputTablePointer{};
		sound::midi::MidiHub* midiHubPointer{};

		std::function<void()> exitCallback{};
		std::function<void()> updateFullscreenCallback{};
		std::function<void()> writeSettingsCallback{};
		
	public:
		//constructor
		Game(
			Settings* settingsPointer,
			resources::ResourceMasterStorage* resourceMasterStoragePointer,
			window::WindowPainter* windowPainterPointer,
			input::IKeyInputTable* keyInputTablePointer,
			sound::midi::MidiHub* midiHubPointer
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

		void recursiveRenderHelper(
			float deltaTime, 
			const SceneList::ReverseIterator& itr
		);
	};
}