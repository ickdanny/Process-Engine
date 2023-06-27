#include "Game/SceneRenderer.h"

namespace process::game {

	SceneRenderer::SceneRenderer(
		window::GraphicsWrapper* graphicsWrapperPointer,
		resources::SpriteStorage& spriteStorage
	)
		: renderSystem{ graphicsWrapperPointer }
		, textRenderSystem{ graphicsWrapperPointer, spriteStorage }

		#ifdef _DEBUG
		, debugRenderSystem{ graphicsWrapperPointer }
		#endif
	{
	}

	void SceneRenderer::operator()(Scene& scene) {
		renderSystem(scene);
		textRenderSystem(scene);

		#ifdef _DEBUG
		debugRenderSystem(scene);
		#endif
	}
}