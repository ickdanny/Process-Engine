#pragma once

#include "systemInclude.h"

#include "Resources/ScriptStorage.h"

namespace process::game::systems {

	class PlayerBombSystem {
	private:
		//typedefs
		using EntityHandle = wasp::ecs::entity::EntityHandle;

		//fields
		resources::ScriptStorage* scriptStoragePointer{};

	public:
		explicit PlayerBombSystem(resources::ScriptStorage* scriptStoragePointer);

		void operator()(Scene& scene);

	private:
		//helper functions
		void bomb(Scene& scene, const EntityHandle& playerHandle);
	};
}