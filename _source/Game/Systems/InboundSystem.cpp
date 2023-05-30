#include "Game/Systems/InboundSystem.h"

namespace process::game::systems {

	namespace {
		using Point2 = wasp::math::Point2;
		using Vector2 = wasp::math::Vector2;
		
		constexpr Point2 inboundPosition(const Point2& pos, float bound) {
			Point2 copy{ pos };
			float lowXBound = bound + config::gameOffset.x;
			float lowYBound = bound + config::gameOffset.y;
			float highXBound = config::gameWidth - bound + config::gameOffset.x;
			float highYBound = config::gameHeight - bound + config::gameOffset.y;
			if (pos.x < lowXBound) {
				copy.x = lowXBound;
			}
			else if (pos.x > highXBound) {
				copy.x = highXBound;
			}
			if (pos.y < lowYBound) {
				copy.y = lowYBound;
			}
			else if (pos.y > highYBound) {
				copy.y = highYBound;
			}
			return copy;
		}
	}

	void InboundSystem::operator()(Scene& scene) {
		//get the group iterator for Position, Velocity, and Inbound
		static const Topic<Group*> groupPointerStorageTopic{};
		auto groupPointer{
			getGroupPointer<Position, Velocity, Inbound>(
				scene,
				groupPointerStorageTopic
			)
		};
		auto groupIterator{
			groupPointer->groupIterator<Position, Velocity, Inbound>()
		};
		
		//change all velocities to be in bound
		while (groupIterator.isValid()) {
			auto [position, velocity, inbound] = *groupIterator;
			Point2 nextPos{ position + velocity };
			if (isOutOfBounds(nextPos, inbound.bound)) {
				Point2 inboundPos{ inboundPosition(nextPos, inbound.bound) };
				Vector2 inboundVel = wasp::math::vectorFromAToB(position, inboundPos);
				velocity = Velocity{ inboundVel };
			}
			++groupIterator;
		}
	}
}