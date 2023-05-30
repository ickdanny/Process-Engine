#include "Game/Systems/MiscellaneousSystem.h"

namespace process::game::systems {

	namespace {
		//clears death channel and pause flag channel
		void clearChannels(Scene& scene) {
			scene.getChannel(SceneTopics::deaths).clear();
			scene.getChannel(SceneTopics::pauseFlag).clear();
		}
	}

	void MiscellaneousSystem::operator()(Scene& scene) {
		clearChannels(scene);
	}
}