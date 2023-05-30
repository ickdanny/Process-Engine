#include "Game/Systems/TextRenderSystem.h"

namespace process::game::systems {

	//beginDraw and endDraw are called in the RenderScheduler
	void TextRenderSystem::operator()(Scene& scene) {

		static const Topic<wasp::ecs::component::Group*> groupPointerStorageTopic{};

		auto groupPointer{
			getGroupPointer<Position, VisibleMarker, TextInstruction>(
				scene,
				groupPointerStorageTopic
			)
		};

		auto groupIterator{ groupPointer->groupIterator<Position, TextInstruction>() };
		while (groupIterator.isValid()) {
			const auto& [position, textInstruction] = *groupIterator;
			drawText(position, textInstruction);
			++groupIterator;
		}
	}

	//helper functions
	void TextRenderSystem::drawText(
		const Position& position,
		const TextInstruction& textInstruction
	) {
		//todo: text rendering to graphics wrapper
		/*
		graphicsWrapperPointer->drawText(
			position, 
			textInstruction.text, 
			textInstruction.bounds
		);
		 */
	}
}