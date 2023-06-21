#pragma once

#include <utility>
#include <vector>
#include <string>
#include <stdexcept>

#include "Utility/Ticker.h"

namespace process::game::components {

	struct Animation {
		//fields
		std::vector<std::wstring> frames{};
		bool looping{};
		std::size_t currentIndex{};

		explicit Animation(std::vector<std::wstring> frames, bool looping = true)
			: frames{ std::move( frames ) }
			, looping{ looping }
			, currentIndex{ 0 } {
		}

		std::wstring getCurrentFrame() {
			if (currentIndex >= frames.size()) {
				throw std::runtime_error{ "bad index in animation" };
			}
			return frames[currentIndex];
		}
	};

	struct AnimationList {
	private:
		//typedefs
		using Ticker = wasp::utility::Ticker;

	public:
		//fields
		std::vector<Animation> animations{};
		std::size_t idleIndex{};
		std::size_t currentIndex{};
		Ticker ticker;			//uninitialized!

		AnimationList(
			std::vector<Animation> animations,
			std::size_t idleIndex, 
			int ticks
		)
			: animations{ std::move( animations ) }
			, idleIndex{ idleIndex }
			, currentIndex{ idleIndex }
			, ticker{ ticks, true } {
		}

		AnimationList(const Animation& animation, int ticks)
			: animations{ { animation } }
			, idleIndex{ 0 }
			, currentIndex{ idleIndex }
			, ticker{ ticks, true } {
		}

		AnimationList(const std::vector<std::wstring>& frames, int ticks)
			: animations{ { static_cast<Animation>(frames) } }
			, idleIndex{ 0 }
			, currentIndex{ idleIndex }
			, ticker{ ticks, true } {
		}

		Animation& getCurrentAnimation() {
			if (currentIndex >= animations.size()) {
				throw std::runtime_error{ "bad index in animation list" };
			}
			return animations[currentIndex];
		}
	};
}