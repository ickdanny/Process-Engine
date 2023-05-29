#include "Game/Systems/AnimationSystem.h"

namespace process::game::systems {

	namespace {
		constexpr float velocityEpsilon{ .01f };
	}

	AnimationSystem::AnimationSystem(resources::SpriteStorage* spriteStoragePointer)
		: spriteStoragePointer{ spriteStoragePointer } {
	}

	void AnimationSystem::operator()(Scene& scene) {
		//store entityHandles to remove animation component after running
		std::vector<EntityHandle> componentsToRemove{};

		//get the group iterator for AnimationList
		static const Topic<Group*> groupPointerStorageTopic{};
		auto groupPointer{
			getGroupPointer<AnimationList>(
				scene,
				groupPointerStorageTopic
			)
		};
		auto groupIterator{ groupPointer->groupIterator<AnimationList>() };

		auto& dataStorage{ scene.getDataStorage() };

		while (groupIterator.isValid()) {
			auto [animationList] = *groupIterator;
			if (animationList.ticker.stepAndGetTick() == 1) {
				EntityID entityID{ groupIterator.getEntityID() };
				EntityHandle entityHandle{ dataStorage.makeHandle(entityID) };

				if (handleAnimation(
					dataStorage,
					animationList,
					entityHandle
				)) {
					componentsToRemove.push_back(entityHandle);
				}
			}
			++groupIterator;
		}

		//remove finished animation components
		for (auto& entityHandle : componentsToRemove) {
			dataStorage.removeComponent(
				wasp::ecs::RemoveComponentOrder<AnimationList>{entityHandle}
			);
		}
	}

	bool AnimationSystem::handleAnimation(
		DataStorage& dataStorage,
		AnimationList& animationList,
		const EntityHandle& entityHandle
	) {
		bool removeComponent{ false };
		if (!handleTurning(dataStorage, animationList, entityHandle)) {
			removeComponent = stepAnimation(animationList);
		}
		if (dataStorage.containsComponent<SpriteInstruction>(entityHandle)) {
			auto& spriteInstruction{
				dataStorage.getComponent<SpriteInstruction>(entityHandle)
			};
			spriteInstruction.setSprite(
				spriteStoragePointer->get(
					animationList.getCurrentAnimation().getCurrentFrame()
				)->sprite
			);
		}
		return removeComponent;
	}

	bool AnimationSystem::handleTurning(
		DataStorage& dataStorage,
		AnimationList& animationList,
		const EntityHandle& entityHandle
	) {

		if (animationList.animations.size() > 1) {
			if (dataStorage.containsComponent<Velocity>(entityHandle)) {
				const Velocity& velocity{
					dataStorage.getComponent<Velocity>(entityHandle)
				};
				float velocityX{ static_cast<wasp::math::Vector2>(velocity).x };
				if (velocityX < -velocityEpsilon) {
					return tryToTurnLeft(animationList);
				}
				else if (velocityX > velocityEpsilon) {
					return tryToTurnRight(animationList);
				}
				else {
					return tryToTurnCenter(animationList);
				}
			}
		}
		return false;
	}

	bool AnimationSystem::tryToTurnLeft(AnimationList& animationList) {
		if (animationList.currentIndex > 0) {
			std::size_t nextIndex{ animationList.currentIndex - 1 };

			//reset the indices of the animations
			animationList.animations[animationList.currentIndex].currentIndex = 0;
			animationList.animations[nextIndex].currentIndex = 0;

			//set our animation left
			animationList.currentIndex = nextIndex;
			return true;
		}
		return false;
	}

	bool AnimationSystem::tryToTurnRight(AnimationList& animationList) {
		if (animationList.currentIndex + 1 < animationList.animations.size()) {
			std::size_t nextIndex{ animationList.currentIndex + 1 };

			//reset the indices of the animations
			animationList.animations[animationList.currentIndex].currentIndex = 0;
			animationList.animations[nextIndex].currentIndex = 0;

			//set our animation right
			animationList.currentIndex = nextIndex;
			return true;
		}
		return false;
	}

	bool AnimationSystem::tryToTurnCenter(AnimationList& animationList) {
		if (animationList.currentIndex != animationList.idleIndex) {
			if (animationList.currentIndex < animationList.idleIndex) {
				return tryToTurnRight(animationList);
			}
			else {
				return tryToTurnLeft(animationList);
			}
		}
		return false;
	}

	bool AnimationSystem::stepAnimation(AnimationList& animationList) {
		Animation& currentAnimation{ animationList.getCurrentAnimation() };
		std::size_t nextIndex{ currentAnimation.currentIndex + 1 };
		if (nextIndex >= currentAnimation.frames.size()) {
			if (!currentAnimation.looping) {
				return true;
			}
			nextIndex = 0;
		}
		currentAnimation.currentIndex = nextIndex;
		return false;
	}
}