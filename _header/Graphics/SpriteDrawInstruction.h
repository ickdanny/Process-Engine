#pragma once

#pragma warning(suppress : 4068) //suppress unknown pragma
#pragma clang diagnostic push
#pragma warning(suppress : 4068) //suppress unknown pragma
#pragma clang diagnostic ignored "-Wshadow"

#include "windowsInclude.h"
#include "d3dInclude.h"
#include <stdexcept>
#include <utility>

#include "Math/Angle.h"
#include "Math/Vector2.h"
#include "Graphics/Sprite.h"

namespace process::graphics {
	class SpriteDrawInstruction {
	private:
		//typedefs
		using Angle = wasp::math::Angle;
		using Vector2 = wasp::math::Vector2;
		
		//constants
	public:
		static constexpr int minDepth{ -10000 };
		static constexpr int maxDepth{ 10000 };
		static constexpr float minScale{ 0.01f };
		static constexpr float defaultRotation{ 0.0f };
		static constexpr float defaultScale{ 1.0f };
		
	private:
		//fields
		Sprite sprite{};
		int depth{};
		Vector2 offset{};
		Angle rotation{ defaultRotation };
		float scale{ defaultScale };

	public:
		//constructor
		explicit SpriteDrawInstruction(
			const Sprite& sprite,
			int depth,
			const Vector2& offset = {},
			Angle rotation = { defaultRotation },
			float scale = defaultScale
		)
			: sprite{ sprite }
			, depth{ depth }
			, offset{ offset }
			, rotation{ rotation }
			, scale{ scale }
		{
			throwIfDepthOutOfRange();
			throwIfScaleOutOfRange();
		}

		//getters
		const Sprite& getSprite() const {
			return sprite;
		}
		int getDepth() const {
			return depth;
		}
		const Vector2& getOffset() const {
			return offset;
		}
		Angle getRotation() const {
			//return by value
			return rotation;
		}
		float getScale() const {
			return scale;
		}

		//setters
		void setSprite(const Sprite& sprite) {
			this->sprite = sprite;
		}
		void setDepth(int depth){
			this->depth = depth;
		}
		void setOffset(const Vector2& offset) {
			this->offset = offset;
		}
		void setRotation(Angle rotation) {
			this->rotation = rotation;
		}
		void setScale(float scale) {
			this->scale = scale;
			throwIfScaleOutOfRange();
		}

	private:
		void throwIfDepthOutOfRange() const {
			if(depth < minDepth || depth > maxDepth){
				throw std::out_of_range("Error depth out of range");
			}
		}
		void throwIfScaleOutOfRange() const {
			if (scale <= minScale) {
				throw std::out_of_range("Error scale out of range");
			}
		}
	};
}

#pragma warning(suppress : 4068) //suppress unknown pragma
#pragma clang diagnostic pop