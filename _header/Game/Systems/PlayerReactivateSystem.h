#pragma once

#include "systemInclude.h"

namespace process::game::systems {

	class PlayerReactivateSystem {
	private:
		//typedefs
		using EntityHandle = wasp::ecs::entity::EntityHandle;

	public:
		void operator()(Scene& scene);

	private:
		//helper functions
		void reactivate(Scene& scene, const EntityHandle& playerHandle);
	};
}