#pragma once

#include "systemInclude.h"

#include "Game/Systems/Programs/Programs.h"

namespace process::game::systems {

	class PlayerBombSystem {
	private:
		//typedefs
		using EntityHandle = ecs::entity::EntityHandle;

		//fields
		Programs* programsPointer{};

	public:
		PlayerBombSystem(Programs* programsPointer)
			: programsPointer{ programsPointer } {
		}

		void operator()(Scene& scene);

	private:
		//helper functions
		void bomb(Scene& scene, const EntityHandle& playerHandle);
	};
}