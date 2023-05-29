#pragma once

#include "systemInclude.h"

namespace process::game::systems {

	class PlayerDeathDetectorSystem {
	private:
		//typedefs
		using EntityHandle = ecs::entity::EntityHandle;
	public:
		void operator()(Scene& scene);
	};
}