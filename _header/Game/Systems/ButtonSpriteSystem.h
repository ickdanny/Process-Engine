#pragma once

#include "systemInclude.h"
#include "Game/Resources/SpriteStorage.h"

namespace process::game::systems {

    class ButtonSpriteSystem {

    private:
        //typedefs
        using EntityHandle = wasp::ecs::entity::EntityHandle;
        using DataStorage = wasp::ecs::DataStorage;

        //fields
        resources::SpriteStorage* spriteStoragePointer{};

    public:
        ButtonSpriteSystem(resources::SpriteStorage* spriteStoragePointer);

        void operator()(Scene& scene);

    private:
        //helper functions
        void selectButton(DataStorage& dataStorage, const EntityHandle& buttonHandle);
        void unselectButton(DataStorage& dataStorage, const EntityHandle& buttonHandle);
        void changeSpriteAndPosition(
            DataStorage& dataStorage,
            const EntityHandle& buttonHandle,
            const std::wstring& spriteName,
            const wasp::math::Point2& position
        );
    };
}