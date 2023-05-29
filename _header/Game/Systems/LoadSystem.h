#pragma once

#include "systemInclude.h"

namespace process::game::systems {

	class LoadSystem {
	private:
		//fields
		channel::ChannelSet* globalChannelSetPointer{};

	public:
		LoadSystem(channel::ChannelSet* globalChannelSetPointer);
		void operator()(Scene& scene);
	};
}