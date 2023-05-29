#pragma once

#include "systemInclude.h"

namespace process::game::systems {

	class OutboundSystem {
	public:
		void operator()(Scene& scene);
	};
}