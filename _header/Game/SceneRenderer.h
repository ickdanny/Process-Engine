#pragma once

#include "Game/Scenes.h"
#include "Game/Systems/RenderSystem.h"
#include "Game/Systems/TextRenderSystem.h"
#include "Game/Systems/DebugRenderSystem.h"

namespace process::game {

	class SceneRenderer {
	private:
		//fields
		systems::RenderSystem renderSystem;				//not initialized!
		systems::TextRenderSystem textRenderSystem;		//not initialized!

		//debug fields
		#ifdef _DEBUG
		systems::DebugRenderSystem debugRenderSystem;	//not initialized!
		#endif

	public:

		SceneRenderer(window::GraphicsWrapper* graphicsWrapperPointer);

		void operator()(Scene& scene);
	};
}