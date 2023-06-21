#pragma once

#include "systemInclude.h"

#include "Game/Systems/ComponentOrderQueue.h"
#include "Interpreter.h"
#include "ScriptStorage.h"
#include "SpriteStorage.h"

namespace process::game::systems {

	class ScriptSystem : private darkness::Interpreter<wasp::math::Vector2>{
	private:
		//typedefs
		using EntityID = wasp::ecs::entity::EntityID;
		using EntityHandle = wasp::ecs::entity::EntityHandle;
		using ScriptContainer = components::ScriptContainer;

		//fields
		wasp::channel::ChannelSet* globalChannelSetPointer{};
		resources::ScriptStorage* scriptStoragePointer{};
		resources::SpriteStorage* spriteStoragePointer{};
		Scene* currentScenePointer{};
		EntityID currentEntityID{};
		ScriptContainer* currentScriptContainerPointer{};
		ComponentOrderQueue componentOrderQueue{};//cleared at end of every call

	public:
		ScriptSystem(
			wasp::channel::ChannelSet* globalChannelSetPointer,
			resources::ScriptStorage* scriptStoragePointer,
			resources::SpriteStorage* spriteStoragePointer
		);
		void operator()(Scene& scene);

	private:
		//helper functions
		EntityHandle makeCurrentEntityHandle();
		
		template <typename T>
		bool containsComponent(const EntityHandle& entityHandle){
			return currentScenePointer->getDataStorage().containsComponent<T>(entityHandle);
		}
		
		template <typename T>
		T& getComponent(const EntityHandle& entityHandle){
			return currentScenePointer->getDataStorage().getComponent<T>(entityHandle);
		}
		
		void runScriptList(ScriptList& scriptList);
		
		//native functions
		static DataType print(const std::vector<DataType>& parameters);
		DataType timer(const std::vector<DataType>& parameters);
		DataType stall(const std::vector<DataType>& parameters);
		static DataType stallUntil(const std::vector<DataType>& parameters);
		static DataType throwError(const std::vector<DataType>& parameters);
		DataType removeVisible(const std::vector<DataType>& parameters);
		DataType setSpriteInstruction(const std::vector<DataType>& parameters);
		DataType setDepth(const std::vector<DataType>& parameters);
		
		
	};
}