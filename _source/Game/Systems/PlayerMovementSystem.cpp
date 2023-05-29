#include "Game/Systems/PlayerMovementSystem.h"

namespace process::game::systems {

    namespace {
        //checks if this system should operate on the player based on their state
        bool canMove(const PlayerData& playerData) {
            switch (playerData.stateMachine.playerState) {
                case PlayerStates::normal:
                case PlayerStates::bombing:
                case PlayerStates::respawnInvulnerable:
                    return true;
                case PlayerStates::dead:
                case PlayerStates::respawning:
                case PlayerStates::gameOver:
                    return false;
                default:
                    throw std::runtime_error{ "Unexpected player state!" };
            }
        }
    }

	void PlayerMovementSystem::operator()(Scene& scene) {

        //this system only operators on scenes with game commands 
        if (scene.hasChannel(SceneTopics::gameCommands)) {

            auto& gameCommandChannel{ scene.getChannel(SceneTopics::gameCommands) };

            //retrieve system data from the scene
            static Topic<TwoFramePlayerInputData> inputDataTopic{};
            auto& inputDataChannel{
                scene.getChannel(inputDataTopic)
            };
            if (inputDataChannel.isEmpty()) {
                inputDataChannel.addMessage({ });
            }
            auto& twoFramePlayerInputData{ inputDataChannel.getMessages()[0] };

            //update the input data
            twoFramePlayerInputData.reset();
            if (gameCommandChannel.hasMessages()) {
                for (auto& gameCommand : gameCommandChannel.getMessages()) {
                    parseGameCommand(gameCommand, twoFramePlayerInputData);
                }
            }

            //update player velocity if needed
            if (twoFramePlayerInputData != twoFramePlayerInputData.getPast()) {
                Vector2 velocity = calculateVelocity(twoFramePlayerInputData);

                static const Topic<ecs::component::Group*> groupPointerStorageTopic{};

                auto groupPointer{
                    getGroupPointer<PlayerData, Velocity>(
                        scene, 
                        groupPointerStorageTopic
                    )
                };

                auto groupIterator{ 
                    groupPointer->groupIterator<PlayerData, Velocity>() 
                };

                while (groupIterator.isValid()) {
                    auto [playerData, playerVelocity] = *groupIterator;
                    if (canMove(playerData)) {
                        playerVelocity = Velocity{ velocity };
                    }
                    else {
                        playerVelocity = Velocity{};
                    }
                    ++groupIterator;
                }
            }
            twoFramePlayerInputData.step();
        }
	}



    //updates the given PlayerInputData based on the specified game command
    void PlayerMovementSystem::parseGameCommand(
        GameCommands gameCommand,
        PlayerInputData& inputData
    ) {
        switch (gameCommand) {
            case GameCommands::focus:
                inputData.setFocus(true);
                break;
            case GameCommands::up:
                inputData.setUp(true);
                break;
            case GameCommands::down:
                inputData.setDown(true);
                break;
            case GameCommands::left:
                inputData.setLeft(true);
                break;
            case GameCommands::right:
                inputData.setRight(true);
                break;
        }
    }

    //converts a PlayerInputData object into a Vector2 object
    math::Vector2 PlayerMovementSystem::calculateVelocity(
        const PlayerInputData inputData
    ) {
        //return the zero vector if either no direction is pressed or all directions are
        if (inputData.isZero()) {
            return {};
        }

        //use ints to prevent floating point nonsense
        int x{ 0 };
        int y{ 0 };
        if (inputData.isUp()) {
            --y;
        }
        if (inputData.isDown()) {
            ++y;
        }
        if (inputData.isLeft()) {
            --x;
        }
        if (inputData.isRight()) {
            ++x;
        }
        if (x | y) {
            Vector2 velocity{ static_cast<float>(x), static_cast<float>(y) };
            float magnitude = inputData.isFocused() 
                ? config::focusedSpeed 
                : config::playerSpeed;
            velocity *= magnitude / math::getMagnitude(velocity);
            return velocity;
        }
        else {
            return {};
        }
    }
}