#include "Game/Game.h"

#include "Logging.h"

namespace process::game {

	Game::Game(
		wasp::game::Settings* settingsPointer,
		resources::ResourceMasterStorage* resourceMasterStoragePointer,
		window::GraphicsWrapper* graphicsWrapperPointer,
		wasp::input::IKeyInputTable* keyInputTablePointer,
		wasp::sound::midi::MidiHub* midiHubPointer
	)
		: sceneList{ std::move(makeSceneList()) }
		, sceneUpdater{ 
			resourceMasterStoragePointer, 
			keyInputTablePointer, 
			&globalChannelSet 
		}
		, sceneRenderer{ graphicsWrapperPointer, resourceMasterStoragePointer->spriteStorage }
		, settingsPointer{ settingsPointer }
		, resourceMasterStoragePointer{ resourceMasterStoragePointer }
		, graphicsWrapperPointer{ graphicsWrapperPointer }
		, keyInputTablePointer{ keyInputTablePointer }
		, midiHubPointer{ midiHubPointer }
	{
		sceneList.pushScene(SceneNames::main);
	}

	void Game::update() {
		for (auto itr{ sceneList.rbegin() }; itr != sceneList.rend(); ++itr) {
			auto& scene{ *(*itr) };	//dereference itr and shared_ptr
			sceneUpdater(scene);
			if (!scene.isTransparent(SystemChainIDs::update)) {
				break;
			}
		}

		if (wasExitFlagRaised()) {
			exitCallback();
			return;
		}

		updateSceneList();
		updateInput();
		updateMusic();
		updateSettings();
	}

	bool Game::wasExitFlagRaised() {
		return globalChannelSet.getChannel(GlobalTopics::exitFlag).hasMessages();
	}

	void Game::updateSceneList() {		
		//lock input (don't let new scenes see input)
		keyInputTablePointer->lockAll();

		//pop scenes first
		auto& sceneExitToChannel{ 
			globalChannelSet.getChannel(GlobalTopics::sceneExitTo) 
		};
		if (sceneExitToChannel.hasMessages()) {
			auto& sceneExitToMessages{ sceneExitToChannel.getMessages() };
			if (sceneExitToMessages.size() > 1) {
				throw std::runtime_error{ "trying to exit to 2 scenes at once!" };
			}
			sceneList.popBackTo(sceneExitToMessages[0]);
			sceneExitToChannel.clear();
		}

		//then push and update new scenes
		auto& sceneEntryChannel{ globalChannelSet.getChannel(GlobalTopics::sceneEntry) };
		if (sceneEntryChannel.hasMessages()) {
			for (SceneNames sceneName : sceneEntryChannel.getMessages()) {
				sceneList.pushScene(sceneName);
				sceneUpdater(*(*sceneList.rbegin()));	//dereference itr and shared_ptr
			}
			sceneEntryChannel.clear();
		}
	}
	void Game::updateInput() {
		keyInputTablePointer->tickOver();
	}
	void Game::updateMusic() {
		//check stop flag
		auto& stopMusicFlagChannel{
			globalChannelSet.getChannel(GlobalTopics::stopMusicFlag)
		};
		if (stopMusicFlagChannel.hasMessages()) {
			midiHubPointer->stop();
			stopMusicFlagChannel.clear();
		}

		//check startMusic topic
		auto& startMusicChannel{
			globalChannelSet.getChannel(GlobalTopics::startMusic)
		};
		if (startMusicChannel.hasMessages()) {
			//if more than 1 start command (somehow), only start the last one
			const std::wstring& lastTrackName{ 
				*startMusicChannel.getMessages().rbegin() 
			};
			const auto& midiSequencePointer{
				resourceMasterStoragePointer->midiSequenceStorage.get(lastTrackName)
			};
			if (midiSequencePointer) {
				midiHubPointer->start(midiSequencePointer);
			}
			else {
				wasp::debug::log("can't find midisequence called:");
				wasp::debug::log(lastTrackName);
			}
			startMusicChannel.clear();
		}
	}
	void Game::updateSettings() {
		//toggle muted
		auto& soundToggleFlagChannel{
			globalChannelSet.getChannel(GlobalTopics::toggleSoundFlag)
		};
		if (soundToggleFlagChannel.hasMessages()) {
			settingsPointer->muted = !settingsPointer->muted;
			midiHubPointer->toggleMute();
			soundToggleFlagChannel.clear();

			if (!midiHubPointer->isMuted()) {
				globalChannelSet.getChannel(GlobalTopics::startMusic).addMessage(L"01");
			}
		}

		//toggle fullscreen
		auto& fullscreenToggleFlagChannel{
			globalChannelSet.getChannel(GlobalTopics::toggleFullscreenFlag)
		};
		if (fullscreenToggleFlagChannel.hasMessages()) {
			settingsPointer->fullscreen = !settingsPointer->fullscreen;
			updateFullscreenCallback();
			fullscreenToggleFlagChannel.clear();
		}
		
		//write settings to disk
		auto& writeSettingsFlagChannel{
			globalChannelSet.getChannel(GlobalTopics::writeSettingsFlag)
		};
		if (writeSettingsFlagChannel.hasMessages()) {
			writeSettingsCallback();
			writeSettingsFlagChannel.clear();
		}
	}

	void Game::render() {
		recursiveRenderHelper(sceneList.rbegin());
	}

	void Game::recursiveRenderHelper(const SceneList::ReverseIterator& itr) {
		auto& scene{ *(*itr) };	//dereference itr and shared_ptr
		if (itr != sceneList.rend()
				&& scene.isTransparent(SystemChainIDs::render))
		{
			recursiveRenderHelper(itr + 1);
		}
		sceneRenderer(scene);
		//for each scene, we need to clear the depth since painter's algorithm
		graphicsWrapperPointer->clearDepth();
	}
}