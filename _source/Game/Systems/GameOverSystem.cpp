#include "Game/Systems/GameOverSystem.h"

namespace process::game::systems {

	void GameOverSystem::operator()(Scene& scene) {
		if (scene.hasChannel(SceneTopics::playerStateEntry)) {
			const auto& playerStateEntryChannel{
				scene.getChannel(SceneTopics::playerStateEntry)
			};
			for (const auto& [playerHandle, playerState]
				: playerStateEntryChannel.getMessages()
				) {
				if (playerState == PlayerStates::gameOver) {
					gameOver();
				}
			}
		}
	}

	void GameOverSystem::gameOver() {
		//handle music
		globalChannelSetPointer->getChannel(GlobalTopics::startMusic).addMessage(
			L"01"
		);
		
		//send us back to the correct menu
		auto& gameStateChannel{
			globalChannelSetPointer->getChannel(GlobalTopics::gameState)
		};
		GameState& gameState{ gameStateChannel.getMessages()[0] };

		SceneNames backTo{};
		switch (gameState.gameMode) {
			case GameMode::campaign:
				backTo = SceneNames::main;
				break;
			case GameMode::practice:
				backTo = SceneNames::stage;
				break;
			default:
				throw std::runtime_error{
					"default case reached in GameOverSystem.gameOver()!"
				};
		}

		globalChannelSetPointer->getChannel(GlobalTopics::sceneExitTo).addMessage(
			backTo
		);
	}
}