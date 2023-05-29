#include "Game/SceneRenderer.h"

namespace process::game {

	SceneRenderer::SceneRenderer(window::WindowPainter* windowPainterPointer)
		: renderSystem{ windowPainterPointer }
		, textRenderSystem{ windowPainterPointer }

		#ifdef _DEBUG
		, debugRenderSystem{ windowPainterPointer }
		#endif
	{
	}

	void SceneRenderer::operator()(Scene& scene, float deltaTime) {
		renderSystem(scene, deltaTime);
		textRenderSystem(scene);

		#ifdef _DEBUG
		debugRenderSystem(scene, deltaTime);
		#endif
	}
}