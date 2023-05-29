#include "Game/Systems/SpriteSpinSystem.h"

namespace process::game::systems {

	void SpriteSpinSystem::operator()(Scene& scene) {

		//get the group iterator for SpriteInstruction, SpriteSpin
		static const Topic<ecs::component::Group*> groupPointerStorageTopic{};
		auto groupPointer{
			getGroupPointer<SpriteInstruction, SpriteSpin>(
				scene,
				groupPointerStorageTopic
			)
		};
		auto groupIterator{ 
			groupPointer->groupIterator<SpriteInstruction, SpriteSpin>() 
		};

		//spin sprites
		while (groupIterator.isValid()) {
			auto [spriteInstruction, spriteSpin] = *groupIterator;
			spriteInstruction.setRotation(
				static_cast<float>(spriteInstruction.getRotation()) + spriteSpin.spin
			);
			++groupIterator;
		}
	}
}