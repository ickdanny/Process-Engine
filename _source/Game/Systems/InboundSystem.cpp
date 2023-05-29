#include "Game/Systems/InboundSystem.h"

namespace process::game::systems {

	namespace {
		constexpr math::Point2 inboundPosition(const math::Point2& pos, float bound) {
			math::Point2 copy{ pos };
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
		static const Topic<ecs::component::Group*> groupPointerStorageTopic{};
		auto groupPointer{
			getGroupPointer<Position, Velocity, Inbound>(
				scene,
				groupPointerStorageTopic
			)
		};
		auto groupIterator{ groupPointer->groupIterator<Position, Velocity, Inbound>() };
		
		//change all velocities to be in bound
		while (groupIterator.isValid()) {
			auto [position, velocity, inbound] = *groupIterator;
			math::Point2 nextPos{ position + velocity };
			if (isOutOfBounds(nextPos, inbound.bound)) {
				math::Point2 inboundPos{ inboundPosition(nextPos, inbound.bound) };
				math::Vector2 inboundVel = math::vectorFromAToB(position, inboundPos);
				velocity = Velocity{ inboundVel };
			}
			++groupIterator;
		}
	}
}