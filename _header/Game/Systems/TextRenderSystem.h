#pragma once

#include "systemInclude.h"
#include "Window/GraphicsWrapper.h"
#include "SpriteStorage.h"

namespace process::game::systems {

	class TextRenderSystem {
	private:
		//typedefs
		using EntityID = wasp::ecs::entity::EntityID;

		//fields
		graphics::SymbolMap<wchar_t> symbolMap;
		window::GraphicsWrapper* graphicsWrapperPointer{};

	public:
		TextRenderSystem(
			window::GraphicsWrapper* graphicsWrapperPointer,
			resources::SpriteStorage& spriteStorage
		)
			: graphicsWrapperPointer{ graphicsWrapperPointer }
			, symbolMap{ loadSymbolMap(spriteStorage) } {
		}

		//beginDraw and endDraw are called in the RenderScheduler
		void operator()(Scene& scene);

	private:
		//helper functions
		void drawText(
			const Position& position,
			const TextInstruction& textInstruction
		);
		
		static graphics::SymbolMap<wchar_t> loadSymbolMap(
			resources::SpriteStorage& spriteStorage
		);
	};
}