#pragma once

#include "Graphics\SpriteDrawInstruction.h"
#include "Math\Point2.h"
#include "Math\Rectangle.h"

namespace process::graphics {
	class ISpriteDrawer {
	private:
		//typedefs
		using Point2 = wasp::math::Point2;
		using Rectangle = wasp::math::Rectangle;
	public:
		virtual void drawSprite(
			const Point2 center,
			const SpriteDrawInstruction& spriteDrawInstruction
		) = 0;

		virtual void drawSubSprite(
			const Point2 center,
			const SpriteDrawInstruction& spriteDrawInstruction,
			const Rectangle& sourceRectangle
		) = 0;
	};
}