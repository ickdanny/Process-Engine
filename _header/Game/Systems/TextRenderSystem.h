#pragma once

#include "systemInclude.h"
#include "Window/WindowPainter.h"

namespace process::game::systems {

	class TextRenderSystem {
	private:
		//typedefs
		using EntityID = ecs::entity::EntityID;

		//fields
		window::WindowPainter* windowPainterPointer{};

	public:
		TextRenderSystem(window::WindowPainter* windowPainterPointer)
			: windowPainterPointer{ windowPainterPointer } {
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