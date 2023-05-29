#pragma once

#include "systemInclude.h"

namespace process::game::systems {
	class CollisionDetectorSystem {
	public:
		void operator()(Scene& scene);
	};
}