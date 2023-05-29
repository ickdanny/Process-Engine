#include "Game/Systems/ContinueSystem.h"

#include "Logging.h"

namespace process::game::systems {

	void ContinueSystem::operator()(Scene& scene) {

		auto& dataStorage{ scene.getDataStorage() };

		//if there is already a continue timer, handle that
		static const Topic<SceneData> sceneDataTopic{};
		auto& sceneDataChannel{ scene.getChannel(sceneDataTopic) };
		if (sceneDataChannel.hasMessages()) {
			auto& sceneDataVector{ sceneDataChannel.getMessages() };
			for (auto itr{ sceneDataVector.begin() }; itr != sceneDataVector.end();) {
				auto& [playerHandle, continueTimer] = *itr;
				//if our timer has hit 0, enter the continue screen
				if (continueTimer == 0) {
					enterContinueMenu(scene, playerHandle);
				}
				//if our timer is below 0, that means the player has chosen to continue
				else if (continueTimer < 0) {
					handleContinue(scene, playerHandle);
					//remove this timer from the message list
					itr = sceneDataVector.erase(itr);
					continue;
				}
				--continueTimer;
				++itr;
			}
		}

		//otherwise, check to see if we need to begin a continue timer
		else {
			if (scene.hasChannel(SceneTopics::playerStateEntry)) {
				const auto& playerStateEntryChannel{
					scene.getChannel(SceneTopics::playerStateEntry)
				};
				for (const auto& [playerHandle, playerState]
					: playerStateEntryChannel.getMessages()
				) {
					if (playerState == PlayerStates::dead) {
						if (!dataStorage.containsComponent<PlayerData>(playerHandle)) {
							throw std::runtime_error{ 
								"cannot find player data in continue system!" 
							};
						}
						PlayerData& playerData{
							dataStorage.getComponent<PlayerData>(playerHandle)
						};
						if (playerData.lives <= 0 && playerData.continues > 0) {
							sceneDataChannel.addMessage(
								{ playerHandle, config::deathPeriod - 2 }
							);
						}
						//if continues <= 0 this system does nothing; see gameOverSystem
					}
				}
			}
		}
	}

	//Handles the return from the continue menu
	void ContinueSystem::handleContinue(
		Scene& scene,
		const EntityHandle& playerHandle
	) {
		auto& dataStorage{ scene.getDataStorage() };
		if (!dataStorage.containsComponent<PlayerData>(playerHandle)) {
			throw std::runtime_error{ "cannot find player data in continue system!" };
		}
		PlayerData& playerData{
			dataStorage.getComponent<PlayerData>(playerHandle)
		};
		if (playerData.continues > 0) {
			--playerData.continues;
		}
	}

	//playerHandle is unused
	void ContinueSystem::enterContinueMenu(
		Scene& scene, 
		const EntityHandle& playerHandle
	) {
		//push a scene entry message to the continue menu
		globalChannelSetPointer->getChannel(GlobalTopics::sceneEntry)
			.addMessage(SceneNames::continues);

		//pause the game
		scene.getChannel(SceneTopics::pauseFlag).addMessage();

		//send player data to global
		auto& dataStorage{ scene.getDataStorage() };
		PlayerData& playerData{
			dataStorage.getComponent<PlayerData>(playerHandle)
		};
		auto& playerDataChannel{
			globalChannelSetPointer->getChannel(GlobalTopics::playerData)
		};
		playerDataChannel.clear();
		playerDataChannel.addMessage(playerData);
	}
}