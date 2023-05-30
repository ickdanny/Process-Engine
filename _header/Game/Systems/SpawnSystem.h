#pragma once

#include "systemInclude.h"
#include "Game/Systems/EntityBuilder.h"

namespace process::game::systems {

	class SpawnSystem {

	private:
		//typedefs
		using EntityID = wasp::ecs::entity::EntityID;
		using SpawnList = std::vector<std::shared_ptr<ComponentTupleBase>>;
		using SpawnProgram = wasp::game::components::SpawnProgram;
		using Point2 = wasp::math::Point2;
		using SpawnInstructions = wasp::game::components::SpawnInstructions;
		template <typename T>
		using SpawnNodeData = wasp::game::components::SpawnNodeData<T>;
		using SpawnNode = wasp::game::components::SpawnNode;

		//fields
		wasp::channel::ChannelSet* globalChannelSetPointer{};

	public:
		SpawnSystem(wasp::channel::ChannelSet* globalChannelSetPointer);
		void operator()(Scene& scene);

	private:
		//helper functions
		void runSpawnProgramList(
			Scene& scene,
			SpawnProgramList& spawnProgramList,
			EntityID entityID,
			SpawnList& spawnList
		);

		//Returns true if the program can continue, false if the program is over.
		bool runSpawnProgram(
			Scene& scene,
			SpawnProgram& spawnProgram,
			EntityID entityID,
			SpawnList& spawnList
		);

		#define NODE_HANDLER_ARGS \
			Scene& scene, \
			EntityID entityID, \
			std::shared_ptr<wasp::game::components::SpawnNode>& currentSpawnNodePointer, \
			int tick, \
			SpawnList& spawnList

		void runSpawnNode(NODE_HANDLER_ARGS);
		void runSpawnNodePassingPos(NODE_HANDLER_ARGS, const Point2& pos);
		void runSpawnNodePassingVel(NODE_HANDLER_ARGS, const Velocity& vel);
		void runSpawnNodePassingPosVel(
			NODE_HANDLER_ARGS,
			const wasp::math::Point2& pos,
			const Velocity& vel
		);
		int evaluateIntNode(NODE_HANDLER_ARGS);
		float evaluateFloatNode(NODE_HANDLER_ARGS);
		bool evaluatePredicateNode(NODE_HANDLER_ARGS);
		Velocity evaluateVelocityNode(NODE_HANDLER_ARGS);
		Point2 evaluatePointNode(NODE_HANDLER_ARGS);

		#undef NODE_HANDLER_ARGS

		Difficulty getDifficulty();
	};
}