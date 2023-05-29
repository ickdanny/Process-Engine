#pragma once

#include "systemInclude.h"

namespace process::game::systems {

	class PlayerReactivateSystem {
	private:
		//typedefs
		using EntityHandle = ecs::entity::EntityHandle;

	public:
		void operator()(Scene& scene);

	private:
		//helper functions
		void reactivate(Scene& scene, const EntityHandle& playerHandle);
	};
}