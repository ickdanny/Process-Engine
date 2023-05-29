#include "Game/Systems/MiscellaneousSystem.h"

namespace process::game::systems {

	namespace {
		void resetLastDeltaTime(Scene& scene) {
			auto& lastDeltaTimeChannel{ 
				scene.getChannel(SceneTopics::lastDeltaTime) 
			};
			lastDeltaTimeChannel.clear();
			lastDeltaTimeChannel.addMessage(0.0f);
		}

		//clears death channel and pause flag channel
		void clearChannels(Scene& scene) {
			scene.getChannel(SceneTopics::deaths).clear();
			scene.getChannel(SceneTopics::pauseFlag).clear();
		}
	}

	void MiscellaneousSystem::operator()(Scene& scene) {
		resetLastDeltaTime(scene);
		clearChannels(scene);
	}
}