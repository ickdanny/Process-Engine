#include "Game/Systems/OverlaySystem.h"

namespace process::game::systems {

    namespace {
        constexpr float initX{ 242.0f };
        constexpr float initY{ 28.0f };

        constexpr float xOffset{ 13.0f };
        constexpr float yOffset{ 27.0f };

        constexpr math::Vector2 offset{ xOffset, 0.0f };

        constexpr math::Point2 lifeInitPos{ initX, initY };
        constexpr math::Point2 bombInitPos{ initX, initY + yOffset };
        constexpr math::Point2 powerMeterPos{
            initX - 5.0f,
            bombInitPos.y + yOffset
        };
    }

	void OverlaySystem::operator()(Scene& scene) {
        //get the group iterator for PlayerData
        static const Topic<ecs::component::Group*> groupPointerStorageTopic{};
        auto groupPointer{
            getGroupPointer<PlayerData>(
                scene,
                groupPointerStorageTopic
            )
        };
        auto groupIterator{ groupPointer->groupIterator<PlayerData>() };

        //update all player states
        while (groupIterator.isValid()) {
            auto [playerData] = *groupIterator;
            updateOverlay(scene, playerData);
            ++groupIterator;
            break;      //we only update on the first player for now
        }
	}

    void OverlaySystem::updateOverlay(Scene& scene, const PlayerData& playerData) {
        SceneData& sceneData{ getSceneData(scene, playerData) };
        updateLives(scene, sceneData, playerData);
        updateBombs(scene, sceneData, playerData);
        updatePower(scene, sceneData, playerData);
    }

    OverlaySystem::SceneData& OverlaySystem::getSceneData(
        Scene& scene, 
        const PlayerData& playerData
    ) {
        static const Topic<SceneData> sceneDataTopic{};
        auto& sceneDataChannel{ scene.getChannel(sceneDataTopic) };

        if (!sceneDataChannel.hasMessages()) {
            //create the overlay

            auto& dataStorage{ scene.getDataStorage() };

            std::array<EntityHandle, config::maxLives> lifeHandles{};
            for (int i{ 0 }; i < config::maxLives; ++i) {
                lifeHandles[i] = dataStorage.addEntity(
                    makeIcon(
                        lifeInitPos,
                        offset,
                        i,
                        L"ui_life"
                    ).package()
                );
            }

            std::array<EntityHandle, config::maxBombs> bombHandles{};
            for (int i{ 0 }; i < config::maxBombs; ++i) {
                bombHandles[i] = dataStorage.addEntity(
                    makeIcon(
                        bombInitPos,
                        offset,
                        i,
                        L"ui_bomb"
                    ).package()
                );
            }

            EntityHandle powerMeterHandle{
                dataStorage.addEntity(
                    (makeIcon(
                        powerMeterPos,
                        {},
                        0,
                        playerData.power == config::maxPower 
                            ? L"ui_power_max" 
                            : L"ui_power"
                    ) + SubImage{ 0.0f, 0.0f, 80.0f, 13.0f }).package()
                )
            };

            SceneData sceneData{
                std::move(lifeHandles),
                std::move(bombHandles),
                powerMeterHandle,
                -1,
                -1
            };
            sceneDataChannel.addMessage(sceneData);
        }
        return sceneDataChannel.getMessages()[0];
    }

    OverlaySystem::IconComponentTuple OverlaySystem::makeIcon(
        const math::Point2& initPos,
        const math::Vector2& offset,
        int index,
        const std::wstring& imageName
    ) const {
        math::Point2 position{ initPos + (offset * static_cast<float>(index)) };
        return EntityBuilder::makePosition(
            position,
            SpriteInstruction{
                bitmapStoragePointer->get(imageName)->d2dBitmap
            },
            DrawOrder{ config::foregroundDrawOrder + 1 }
        );
    }

