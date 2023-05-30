#pragma once

#include "systemInclude.h"

namespace process::game::systems {

	class PlayerRespawnSystem {
	private:
		//typedefs
		using EntityHandle = wasp::ecs::entity::EntityHandle;

	public:
		void operator()(Scene& scene);

	private:
		//helper functions
		void respawn(Scene& scene, const EntityHandle& playerHandle);
	};
}