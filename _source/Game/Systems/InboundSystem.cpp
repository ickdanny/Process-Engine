#include "Game/Systems/InboundSystem.h"

namespace process::game::systems {

	namespace {
		using Point2 = wasp::math::Point2;
		using Vector2 = wasp::math::Vector2;
		
		void inboundPosition(Point2& pos, float bound) {
			float lowXBound = bound + config::gameOffset.x;
			float lowYBound = bound + config::gameOffset.y;
			float highXBound = config::gameWidth - bound + config::gameOffset.x;
			float highYBound = config::gameHeight - bound + config::gameOffset.y;
			if (pos.x < lowXBound) {
				pos.x = lowXBound;
			}
			else if (pos.x > highXBound) {
				pos.x = highXBound;
			}
			if (pos.y < lowYBound) {
				pos.y = lowYBound;
			}
			else if (pos.y > highYBound) {
				pos.y = highYBound;
			}
		}
	}

	void InboundSystem::operator()(Scene& scene) {
		//get the group iterator for Position and Inbound
		static const Topic<Group*> groupPointerStorageTopic{};
		auto groupPointer{
			getGroupPointer<Position, Inbound>(
				scene,
				groupPointerStorageTopic
			)
		};
		auto groupIterator{
			groupPointer->groupIterator<Position, Inbound>()
		};
		
		//change all velocities to be in bound
		while (groupIterator.isValid()) {
			auto [position, inbound] = *groupIterator;
			inboundPosition(position, inbound.bound);
			++groupIterator;
		}
	}
}