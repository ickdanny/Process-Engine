#pragma once

namespace wasp::game::components {

	struct DeathCommand {
        //fields
        enum class Commands {
            playerDeath,
            bossDeath,
            deathSpawn
        } command{};
	};
}