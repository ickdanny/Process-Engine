#pragma once

#include "systemInclude.h"
#include "Window/GraphicsWrapper.h"

namespace process::game::systems {

	class TextRenderSystem {
	private:
		//typedefs
		using EntityID = wasp::ecs::entity::EntityID;

		//fields
		window::GraphicsWrapper* graphicsWrapperPointer{};

	public:
		TextRenderSystem(window::GraphicsWrapper* graphicsWrapperPointer)
			: graphicsWrapperPointer{ graphicsWrapperPointer } {
		}

		//beginDraw and endDraw are called in the RenderScheduler
		void operator()(Scene& scene);

	private:
		//helper functions
		void drawText(
			const Position& position,
			const TextInstruction& textInstruction
		);
	};
}