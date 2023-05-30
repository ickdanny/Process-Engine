#pragma once

#include "systemInclude.h"
#include "Window/GraphicsWrapper.h"

namespace process::game::systems {

	class DebugRenderSystem {
	private:
		//fields
		window::GraphicsWrapper* graphicsWrapperPointer{};

	public:
		DebugRenderSystem(window::GraphicsWrapper* graphicsWrapperPointer)
			: graphicsWrapperPointer{ graphicsWrapperPointer } {
		}

		//beginDraw and endDraw are called in the RenderScheduler
		void operator()(Scene& scene);
	};
}