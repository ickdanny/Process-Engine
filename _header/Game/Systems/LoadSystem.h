#pragma once

#include "systemInclude.h"

namespace process::game::systems {

	class LoadSystem {
	private:
		//fields
		wasp::channel::ChannelSet* globalChannelSetPointer{};

	public:
		LoadSystem(wasp::channel::ChannelSet* globalChannelSetPointer);
		void operator()(Scene& scene);
	};
}