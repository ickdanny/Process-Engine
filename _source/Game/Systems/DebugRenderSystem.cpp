#include "Game/Systems/DebugRenderSystem.h"

#ifdef _DEBUG
#include <iostream>
#include <chrono>
#endif

namespace process::game::systems {
	
	void DebugRenderSystem::operator()(Scene& scene) {
		#ifdef _DEBUG
		
		static constexpr float smoothing{ 0.9f };

		static int frameCount{ 0 };

		static std::chrono::steady_clock::time_point lastDraw{};
		static std::chrono::steady_clock::time_point thisDraw{};

		static float timeToDraw{ 1.0 };
		static float fps{ 1.0 };

		/*
		graphicsWrapperPointer->drawText(
			{ 5.0f, 5.0f },
			{ L"frame#: " + std::to_wstring(frameCount++)},
			{ 300.0f, 500.0f }
		);
		graphicsWrapperPointer->drawText(
			{ 5.0f, 15.0f },
			{ L"FPS:    " + std::to_wstring(static_cast<int>(1.0f / fps))},
			{ 400.0f, 300.0f }
		);
		 */

		lastDraw = thisDraw;
		thisDraw = std::chrono::steady_clock::now();

		auto duration{ thisDraw - lastDraw };
		timeToDraw = duration.count()
			* static_cast<float>(std::chrono::steady_clock::period::num)
			/ std::chrono::steady_clock::period::den;

		fps = (fps * smoothing) + (timeToDraw * (1.0f - smoothing));

		#endif
	}
}