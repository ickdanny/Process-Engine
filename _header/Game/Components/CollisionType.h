#pragma once

#include "ECS/Entity/EntityHandle.h"

namespace process::game::components {

	enum class CollisionCommands {
		none,
		death,
		damage,
		removeCollisionType,
		player,
		pickup
	};

	//When creating a collision type, pass the derived type as the template parameter
	//such that the compiler will instantiate a separate Source and Target type for
	//each collision type.
	template <typename Derived>
	struct CollisionType {
	private:
		//typedefs
		using EntityHandle = wasp::ecs::entity::EntityHandle;

	public:
		//set and cleared by CollisionDetectorSystem
		//format = sourceEntity, targetEntity
		static const Topic<std::tuple<EntityHandle, EntityHandle>> collisionTopic;

		//delete constructor
		CollisionType() = delete;

		struct Source {
			CollisionCommands command;
		};
		struct Target {
			CollisionCommands command;
		};
	};

	template <typename Derived>
	const Topic<std::tuple<wasp::ecs::entity::EntityHandle, wasp::ecs::entity::EntityHandle>>
		CollisionType<Derived>::collisionTopic{};
}