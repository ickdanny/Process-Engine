#include "Game/Systems/PlayerDeathDetectorSystem.h"

namespace process::game::systems {

	void PlayerDeathDetectorSystem::operator()(Scene& scene) {
		if (scene.hasChannel(SceneTopics::playerStateEntry)) {
			const auto& playerStateEntryChannel{
				scene.getChannel(SceneTopics::playerStateEntry)
			};
			for (const auto& [playerHandle, playerState]
				: playerStateEntryChannel.getMessages()
			) {
				if (playerState == PlayerStates::dead) {
					scene.getChannel(SceneTopics::deaths).addMessage(playerHandle);
				}
			}
		}
	}
}