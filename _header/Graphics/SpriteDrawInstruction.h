#pragma once

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
		template <typename T>
		using ComPtr = Microsoft::WRL::ComPtr<T>;

		//constants
		static constexpr float minOpacity{ 0.0f };
		static constexpr float maxOpacity{ 1.0f };
		static constexpr float minScale{ 0.01f };
		static constexpr float defaultRotation{ 0.0f };
		static constexpr float defaultOpacity{ 1.0f };
		static constexpr float defaultScale{ 1.0f };

		//fields
		Sprite sprite{};
		Vector2 offset{};
		Angle rotation{ defaultRotation };
		float opacity{ defaultOpacity };
		float scale{ defaultScale };

		bool updated{};         //useful for implementation

	public:
		//constructor
		explicit SpriteDrawInstruction(
			const Sprite& sprite,
			const Vector2& offset = {},
			Angle rotation = { defaultRotation },
			float opacity = defaultOpacity,
			float scale = defaultScale
		)
			: sprite{ sprite }
			, offset{ offset }
			, rotation{ rotation }
			, opacity{ opacity }
			, scale{ scale }
		{
			throwIfOutOfRange();
		}

		//getters
		const Sprite& getSprite() const {
			return sprite;
		}
		const Vector2& getOffset() const {
			return offset;
		}
		Angle getRotation() const {
			//return by value
			return rotation;
		}
		float getOpacity() const {
			return opacity;
		}
		float getScale() const {
			return scale;
		}

		//setters
		void setSprite(const Sprite& sprite) {
			this->sprite = sprite;
			flagForUpdate();
		}
		void setOffset(const Vector2& offset) {
			this->offset = offset;
			flagForUpdate();
		}
		void setRotation(Angle rotation) {
			this->rotation = rotation;
			flagForUpdate();
		}
		void setOpacity(float opacity) {
			this->opacity = opacity;
			throwIfOpacityOutOfRange();
			flagForUpdate();
		}
		void setScale(float scale) {
			this->scale = scale;
			throwIfScaleOutOfRange();
			flagForUpdate();
		}

		bool requiresRotation() const {
			return static_cast<float>(rotation) != defaultRotation;
		}

		bool requiresScale() const {
			return scale != defaultScale;
		}

		//updating
		bool isUpdated() const {
			return updated;
		}
		void flagForUpdate() {
			updated = false;
		}
		void update() {
			updated = true;
		}

	private:
		void throwIfOutOfRange() const {
			throwIfOpacityOutOfRange();
			throwIfScaleOutOfRange();
		}
		void throwIfOpacityOutOfRange() const {
			if (opacity < minOpacity || opacity > maxOpacity) {
				throw std::out_of_range("Error opacity out of range");
			}
		}
		void throwIfScaleOutOfRange() const {
			if (scale <= minScale) {
				throw std::out_of_range("Error scale out of range");
			}
		}
	};
}