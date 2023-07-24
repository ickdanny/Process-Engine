#pragma once

#include "systemInclude.h"

#include "Game/Systems/ComponentOrderQueue.h"
#include "SpawnQueue.h"
#include "Interpreter.h"
#include "ScriptStorage.h"
#include "SpriteStorage.h"
#include "Prototypes.h"

namespace process::game::systems {

	class ScriptSystem : private darkness::Interpreter<
		wasp::math::Point2,
		wasp::math::Vector2,
		wasp::math::PolarVector
	>{
	private:
		//typedefs
		using EntityID = wasp::ecs::entity::EntityID;
		using EntityHandle = wasp::ecs::entity::EntityHandle;
		using Group = wasp::ecs::component::Group;
		using ScriptContainer = ScriptList::value_type;
		using Point2 = wasp::math::Point2;
		using Vector2 = wasp::math::Vector2;
		using PolarVector = wasp::math::PolarVector;
		using Angle = wasp::math::Angle;

		//fields
		wasp::channel::ChannelSet* globalChannelSetPointer{};
		resources::ScriptStorage* scriptStoragePointer{};
		resources::SpriteStorage* spriteStoragePointer{};
		Prototypes prototypes;	//not initialized
		
		Scene* currentScenePointer{};
		EntityID currentEntityID{};
		ScriptContainer* currentScriptContainerPointer{};
		ScriptList scriptsToAddToCurrentEntity{};
		bool clearSpawnsFlag{ false };
		ComponentOrderQueue componentOrderQueue{};	//cleared at end of every call
		SpawnQueue spawnQueue{};	//cleared at end of every call

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
		static float getAsFloat(const DataType& data);
		
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
		
		//native operator handlers
		static DataType nativeUnaryMinus(const std::vector<DataType>& parameters);
		static DataType nativeBinaryPlus(const std::vector<DataType>& parameters);
		static DataType nativeBinaryMinus(const std::vector<DataType>& parameters);
		static DataType nativeBinaryStar(const std::vector<DataType>& parameters);
		
		//utility functions
		static DataType throwError(const std::vector<DataType>& parameters);
		static DataType print(const std::vector<DataType>& parameters);
		DataType timer(const std::vector<DataType>& parameters);
		DataType stall(const std::vector<DataType>& parameters);
		DataType stallUntil(const std::vector<DataType>& parameters);
		
		//general queries
		DataType isBossDead(const std::vector<DataType>& parameters);
		DataType isDialogueOver(const std::vector<DataType>& parameters);
		DataType isWin(const std::vector<DataType>& parameters);
		DataType getDifficulty(const std::vector<DataType>& parameters);
		DataType getPlayerPos(const std::vector<DataType>& parameters);
		
		//entity graphics
		DataType setVisible(const std::vector<DataType>& parameters);
		DataType setSpriteInstruction(const std::vector<DataType>& parameters);
		DataType setDepth(const std::vector<DataType>& parameters);
		DataType setRotation(const std::vector<DataType>& parameters);
		
		//entity queries
		DataType angleToPlayer(const std::vector<DataType>& parameters);
		DataType entityPosition(const std::vector<DataType>& parameters);
		DataType entityX(const std::vector<DataType>& parameters);
		DataType entityY(const std::vector<DataType>& parameters);
		DataType entityVelocity(const std::vector<DataType>& parameters);
		DataType entitySpeed(const std::vector<DataType>& parameters);
		DataType entityAngle(const std::vector<DataType>& parameters);
		DataType entitySpin(const std::vector<DataType>& parameters);
		DataType isSpawning(const std::vector<DataType>& parameters);
		DataType isNotSpawning(const std::vector<DataType>& parameters);
		DataType playerPower(const std::vector<DataType>& parameters);
		DataType isFocused(const std::vector<DataType>& parameters);
		DataType isNotSpecialCollisionTarget(const std::vector<DataType>& parameters);
		
		//entity mutators
		DataType setCollidable(const std::vector<DataType>& parameters);
		DataType setSpecialCollisionSource(const std::vector<DataType>& parameters);
		DataType setSpecialCollisionTarget(const std::vector<DataType>& parameters);
		DataType setHealth(const std::vector<DataType>& parameters);
		DataType setDamage(const std::vector<DataType>& parameters);
		DataType setClearMarker(const std::vector<DataType>& parameters);
		DataType setInbound(const std::vector<DataType>& parameters);
		DataType setOutbound(const std::vector<DataType>& parameters);
		DataType setPosition(const std::vector<DataType>& parameters);
		DataType setVelocity(const std::vector<DataType>& parameters);
		DataType setSpeed(const std::vector<DataType>& parameters);
		DataType setAngle(const std::vector<DataType>& parameters);
		DataType setSpin(const std::vector<DataType>& parameters);
		DataType die(const std::vector<DataType>& parameters);
		DataType removeEntity(const std::vector<DataType>& parameters);
		
		//multi-scripting
		DataType addSpawn(const std::vector<DataType>& parameters);
		DataType addDeathSpawn(const std::vector<DataType>& parameters);
		DataType flagClearSpawns(const std::vector<DataType>& parameters);
		DataType addScript(const std::vector<DataType>& parameters);
		
		//math
		static DataType makePoint(const std::vector<DataType>& parameters);
		static DataType makeVector(const std::vector<DataType>& parameters);
		static DataType makePolar(const std::vector<DataType>& parameters);
		static DataType toVector(const std::vector<DataType>& parameters);
		static DataType getX(const std::vector<DataType>& parameters);
		static DataType getY(const std::vector<DataType>& parameters);
		static DataType getR(const std::vector<DataType>& parameters);
		static DataType getTheta(const std::vector<DataType>& parameters);
		static DataType setX(const std::vector<DataType>& parameters);
		static DataType setY(const std::vector<DataType>& parameters);
		static DataType setR(const std::vector<DataType>& parameters);
		static DataType setTheta(const std::vector<DataType>& parameters);
		static DataType flipX(const std::vector<DataType>& parameters);
		static DataType flipY(const std::vector<DataType>& parameters);
		static DataType exponent(const std::vector<DataType>& parameters);
		static DataType sin(const std::vector<DataType>& parameters);
		static DataType cos(const std::vector<DataType>& parameters);
		static DataType tan(const std::vector<DataType>& parameters);
		static DataType sec(const std::vector<DataType>& parameters);
		static DataType csc(const std::vector<DataType>& parameters);
		static DataType cot(const std::vector<DataType>& parameters);
		static DataType arcsin(const std::vector<DataType>& parameters);
		static DataType arccos(const std::vector<DataType>& parameters);
		static DataType arctan(const std::vector<DataType>& parameters);
		static DataType min(const std::vector<DataType>& parameters);
		static DataType max(const std::vector<DataType>& parameters);
		static DataType smallerDifference(const std::vector<DataType>& parameters);
		static DataType largerDifference(const std::vector<DataType>& parameters);
		static DataType absoluteValue(const std::vector<DataType>& parameters);
		static DataType pointDistance(const std::vector<DataType>& parameters);
		static DataType pointAngle(const std::vector<DataType>& parameters);
		DataType random(const std::vector<DataType>& parameters);
		DataType chance(const std::vector<DataType>& parameters);
		
		//scene signaling
		DataType sendBossDeath(const std::vector<DataType>& parameters);
		DataType clearBullets(const std::vector<DataType>& parameters);
		DataType showDialogue(const std::vector<DataType>& parameters);
		DataType win(const std::vector<DataType>& parameters);
		DataType endStage(const std::vector<DataType>& parameters);
		DataType broadcast(const std::vector<DataType>& parameters);
		DataType readPoint(const std::vector<DataType>& parameters);
		DataType killMessage(const std::vector<DataType>& parameters);
		
		//spawning
		DataType spawn(const std::vector<DataType>& parameters);
		
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