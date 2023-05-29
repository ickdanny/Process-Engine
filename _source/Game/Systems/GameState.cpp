#include "Game/Systems/GameState.h"

#include <chrono>

namespace process::game::systems {

	void GameState::setPrngSeedToClock() {
		prngSeed = static_cast<unsigned int>(
			std::chrono::steady_clock::now().time_since_epoch().count()
		);
	}
}