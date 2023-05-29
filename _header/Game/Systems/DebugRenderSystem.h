#pragma once

#include "systemInclude.h"
#include "Window/WindowPainter.h"

namespace process::game::systems {

	class DebugRenderSystem {
	private:
		//fields
		window::WindowPainter* windowPainterPointer{};

	public:
		DebugRenderSystem(window::WindowPainter* windowPainterPointer)
			: windowPainterPointer{ windowPainterPointer } {
		}

		//beginDraw and endDraw are called in the RenderScheduler
		void operator()(Scene& scene, float deltaTime);
	};
}