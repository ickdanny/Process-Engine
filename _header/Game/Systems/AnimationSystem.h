#pragma once

#include "systemInclude.h"
#include "Game/Resources/SpriteStorage.h"
#include "Components/AnimationList.h"

namespace process::game::systems {

    class AnimationSystem {

    private:
        //typedefs
        using EntityID = wasp::ecs::entity::EntityID;
        using EntityHandle = wasp::ecs::entity::EntityHandle;
        using DataStorage = wasp::ecs::DataStorage;
		using Group = wasp::ecs::component::Group;
		template <typename T>
		using Topic = wasp::channel::Topic<T>;
		using Animation = components::Animation;
		//fields
        resources::SpriteStorage* spriteStoragePointer{};

    public:
        AnimationSystem(resources::SpriteStorage* spriteStoragePointer);
        void operator()(Scene& scene);

    private:
        //helper functions

        //returns true if we need to remove this entity's animation list
        bool handleAnimation(
            DataStorage& dataStorage, 
            AnimationList& animationList,
            const EntityHandle& entityHandle
        );
        //returns true if animation changed, false otherwise.
        bool handleTurning(
            DataStorage& dataStorage,
            AnimationList& animationList,
            const EntityHandle& entityHandle
        );
        bool tryToTurnLeft(AnimationList& animationList);
        bool tryToTurnRight(AnimationList& animationList);
        bool tryToTurnCenter(AnimationList& animationList);

        //returns true if end of animation
        bool stepAnimation(AnimationList& animationList);
    };
}