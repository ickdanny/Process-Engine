#pragma once

#include "systemInclude.h"

namespace process::game::systems {

	class DeathHandlerSystem {
	private:
		//typedefs
		using EntityHandle = ecs::entity::EntityHandle;
		using ScriptNode = components::ScriptNode;
		using ScriptInstructions = components::ScriptInstructions;

		//fields
		components::ScriptProgram ghostProgram;		//uninitialized!

	public:
		DeathHandlerSystem();

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