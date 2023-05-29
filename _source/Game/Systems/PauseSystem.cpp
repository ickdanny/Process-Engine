#include "Game/Systems/PauseSystem.h"

namespace process::game::systems {

	void PauseSystem::operator()(Scene& scene){

		const auto& gameCommandChannel{
			scene.getChannel(SceneTopics::gameCommands)
		};
		for (GameCommands gameCommand : gameCommandChannel.getMessages()) {
			if (gameCommand == GameCommands::pause) {
				enterPauseMenu(scene);
			}
		}
	}

	void PauseSystem::enterPauseMenu(Scene& scene) {
		//push a scene entry message to the pause menu
		globalChannelSetPointer->getChannel(GlobalTopics::sceneEntry)
			.addMessage(SceneNames::pause);

		//pause the game
		scene.getChannel(SceneTopics::pauseFlag).addMessage();
	}
}