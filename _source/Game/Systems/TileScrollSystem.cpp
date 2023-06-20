#include "Game/Systems/TileScrollSystem.h"

#include "Logging.h"

namespace process::game::systems {

	namespace {
		void updateTileOffset(
			TilingInstruction& tilingInstruction,
			const SpriteInstruction& spriteInstruction,
			const TileScroll& tileScroll
		) {
			tilingInstruction.pixelOffset =
				tilingInstruction.pixelOffset + tileScroll.pixelScroll;
			
			float spriteWidth{ static_cast<float>(spriteInstruction.getSprite().width) };
			float spriteHeight{ static_cast<float>(spriteInstruction.getSprite().height) };
			
			if (tilingInstruction.pixelOffset.x < 0.0f) {
				tilingInstruction.pixelOffset.x += spriteWidth;
			}
			else if (tilingInstruction.pixelOffset.x >= spriteWidth) {
				tilingInstruction.pixelOffset.x -= spriteWidth;
			}
			if (tilingInstruction.pixelOffset.y < 0.0f) {
				tilingInstruction.pixelOffset.y += spriteHeight;
			}
			else if (tilingInstruction.pixelOffset.y >= spriteHeight) {
				tilingInstruction.pixelOffset.y -= spriteHeight;
			}
		}
	}

	void TileScrollSystem::operator()(Scene& scene) {
		static const Topic<wasp::ecs::component::Group*> groupPointerStorageTopic{};

		auto groupPointer{
			getGroupPointer<TilingInstruction, SpriteInstruction, TileScroll>(
				scene, groupPointerStorageTopic
			)
		};

		//step each offset by tile scroll
		auto groupIterator{ groupPointer->groupIterator<
		    TilingInstruction,
			SpriteInstruction,
			TileScroll
		>() };
		while (groupIterator.isValid()) {
			const auto& [tilingInstruction, spriteInstruction, tileScroll] = *groupIterator;
			updateTileOffset(tilingInstruction, spriteInstruction, tileScroll);
			++groupIterator;
		}
	}
}