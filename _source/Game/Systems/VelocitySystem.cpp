#include "Game/Systems/VelocitySystem.h"

namespace process::game::systems {

	void VelocitySystem::operator()(Scene& scene) {
		static const Topic<ecs::component::Group*> groupPointerStorageTopic{};

		auto groupPointer{
			getGroupPointer<Position, Velocity>(
				scene, groupPointerStorageTopic
			)
		};

		//add each entity's velocity to it's position and step the position
		auto groupIterator{ groupPointer->groupIterator<Position, Velocity>() };
		while (groupIterator.isValid()) {
			const auto [position, velocity] = *groupIterator;
			position += velocity;
			position.step();
			++groupIterator;
		}
	}
}