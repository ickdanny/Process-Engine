#include "Game/Systems/PlayerBombSystem.h"

#include "Logging.h"

namespace process::game::systems {
	
	PlayerBombSystem::PlayerBombSystem(resources::ScriptStorage* scriptStoragePointer)
		: scriptStoragePointer{ scriptStoragePointer } {
	}

	void PlayerBombSystem::operator()(Scene& scene) {
		if (scene.hasChannel(SceneTopics::playerStateEntry)) {
			const auto& playerStateEntryChannel{
				scene.getChannel(SceneTopics::playerStateEntry)
			};
			for (const auto& [playerHandle, playerState]
				: playerStateEntryChannel.getMessages()
			) {
				if (playerState == PlayerStates::bombing) {
					bomb(scene, playerHandle);
				}
			}
		}
	}

	void PlayerBombSystem::bomb(Scene& scene, const EntityHandle& playerHandle) {
		auto& dataStorage{ scene.getDataStorage() };

		if (!dataStorage.containsComponent<PlayerData>(playerHandle)) {
			throw std::runtime_error{ "cannot find player data for bomb!" };
		}
		if (!dataStorage.containsComponent<ScriptList>(playerHandle)) {
			throw std::runtime_error{ "player has no spawn data at bomb!" };
		}

		auto& playerData{ dataStorage.getComponent<PlayerData>(playerHandle) };
		auto& scriptList{
			dataStorage.getComponent<ScriptList>(playerHandle)
		};
		if (playerData.shotType == ShotType::shotA) {
			scriptList.push_back({
				scriptStoragePointer->get(L"bombA"),
				"bombA"		//don't mark as a spawn script since shot system uses that
			});
		}
		else if (playerData.shotType == ShotType::shotB) {
			scriptList.push_back({
				scriptStoragePointer->get(L"bombB"),
				"bombB"		//don't mark as a spawn script since shot system uses that
			});
		}
		else {
			throw std::runtime_error("unexpected player shot type!");
		}
		--playerData.bombs;
	}
}