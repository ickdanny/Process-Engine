#pragma once

#include "systemInclude.h"

namespace process::game::systems {

	class GameBuilderSystem {

	private:
		//fields
		channel::ChannelSet* globalChannelSetPointer{};
	public:
		GameBuilderSystem(channel::ChannelSet* globalChannelSetPointer)
			: globalChannelSetPointer{ globalChannelSetPointer } {
		}

		void operator()(Scene& scene);

	private:
		//helper functions
		void handleGameBuilderCommand(GameBuilderCommands gameBuilderCommand);

		GameState* retrieveOrInitGameStatePointer();

		//makes sure the game state is prepared to start a game
		void finalizeGameState(GameState* gameStatePointer);
	};
}