#pragma once

#include "systemInclude.h"

namespace process::game::systems {

	class RotateSpriteForwardSystem {
	public:
		void operator()(Scene& scene);
	};
}