#pragma once

#include "systemInclude.h"

namespace process::game::systems {

	class CreditsSystem {
	private:
		//fields
		wasp::channel::ChannelSet* globalChannelSetPointer{};

	public:
		CreditsSystem(wasp::channel::ChannelSet* globalChannelSetPointer);
		void operator()(Scene& scene);
	};
}