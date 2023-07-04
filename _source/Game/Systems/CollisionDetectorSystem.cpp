#include "Game/Systems/CollisionDetectorSystem.h"

#include "Game/Systems/QuadTree.h"

#include "Logging.h"

namespace process::game::systems {

	namespace {
		using Group = wasp::ecs::component::Group;
		using EntityID = wasp::ecs::entity::EntityID;
		using EntityHandle = wasp::ecs::entity::EntityHandle;
		
		template <typename CollisionType>
		void detectCollisions(Scene& scene) {

			//this system is responsible for clearing the collision channel
			auto& collisionChannel{ 
				scene.getChannel(CollisionType::collisionTopic) 
			};
			collisionChannel.clear();

			auto& dataStorage{ scene.getDataStorage() };

			//get the group iterator for Position, Hitbox, CollidableMarker, and our
			//source type
			static const Topic<Group*> sourceGroupPointerStorageTopic{};
			auto sourceGroupPointer{
				getGroupPointer<
					Position,
					Hitbox,
					CollidableMarker,
					typename CollisionType::Source
				>(
					scene,
					sourceGroupPointerStorageTopic
				)
			};
			auto sourceGroupIterator{ 
				sourceGroupPointer->groupIterator<Position, Hitbox>() 
			};

			//insert all source entities into a quadTree
			QuadTree<EntityID> quadTree{ config::collisionBounds };
			while (sourceGroupIterator.isValid()) {
				const auto [position, hitbox] = *sourceGroupIterator;
				EntityID id{ sourceGroupIterator.getEntityID() };
				quadTree.insert(id, hitbox, position);
				++sourceGroupIterator;
			}

			//if our quadTree is empty, bail
			if (quadTree.isEmpty()) {
				return;
			}

			//get the group iterator for Position, Hitbox, CollidableMarker, and our
			//target type
			static const Topic<Group*> targetGroupPointerStorageTopic{};
			auto targetGroupPointer{
				getGroupPointer<
					Position,
					Hitbox,
					CollidableMarker,
					typename CollisionType::Target
				>(
					scene,
					targetGroupPointerStorageTopic
				)
			};
			auto targetGroupIterator{
				targetGroupPointer->groupIterator<Position, Hitbox>()
			};

			//check every target entity against our quadTree
			while (targetGroupIterator.isValid()) {
				const auto [position, hitbox] = *targetGroupIterator;
				const auto& collidedEntities{ 
					quadTree.checkCollisions(hitbox, position) 
				};

				if (!collidedEntities.empty()) {
					EntityID targetID{ targetGroupIterator.getEntityID() };
					for (EntityID sourceID : collidedEntities) {
						if (sourceID != targetID) {
							EntityHandle sourceHandle{
								dataStorage.makeHandle(sourceID)
							};
							EntityHandle targetHandle{
								dataStorage.makeHandle(targetID)
							};
							collisionChannel.addMessage({ sourceHandle, targetHandle });
						}
					}
				}
				++targetGroupIterator;
			}
		}
	}

	void CollisionDetectorSystem::operator()(Scene& scene) {
		detectCollisions<PlayerCollisions>(scene);
		detectCollisions<EnemyCollisions>(scene);
		detectCollisions<BulletCollisions>(scene);
		detectCollisions<PickupCollisions>(scene);
	}
}