#pragma once

#include "systemInclude.h"

namespace process::game::systems {

	class ContinueSystem {
	private:
		//typedefs
		using EntityHandle = wasp::ecs::entity::EntityHandle;
		using SceneData = std::tuple<EntityHandle, int>;

		//fields
		wasp::channel::ChannelSet* globalChannelSetPointer{};

	public:
		ContinueSystem(wasp::channel::ChannelSet* globalChannelSetPointer)
			: globalChannelSetPointer{ globalChannelSetPointer } {
		}

		void operator()(Scene& scene);

	private:
		void handleContinue(Scene& scene, const EntityHandle& playerHandle);
		void enterContinueMenu(Scene& scene, const EntityHandle& playerHandle);
	};
}