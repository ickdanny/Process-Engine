#pragma once

#include "systemInclude.h"

namespace process::game::systems {

	class PlayerStateSystem {
	private:
		//typedefs
		using EntityHandle = wasp::ecs::entity::EntityHandle;

	public:
		void operator()(Scene& scene);
	};
}