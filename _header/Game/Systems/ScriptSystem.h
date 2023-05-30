#pragma once

#include "systemInclude.h"

#include "Game/Systems/ComponentOrderQueue.h"

namespace process::game::systems {

	class ScriptSystem {
	private:
		//typedefs
		using ScriptInstructions = wasp::game::components::ScriptInstructions;
		using ScriptNode = wasp::game::components::ScriptNode;
		using ScriptProgram = wasp::game::components::ScriptProgram;
		template <typename Internal, typename External>
		using ScriptNodeData = wasp::game::components::ScriptNodeData<Internal, External>;
		
		using EntityID = wasp::ecs::entity::EntityID;

		//fields
		wasp::channel::ChannelSet* globalChannelSetPointer{};

	public:
		ScriptSystem(wasp::channel::ChannelSet* globalChannelSetPointer);
		void operator()(Scene& scene);

	private:
		//helper functions
		void runScriptProgramList(
			Scene& scene,
			ScriptProgramList& scriptProgramList,
			EntityID entityID,
			ComponentOrderQueue& componentOrderQueue
		);

		#define NODE_HANDLER_ARGS \
			Scene& scene, \
			EntityID entityID, \
			std::shared_ptr<ScriptNode>& currentScriptNodePointer, \
			std::unordered_map<ScriptNode*, void*>& externalData, \
			ComponentOrderQueue& componentOrderQueue

		bool runScriptNode(NODE_HANDLER_ARGS);
		Velocity evaluateVelocityNode(NODE_HANDLER_ARGS);
		bool evaluatePredicateNode(NODE_HANDLER_ARGS);

		#undef NODE_HANDLER_ARGS
	};
}