#pragma once

namespace process::game::systems {
	enum class PlayerStates {
		none,
		normal,
		bombing,
		dead,
		respawning,
		respawnInvulnerable,
		gameOver
	};
}