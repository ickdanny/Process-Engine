#pragma once

#include "systemInclude.h"

#include "Game/Systems/Programs/Programs.h"

namespace process::game::systems {

	class PlayerShotSystem {
	private:
		//fields
		Programs* programsPointer{};

	public:
		PlayerShotSystem(Programs* programsPointer)
			: programsPointer{ programsPointer } {
		}

		void operator()(Scene& scene);

	private:
		//helper functions
		void addPlayerShot(Scene& scene);
	};
}