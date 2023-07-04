#include "Game/Systems/LoadSystem.h"

namespace process::game::systems {

	namespace {
		constexpr int waitTime{ 75 };
	}

	LoadSystem::LoadSystem(wasp::channel::ChannelSet* globalChannelSetPointer)
		: globalChannelSetPointer{ globalChannelSetPointer } {
	}

	void LoadSystem::operator()(Scene& scene) {
		if (scene.getName() == SceneNames::load) {
			auto& dataStorage{ scene.getDataStorage() };

			static const Topic<int> timerTopic{};
			auto& timerChannel{ scene.getChannel(timerTopic) };

			//if there is already a timer, handle it
			if (timerChannel.hasMessages()) {
				int& timer{ timerChannel.getMessages()[0] };

				if (timer <= 0) {
					globalChannelSetPointer->getChannel(GlobalTopics::sceneExitTo)
						.addMessage(SceneNames::game);
					timerChannel.clear();
				}
				--timer;
			}

			//otherwise, start a timer
			else {
				timerChannel.addMessage(waitTime);
			}
		}
	}
}