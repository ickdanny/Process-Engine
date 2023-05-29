#pragma once

#include "systemInclude.h"

#include "Game/Systems/ComponentOrderQueue.h"

namespace process::game::systems {

	class ScriptSystem {
	private:
		//typedefs
		using ScriptInstructions = components::ScriptInstructions;
		using ScriptNode = components::ScriptNode;
		using ScriptProgram = components::ScriptProgram;
		template <typename Internal, typename External>
		using ScriptNodeData = components::ScriptNodeData<Internal, External>;
		
		using EntityID = ecs::entity::EntityID;

		//fields
		channel::ChannelSet* globalChannelSetPointer{};

	public:
		ScriptSystem(channel::ChannelSet* globalChannelSetPointer);
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