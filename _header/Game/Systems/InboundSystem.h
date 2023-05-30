#pragma once

#include "systemInclude.h"

namespace process::game::systems {

	class InboundSystem {
	private:
		using Group = wasp::ecs::component::Group;
	public:
		void operator()(Scene& scene);
	};
}