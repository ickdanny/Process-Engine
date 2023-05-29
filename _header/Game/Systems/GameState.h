#pragma once

#include "ShotType.h"

namespace process::game::systems {

	enum class GameMode {
		campaign,
		practice
	};
	enum class Difficulty {
		easy,
		normal,
		hard,
		lunatic
	};

	struct GameState {
		//fields
		GameMode gameMode{};
		Difficulty difficulty{};
		ShotType shotType{};
		int stage{};
		unsigned int prngSeed{};

		//methods
		void setPrngSeedToClock();
	};
}