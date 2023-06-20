#include "Game/Systems/RenderSystem.h"

#include "Logging.h"

namespace process::game::systems {
	
	void RenderSystem::operator()(Scene& scene) {
		static const Topic<Group*> groupPointerStorageTopic{};

		auto groupPointer{
			getGroupPointer<Position, VisibleMarker, SpriteInstruction>(
				scene, 
				groupPointerStorageTopic
			)
		};
		
		//draw all sprites
		auto& dataStorage{ scene.getDataStorage() };
		
		auto groupIterator{
			groupPointer->groupIterator<Position, SpriteInstruction>()
		};
		while (groupIterator.isValid()) {
			const auto [position, spriteInstruction] = *groupIterator;
			auto entityID = groupIterator.getEntityID();
			if (dataStorage.containsComponent<SubImage>(entityID)) {
				const auto& subImage{
					dataStorage.getComponent<SubImage>(entityID)
				};
				drawSprite(position, spriteInstruction, subImage);
			}
			else if(dataStorage.containsComponent<TilingInstruction>(entityID)){
				const auto& tilingInstruction{
					dataStorage.getComponent<TilingInstruction>(entityID)
				};
				drawSprite(tilingInstruction, spriteInstruction);
			}
			else {
				drawSprite(position, spriteInstruction);
			}
			++groupIterator;
		}
	}

	//helper functions
	
	void RenderSystem::drawSprite(
		const Position& position,
		const SpriteInstruction& spriteInstruction
	) {
		graphicsWrapperPointer->drawSprite(
			position,
			spriteInstruction
		);
	}

	void RenderSystem::drawSprite(
		const Position& position,
		const SpriteInstruction& spriteInstruction,
		const SubImage& subImage
	) {
		graphicsWrapperPointer->drawSubSprite(
			position,
			spriteInstruction,
			subImage
		);
	}
	
	void RenderSystem::drawSprite(
		const TilingInstruction& tilingInstruction,
		const SpriteInstruction& spriteInstruction
	) {
		graphicsWrapperPointer->drawTileSprite(
			tilingInstruction.drawRectangle,
			spriteInstruction,
			tilingInstruction.pixelOffset
		);
	}
}