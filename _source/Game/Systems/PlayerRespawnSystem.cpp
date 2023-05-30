#include "Game/Systems/PlayerRespawnSystem.h"

#include "Logging.h"

namespace process::game::systems {

	void PlayerRespawnSystem::operator()(Scene& scene) {
		if (scene.hasChannel(SceneTopics::playerStateEntry)) {
			const auto& playerStateEntryChannel{
				scene.getChannel(SceneTopics::playerStateEntry)
			};
			for (const auto& [playerHandle, playerState]
				: playerStateEntryChannel.getMessages()
			) {
				if (playerState == PlayerStates::respawning) {
					respawn(scene, playerHandle);
				}
			}
		}
	}

	void PlayerRespawnSystem::respawn(Scene& scene, const EntityHandle& playerHandle) {
		auto& dataStorage{ scene.getDataStorage() };

		if (!dataStorage.containsComponent<PlayerData>(playerHandle)) {
			throw std::runtime_error{ "cannot find player data in respawn system!" };
		}

		//make the player collidable
		dataStorage.addComponent<CollidableMarker>(
			wasp::ecs::AddComponentOrder<CollidableMarker>{ playerHandle, {} }
		);

		//reset the player's position to spawn
		dataStorage.setComponent<Position>(
			wasp::ecs::SetComponentOrder<Position>{ playerHandle, config::playerSpawn }
		);

		PlayerData& playerData{ dataStorage.getComponent<PlayerData>(playerHandle) };
		playerData.bombs = config::respawnBombs;
		if (playerData.lives > 0) {
			--playerData.lives;
		}
		else {
			//this only occurs when the player uses a continue
			//continues get used up in the ContinueSystem
			playerData.lives = config::continueLives;
		}
	}
}