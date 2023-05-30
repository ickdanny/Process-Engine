#include "Game/Systems/SubImageScrollSystem.h"

namespace process::game::systems {

	namespace {
		void updateSubImage(SubImage& subImage, const SubImageScroll& subImageScroll) {
			subImage.x += subImageScroll.x;
			subImage.y += subImageScroll.y;
			subImage.width += subImageScroll.width;
			subImage.height += subImageScroll.height;

			if (subImage.x < 0.0f) {
				subImage.x = 0.0f;
			}
			if (subImage.y < 0.0f) {
				subImage.y = 0.0f;
			}
			if (subImage.width < 0.0f) {
				subImage.width = 0.0f;
			}
			if (subImage.height < 0.0f) {
				subImage.height = 0.0f;
			}
		}
	}

	void SubImageScrollSystem::operator()(Scene& scene) {
		static const Topic<wasp::ecs::component::Group*> groupPointerStorageTopic{};

		auto groupPointer{
			getGroupPointer<SubImage, SubImageScroll>(
				scene, groupPointerStorageTopic
			)
		};

		//step each subimage by subimagescroll
		auto groupIterator{ groupPointer->groupIterator<SubImage, SubImageScroll>() };
		while (groupIterator.isValid()) {
			const auto [subImage, subImageScroll] = *groupIterator;
			updateSubImage(subImage, subImageScroll);
			++groupIterator;
		}
	}
}