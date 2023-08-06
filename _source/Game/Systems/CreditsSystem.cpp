#include "Game/Systems/CreditsSystem.h"

namespace process::game::systems {

	namespace {
		constexpr int waitTime{ 60 * 43 };
	}

	CreditsSystem::CreditsSystem(wasp::channel::ChannelSet* globalChannelSetPointer)
		: globalChannelSetPointer{ globalChannelSetPointer } {
	}

	void CreditsSystem::operator()(Scene& scene) {
		if (scene.getName() == SceneNames::credits) {
			auto& dataStorage{ scene.getDataStorage() };

			static const Topic<int> timerTopic{};
			auto& timerChannel{ scene.getChannel(timerTopic) };

			//if there is already a timer, handle it
			if (timerChannel.hasMessages()) {
				int& timer{ timerChannel.getMessages()[0] };

				if (timer <= 0) {
					globalChannelSetPointer->getChannel(GlobalTopics::sceneExitTo)
						.addMessage(SceneNames::main);
					globalChannelSetPointer->getChannel(GlobalTopics::startMusic)
						.addMessage(L"01");
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