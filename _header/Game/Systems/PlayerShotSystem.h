#pragma once

#include "systemInclude.h"

namespace process::game::systems {

	class PlayerShotSystem {
	private:
		//fields
		//todo: need way to access scripts
		//Programs* programsPointer{};

	public:
		
		/*
		PlayerShotSystem(Programs* programsPointer)
			: programsPointer{ programsPointer } {
		}
		 */

		void operator()(Scene& scene);

	private:
		//helper functions
		void addPlayerShot(Scene& scene);
	};
}