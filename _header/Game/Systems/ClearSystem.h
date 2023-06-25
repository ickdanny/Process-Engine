#pragma once

#include "systemInclude.h"

namespace process::game::systems {
	
	class ClearSystem {
	private:
		//typedefs
		using EntityID = wasp::ecs::entity::EntityID;
		using EntityHandle = wasp::ecs::entity::EntityHandle;
		using Group = wasp::ecs::component::Group;
	
	public:
		void operator()(Scene& scene);
	
	private:
		//finds every entity marked with ClearMarker and broadcasts their death
		void handleClear(Scene& scene);
	};
}