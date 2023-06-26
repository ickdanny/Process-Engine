#pragma once

#include "systemInclude.h"

#include "ScriptStorage.h"

namespace process::game::systems {

	class DeathHandlerSystem {
	private:
		//typedefs
		using EntityHandle = wasp::ecs::entity::EntityHandle;
		using ScriptContainer = ScriptList::value_type;

		//fields
		ScriptContainer ghostScriptContainer;//uninitialized

	public:
		DeathHandlerSystem(resources::ScriptStorage& scriptStorage);

		void operator()(Scene& scene);

	private:
		//helper functions
		void handleDeath(
			Scene& scene,
			const EntityHandle& entityHandle,
			const DeathCommand::Commands command
		);
		void handlePlayerDeath(Scene& scene, const EntityHandle& playerHandle);
		void handleBossDeath(Scene& scene, const EntityHandle& bossHandle);
		void handleDeathSpawn(
			Scene& scene,
			const EntityHandle& entityHandle,
			bool removeEntity
		);
	};
}