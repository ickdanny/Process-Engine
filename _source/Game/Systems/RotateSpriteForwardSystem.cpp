#include "Game/Systems/RotateSpriteForwardSystem.h"

#include "Logging.h"

namespace process::game::systems {


	void RotateSpriteForwardSystem::operator()(Scene& scene) {

		//get the group iterator for SpriteInstruction, Vel, RotateSpriteForwardMarker
		static const Topic<wasp::ecs::component::Group*> groupPointerStorageTopic{};
		auto groupPointer{
			getGroupPointer<SpriteInstruction, Velocity, RotateSpriteForwardMarker>(
				scene,
				groupPointerStorageTopic
			)
		};
		auto groupIterator{
			groupPointer->groupIterator<SpriteInstruction, Velocity>()
		};

		//rotate all sprites to the velocity direction
		while (groupIterator.isValid()) {
			auto [spriteInstruction, velocity] = *groupIterator;

			wasp::math::Angle angle{ -static_cast<float>(velocity.getAngle()) + 90.0f };
			if (spriteInstruction.getRotation() != angle) {
				spriteInstruction.setRotation(angle);
			}
			
			++groupIterator;
		}
	}
}