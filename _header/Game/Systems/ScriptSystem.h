#pragma once

#include "systemInclude.h"

#include "Game/Systems/ComponentOrderQueue.h"
#include "Interpreter.h"
#include "ScriptStorage.h"
#include "SpriteStorage.h"

namespace process::game::systems {

	class ScriptSystem : private darkness::Interpreter<
		wasp::math::Vector2,
		wasp::math::PolarVector
	>{
	private:
		//typedefs
		using EntityID = wasp::ecs::entity::EntityID;
		using EntityHandle = wasp::ecs::entity::EntityHandle;
		using ScriptContainer = ScriptList::value_type;
		using Point2 = wasp::math::Point2;
		using Vector2 = wasp::math::Vector2;
		using PolarVector = wasp::math::PolarVector;

		//fields
		wasp::channel::ChannelSet* globalChannelSetPointer{};
		resources::ScriptStorage* scriptStoragePointer{};
		resources::SpriteStorage* spriteStoragePointer{};
		Scene* currentScenePointer{};
		EntityID currentEntityID{};
		ScriptContainer* currentScriptContainerPointer{};
		ScriptList scriptsToAddToCurrentEntity{};
		bool clearSpawnsFlag{ false };
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
		
		static void throwIfNativeFunctionWrongArity(
			std::size_t expectedArity,
			const std::vector<DataType>& parameters,
			const std::string& funcName
		);
		
		static void throwIfNativeFunctionArityOutOfRange(
			std::size_t arityMinInclusive,
			std::size_t arityMaxInclusive,
			const std::vector<DataType>& parameters,
			const std::string& funcName
		);
		
		void runScriptList(ScriptList& scriptList);
		void clearSpawns(ScriptList& scriptList);
		
		//native functions
		static DataType nativeUnaryMinus(const std::vector<DataType>& parameters);
		
		static DataType print(const std::vector<DataType>& parameters);
		DataType timer(const std::vector<DataType>& parameters);
		DataType stall(const std::vector<DataType>& parameters);
		static DataType stallUntil(const std::vector<DataType>& parameters);
		static DataType throwError(const std::vector<DataType>& parameters);
		DataType setVisible(const std::vector<DataType>& parameters);
		DataType setSpriteInstruction(const std::vector<DataType>& parameters);
		DataType setDepth(const std::vector<DataType>& parameters);
		DataType isSpawning(const std::vector<DataType>& parameters);
		DataType isBossDead(const std::vector<DataType>& parameters);
		DataType isDialogueOver(const std::vector<DataType>& parameters);
		DataType isWin(const std::vector<DataType>& parameters);
		DataType setCollidable(const std::vector<DataType>& parameters);
		DataType setHealth(const std::vector<DataType>& parameters);
		DataType setDamage(const std::vector<DataType>& parameters);
		DataType addSpawn(const std::vector<DataType>& parameters);
		DataType flagClearSpawns(const std::vector<DataType>& parameters);
		DataType addScript(const std::vector<DataType>& parameters);
		DataType setVelocity(const std::vector<DataType>& parameters);
		static DataType makeVector(const std::vector<DataType>& parameters);
		static DataType makePolar(const std::vector<DataType>& parameters);
		DataType angleToPlayer(const std::vector<DataType>& parameters);
		DataType random(const std::vector<DataType>& parameters);
		
		template <typename T>
		DataType removeComponent(
			const std::string& funcName,
			const std::vector<DataType>& parameters
		){
			throwIfNativeFunctionWrongArity(0, parameters, funcName);
			EntityHandle entityHandle{ makeCurrentEntityHandle() };
			componentOrderQueue.queueRemoveComponent<T>(entityHandle);
			return false;
		}
		
		template <bool trueIfX, bool trueIfAbove>
		DataType checkCoordinate(const std::vector<DataType>& parameters){
			throwIfNativeFunctionWrongArity(
				1,
				parameters,
				"checkCoordinate (" + std::to_string(trueIfX) + ", "
					+ std::to_string(trueIfAbove) + ")"
			);
			EntityHandle entityHandle{ makeCurrentEntityHandle() };
			const auto& dataStorage{ currentScenePointer->getDataStorage() };
			if (dataStorage.containsComponent<Position>(entityHandle)) {
				auto& position{ dataStorage.getComponent<Position>(entityHandle) };
				
				float boundary{ std::get<float>(parameters[0]) };
				if constexpr(trueIfX){
					if constexpr(trueIfAbove){
						return position.x > boundary;
					}
					else{
						return position.x < boundary;
					}
				}
				else{
					if constexpr(trueIfAbove){
						return position.y > boundary;
					}
					else{
						return position.y < boundary;
					}
				}
			}
			else{
				throw std::runtime_error{ "native func checkCoordinate no position!" };
			}
		}
	};
}