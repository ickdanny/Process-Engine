#pragma once

#include "systemInclude.h"

#include "Game/Systems/ComponentOrderQueue.h"
#include "Interpreter.h"

namespace process::game::systems {

	class ScriptSystem : private darkness::Interpreter<>{
	private:
		//typedefs
		using EntityID = wasp::ecs::entity::EntityID;

		//fields
		wasp::channel::ChannelSet* globalChannelSetPointer{};
		Scene* currentScenePointer{};
		EntityID currentEntityID{};
		ComponentOrderQueue componentOrderQueue{};//cleared at end of every call

	public:
		explicit ScriptSystem(wasp::channel::ChannelSet* globalChannelSetPointer);
		void operator()(Scene& scene);

	private:
		void runScriptList(ScriptList& scriptList);
	};
}