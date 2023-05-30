#pragma once

#include "systemInclude.h"

namespace process::game::systems {

	class PauseSystem {
	private:
		//fields
		wasp::channel::ChannelSet* globalChannelSetPointer{};

	public:
		PauseSystem(wasp::channel::ChannelSet* globalChannelSetPointer)
			: globalChannelSetPointer{ globalChannelSetPointer } {
		}

		void operator()(Scene& scene);

	private:
		void enterPauseMenu(Scene& scene);
	};
}