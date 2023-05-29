#pragma once

#include "systemInclude.h"

namespace process::game::systems {

	class InboundSystem {
	public:
		void operator()(Scene& scene);
	};
}