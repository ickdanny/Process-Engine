#pragma once

#include "systemInclude.h"

namespace process::game::systems {

	class CreditsSystem {
	private:
		//fields
		channel::ChannelSet* globalChannelSetPointer{};

	public:
		CreditsSystem(channel::ChannelSet* globalChannelSetPointer);
		void operator()(Scene& scene);
	};
}