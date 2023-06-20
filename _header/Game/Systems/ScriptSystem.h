#pragma once

#include "systemInclude.h"

#include "Game/Systems/ComponentOrderQueue.h"
#include "Interpreter.h"
#include "ScriptStorage.h"

namespace process::game::systems {

	class ScriptSystem : private darkness::Interpreter<>{
	private:
		//typedefs
		using EntityID = wasp::ecs::entity::EntityID;
		using ScriptContainer = components::ScriptContainer;

		//fields
		wasp::channel::ChannelSet* globalChannelSetPointer{};
		resources::ScriptStorage* scriptStoragePointer{};
		Scene* currentScenePointer{};
		EntityID currentEntityID{};
		ScriptContainer* currentScriptContainerPointer{};
		ComponentOrderQueue componentOrderQueue{};//cleared at end of every call

	public:
		ScriptSystem(
			wasp::channel::ChannelSet* globalChannelSetPointer,
			resources::ScriptStorage* scriptStoragePointer
		);
		void operator()(Scene& scene);

	private:
		void runScriptList(ScriptList& scriptList);
		
		//native functions
		static DataType print(const std::vector<DataType>& parameters);
		DataType timer(const std::vector<DataType>& parameters);
		DataType stall(const std::vector<DataType>& parameters);
		static DataType stallUntil(const std::vector<DataType>& parameters);
	};
}