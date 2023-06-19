#pragma once

#include "systemInclude.h"
#include "ScriptStorage.h"

namespace process::game::systems {

	class PlayerShotSystem {
	private:
		//fields
		resources::ScriptStorage* scriptStoragePointer{};

	public:
		
		
		PlayerShotSystem(resources::ScriptStorage* scriptStoragePointer);
		

		void operator()(Scene& scene);

	private:
		//helper functions
		void addPlayerShot(Scene& scene);
	};
}