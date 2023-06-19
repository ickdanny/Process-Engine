#include "Game/Systems/CollisionHandlerSystem.h"

#include "Logging.h"

namespace process::game::systems {

	
	CollisionHandlerSystem::CollisionHandlerSystem(
		resources::ScriptStorage* scriptStoragePointer
	)
		: scriptStoragePointer{ scriptStoragePointer } {
	}
	

	void CollisionHandlerSystem::operator()(Scene& scene) {
		//this system is responsible for clearing the playerHits channel
		scene.getChannel(SceneTopics::playerHits).clear();

		handleCollisions<PlayerCollisions>(scene);
		handleCollisions<EnemyCollisions>(scene);
		handleCollisions<BulletCollisions>(scene);
		handleCollisions<PickupCollisions>(scene);
	}

	void CollisionHandlerSystem::handleCollisionCommand(
		Scene& scene,
		const EntityHandle& entityHandle,
		const CollisionCommands command,
		const EntityHandle& collidedHandle
	) {
		switch (command) {
			case CollisionCommands::death:
				handleDeathCommand(scene, entityHandle);
				break;
			case CollisionCommands::damage:
				handleDamageCommand(scene, entityHandle, collidedHandle);
				break;
			case CollisionCommands::player:
				handlePlayerCommand(scene, entityHandle);
				break;
			case CollisionCommands::pickup:
				handlePickupCommand(scene, entityHandle, collidedHandle);
				break;
			case CollisionCommands::none:
				//do nothing
				break;
			default:
				throw std::runtime_error{
					"default case reached in collision handler system"
				};
		}
	}

	void CollisionHandlerSystem::handlePickupCommand(
		Scene& scene,
		const EntityHandle& pickupHandle,
		const EntityHandle& collidedHandle
	) {
		auto& dataStorage{ scene.getDataStorage() };

		auto& playerData{ dataStorage.getComponent<PlayerData>(collidedHandle) };
		auto& pickupType{ dataStorage.getComponent<PickupType>(pickupHandle).type };
		switch (pickupType) {
			case PickupType::Types::life:
				if (playerData.lives < config::maxLives) {
					++playerData.lives;
				}
				break;
			case PickupType::Types::bomb:
				if (playerData.bombs < config::maxBombs) {
					++playerData.bombs;
				}
				break;
			case PickupType::Types::powerSmall:
			case PickupType::Types::powerLarge:
				if (playerData.power < config::maxPower) {
					int powerGain{
						pickupType == PickupType::Types::powerSmall
							? config::smallPowerGain
							: config::largePowerGain
					};
					playerData.power += powerGain;
					if (playerData.power >= config::maxPower) {
						playerData.power = config::maxPower;

						//max power clear by adding death spawn to the pickup
						//ideally should refactor into some sort of message but whatever
						
						//todo: collision death spawn
						/*
						dataStorage.addComponent<DeathCommand>({
							pickupHandle,
							{ DeathCommand::Commands::deathSpawn }
						});
						dataStorage.addComponent<DeathSpawn>({
							pickupHandle,
							{ SpawnProgramList{ 
								programsPointer->pickupPrograms.maxPowerClearSpawnProgram
							}}
						});
						 */
					}
				}
		}

		//kill the pickup
		handleDeathCommand(scene, pickupHandle);
	}

	void CollisionHandlerSystem::handlePlayerCommand(
		Scene& scene,
		const EntityHandle& playerHandle
	) {
		scene.getChannel(SceneTopics::playerHits).addMessage(playerHandle);
	}

	void CollisionHandlerSystem::handleDamageCommand(
		Scene& scene,
		const EntityHandle& entityHandle,
		const EntityHandle& collidedHandle
	) {
		//check to see if this entity has health and the other entity has damage
		auto& dataStorage{ scene.getDataStorage() };
		if (dataStorage.containsComponent<Health>(entityHandle)
			&& dataStorage.containsComponent<Damage>(collidedHandle))
		{
			//if so, subtract damage from health
			auto& health{ dataStorage.getComponent<Health>(entityHandle) };
			if (health.value > 0) {
				const auto& damage{
					dataStorage.getComponent<Damage>(collidedHandle)
				};
				health.value -= damage.value;
				//if health <= 0, treat it as an entity death
				if (health.value <= 0) {
					handleDeathCommand(scene, entityHandle);
				}
			}
		}
	}

	//publish a death message
	void CollisionHandlerSystem::handleDeathCommand(
		Scene& scene,
		const EntityHandle& entityHandle
	) {
		scene.getChannel(SceneTopics::deaths).addMessage(entityHandle);
	}
}