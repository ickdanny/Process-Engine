#pragma once

#include "systemInclude.h"
#include "ScriptStorage.h"

namespace process::game::systems {

	class CollisionHandlerSystem {
	private:
		//typedefs
		using EntityHandle = wasp::ecs::entity::EntityHandle;
		using CollisionCommands = components::CollisionCommands;
		template <typename T>
		using RemoveComponentOrder = wasp::ecs::RemoveComponentOrder<T>;

		//fields
		resources::ScriptStorage* scriptStoragePointer{};

	public:
		CollisionHandlerSystem(resources::ScriptStorage* scriptStoragePointer);
		void operator()(Scene& scene);

	private:
		//helper functions
		template <typename CollisionType>
		void handleCollisions(Scene& scene) {
			const auto& collisionChannel{
				scene.getChannel(CollisionType::collisionTopic)
			};

			if (collisionChannel.hasMessages()) {
				auto& dataStorage{ scene.getDataStorage() };

				//handle the commands for each source/target pair
				for (const auto& [sourceHandle, targetHandle]
					: collisionChannel.getMessages()
				) {
					if(dataStorage.containsComponent<typename CollisionType::Source>(
						sourceHandle
					)) {
						const auto sourceCommand {
							dataStorage.getComponent<typename CollisionType::Source>(
								sourceHandle
							).command
						};
						handleCollisionCommand<CollisionType>(
							scene,
							sourceHandle,
							sourceCommand,
							targetHandle
						);
					}
					if(dataStorage.containsComponent<typename CollisionType::Target>(
						targetHandle
					)) {
						const auto targetCommand {
							dataStorage.getComponent<typename CollisionType::Target>(
								targetHandle
							).command
						};
						handleCollisionCommand<CollisionType>(
							scene,
							targetHandle,
							targetCommand,
							sourceHandle
						);
					}
				}
			}
		}

		template <typename CollisionType>
		void handleCollisionCommand(
			Scene& scene,
			const EntityHandle& entityHandle,
			const components::CollisionCommands command,
			const EntityHandle& collidedHandle
		){
			switch (command) {
				case CollisionCommands::death:
					handleDeathCommand(scene, entityHandle);
					break;
				case CollisionCommands::damage:
					handleDamageCommand(scene, entityHandle, collidedHandle);
					break;
				case CollisionCommands::removeCollisionType:
					handleRemoveCollisionTypeCommand<CollisionType>(scene, entityHandle);
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

		void handlePickupCommand(
			Scene& scene,
			const EntityHandle& pickupHandle,
			const EntityHandle& collidedHandle
		);

		void handlePlayerCommand(Scene& scene, const EntityHandle& playerHandle);

		void handleDamageCommand(
			Scene& scene,
			const EntityHandle& entityHandle,
			const EntityHandle& collidedHandle
		);

		void handleDeathCommand(
			Scene& scene,
			const EntityHandle& entityHandle
		);
		
		template <typename CollisionType>
		void handleRemoveCollisionTypeCommand(
			Scene& scene,
			const EntityHandle& entityHandle
		){
			//since this system does not iterate over entities, just remove
			auto& dataStorage{ scene.getDataStorage() };
			if(!dataStorage.removeComponent(
				RemoveComponentOrder<typename CollisionType::Source>{ entityHandle }
			)){
				if(!dataStorage.removeComponent(
					RemoveComponentOrder<typename CollisionType::Target>{ entityHandle }
				)){
					throw std::runtime_error{ "failed to remove collision type" };
				}
			}
			
		}
	};
}