    void OverlaySystem::updateLives(
        Scene& scene,
        SceneData& sceneData,
        const PlayerData& playerData
    ) {
        auto& dataStorage{ scene.getDataStorage() };
        auto& [lifeHandles, unused1, unused2, currentLifeIndex, unused3] = sceneData;

        int playerLifeIndex = playerData.lives - 1;
        if (playerLifeIndex >= -1) {
            if (currentLifeIndex > playerLifeIndex) {
                for (
                    int i{ std::min(currentLifeIndex, config::maxLives - 1) }; 
                    i > playerLifeIndex;
                    --i
                ) {
                    dataStorage.removeComponent<VisibleMarker>(
                        { lifeHandles[i] }
                    );
                }
            }
            else if (currentLifeIndex < playerLifeIndex) {
                for (
                    int i{ std::max(currentLifeIndex + 1, 0) }; 
                    i <= playerLifeIndex; 
                    ++i
                ) {
                    dataStorage.setComponent<VisibleMarker>({ lifeHandles[i], {} });
                }
            }
            currentLifeIndex = playerLifeIndex;
        }
    }

    void OverlaySystem::updateBombs(
        Scene& scene,
        SceneData& sceneData,
        const PlayerData& playerData
    ) {
        auto& dataStorage{ scene.getDataStorage() };
        auto& [unused1, bombHandles, unused2, unused3, currentBombIndex] = sceneData;

        int playerBombIndex = playerData.bombs - 1;
        if (playerBombIndex >= -1) {
            if (currentBombIndex > playerBombIndex) {
                for (
                    int i{ std::min(currentBombIndex, config::maxBombs - 1) };
                    i > playerBombIndex;
                    --i
                ) {
                    dataStorage.removeComponent<VisibleMarker>(
                        { bombHandles[i] }
                    );
                }
            }
            else if (currentBombIndex < playerBombIndex) {
                for (
                    int i{ std::max(currentBombIndex + 1, 0) };
                    i <= playerBombIndex;
                    ++i
                ) {
                    dataStorage.setComponent<VisibleMarker>({ bombHandles[i], {} });
                }
            }
            currentBombIndex = playerBombIndex;
        }
    }

    void OverlaySystem::updatePower(
        Scene& scene,
        SceneData& sceneData,
        const PlayerData& playerData
    ) {
        auto& dataStorage{ scene.getDataStorage() };
        auto& [unused1, unused2, powerMeterHandle, unused3, unused4] = sceneData;
        auto& spriteInstruction{ 
            dataStorage.getComponent<SpriteInstruction>(powerMeterHandle) 
        };

        int currentPower{ playerData.power };

        //if power is 0, make the power meter invisible
        if (currentPower == 0) {
            if (dataStorage.containsComponent<VisibleMarker>(powerMeterHandle)) {
                dataStorage.removeComponent(
                    ecs::RemoveComponentOrder<VisibleMarker>{ powerMeterHandle }
                );
            }
        }
        //otherwise, make sure the power meter is visible
        else if (!dataStorage.containsComponent<VisibleMarker>(powerMeterHandle)) {
            dataStorage.addComponent(
                ecs::AddComponentOrder<VisibleMarker>{ powerMeterHandle, {} }
            );
        }
        //if power is not max, set the sprite to the non-max version
        if (currentPower != config::maxPower) {
            spriteInstruction.setBitmap(
                bitmapStoragePointer->get(L"ui_power")->d2dBitmap
            );
        }
        //otherwise, set the sprite to the max version
        else {
            spriteInstruction.setBitmap(
                bitmapStoragePointer->get(L"ui_power_max")->d2dBitmap
            );
        }
        //update our subimage
        auto& subImage{ dataStorage.getComponent<SubImage>(powerMeterHandle) };
        subImage.width = static_cast<float>(currentPower);

        //update our position
        auto& position{ dataStorage.getComponent<Position>(powerMeterHandle) };
        position.x = powerMeterPos.x + static_cast<float>(currentPower) / 2;
    }
}