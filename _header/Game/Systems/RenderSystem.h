#pragma once

#include "systemInclude.h"
#include "Window/GraphicsWrapper.h"

namespace process::game::systems {

	class RenderSystem {
	private:
		//typedefs
		using EntityID = wasp::ecs::entity::EntityID;
		using Group = wasp::ecs::component::Group;

		//fields
		window::GraphicsWrapper* graphicsWrapperPointer{};

	public:
		RenderSystem(window::GraphicsWrapper* graphicsWrapperPointer)
			: graphicsWrapperPointer{ graphicsWrapperPointer } {
		}

		void operator()(Scene& scene);

	private:
		//helper functions
		void drawSprite(
			const Position& position,
			const SpriteInstruction& drawInstruction
		);

		void drawSprite(
			const Position& position,
			const SpriteInstruction& drawInstruction,
			const SubImage& subImage
		);
	};
}