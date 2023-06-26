#include "Game/Systems/DeathHandlerSystem.h"

#include "Game/Systems/EntityBuilder.h"

#include "Logging.h"

namespace process::game::systems {

	namespace {

		//utility for player death
		constexpr int lossRatio{ 4 };
		constexpr int minLoss{ 4 };
		constexpr int maxLoss{ 10 };

		int powerLoss(int initPower) {
			if (initPower <= minLoss) {
				return initPower;
			}

			int loss{ initPower / lossRatio };
			if (loss < minLoss) {
				loss = minLoss;
			}
			else if (loss > maxLoss) {
				loss = maxLoss;
			}
			return loss;
		}
	}

	DeathHandlerSystem::DeathHandlerSystem(resources::ScriptStorage& scriptStorage)
		: ghostScriptContainer{
			scriptStorage.get(L"ghostSpawner"),
			"ghostSpawner"
		}
	{
	}

	void DeathHandlerSystem::operator()(Scene& scene) {
		if (scene.hasChannel(SceneTopics::deaths)) {
			const auto& deathChannel{ scene.getChannel(SceneTopics::deaths) };
			auto& dataStorage{ scene.getDataStorage() };

			for (const auto& entityHandle : deathChannel.getMessages()) {
				if (dataStorage.isAlive(entityHandle)) {
					//if the entity has a special deathCommand, defer to that
					if (dataStorage.containsComponent<DeathCommand>(entityHandle)) {
						const DeathCommand deathCommand{
							dataStorage.getComponent<DeathCommand>(entityHandle)
						};
						handleDeath(scene, entityHandle, deathCommand.command);
					}
					//otherwise, remove the entity
					else {
						dataStorage.removeEntity({ entityHandle });
					}
				}
			}
		}
	}

	void DeathHandlerSystem::handleDeath(
		Scene& scene,
		const EntityHandle& entityHandle,
		const DeathCommand::Commands command
	) {
		switch (command) {
			case DeathCommand::Commands::playerDeath:
				handlePlayerDeath(scene, entityHandle);
				break;
			case DeathCommand::Commands::bossDeath:
				handleBossDeath(scene, entityHandle);
				handleDeathSpawn(scene, entityHandle, false);
				break;
			case DeathCommand::Commands::deathSpawn:
				handleDeathSpawn(scene, entityHandle, true);
				break;
			default:
				throw std::runtime_error{ "unexpected default case" };
		}
	}

	void DeathHandlerSystem::handlePlayerDeath(
		Scene& scene,
		const EntityHandle& playerHandle
	) {
		auto& dataStorage{ scene.getDataStorage() };

		//remove spawn component and pickup collision
		dataStorage.removeComponent<ScriptList>(playerHandle);
		dataStorage.removeComponent<PickupCollisions::Target>(playerHandle);

		//spawn blocker + pickups
		handleDeathSpawn(scene, playerHandle, false);

		//remove power
		PlayerData& playerData{
			dataStorage.getComponent<PlayerData>(playerHandle)
		};
		playerData.power -= powerLoss(playerData.power);
	}

	void DeathHandlerSystem::handleBossDeath(
		Scene& scene, 
		const EntityHandle& bossHandle
	) {
		scene.getChannel(SceneTopics::bossDeaths).addMessage(bossHandle);
	}

	void DeathHandlerSystem::handleDeathSpawn(
		Scene& scene,
		const EntityHandle& entityHandle,
		bool removeEntity
	) {
		auto& dataStorage{ scene.getDataStorage() };
		if (dataStorage.containsComponent<DeathSpawn>(entityHandle)) {
			DeathSpawn& deathSpawn{
				dataStorage.getComponent<DeathSpawn>(entityHandle)
			};
			auto scriptList{ std::move(deathSpawn.scriptList) };
			scriptList.push_back(ghostScriptContainer);
			//add a ghost with the death spawn program list and possibly position
			if (dataStorage.containsComponent<Position>(entityHandle)) {
				Position& position{
					dataStorage.getComponent<Position>(entityHandle)
				};
				auto ghostTuple{
					EntityBuilder::makeEntity(
						position, 
						scriptList
					)
				};
				dataStorage.addEntity(ghostTuple.package());
			}
			else {
				auto ghostTuple{ 
					EntityBuilder::makeEntity(scriptList)
				};
				dataStorage.addEntity(ghostTuple.package());
			}
		}
		if (removeEntity) {
			dataStorage.removeEntity(entityHandle);
		}
	}
}