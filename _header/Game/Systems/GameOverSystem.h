#pragma once

#include "systemInclude.h"

namespace process::game::systems {

	class GameOverSystem {
	private:
		//fields
		wasp::channel::ChannelSet* globalChannelSetPointer{};

	public:
		GameOverSystem(wasp::channel::ChannelSet* globalChannelSetPointer)
			: globalChannelSetPointer{ globalChannelSetPointer } {
		}

		void operator()(Scene& scene);

	private:
		//helper functions
		void gameOver();
	};
}