#pragma once

#include "systemInclude.h"

namespace process::game::systems {

	class PlayerBombSystem {
	private:
		//typedefs
		using EntityHandle = wasp::ecs::entity::EntityHandle;

		//fields
		//todo: need way to access scripts Programs* programsPointer{};

	public:
		
		/*
		PlayerBombSystem(Programs* programsPointer)
			: programsPointer{ programsPointer } {
		}
		 */

		void operator()(Scene& scene);

	private:
		//helper functions
		void bomb(Scene& scene, const EntityHandle& playerHandle);
	};
}