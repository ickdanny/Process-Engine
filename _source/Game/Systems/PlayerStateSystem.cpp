#include "Game/Systems/PlayerStateSystem.h"

#include "Logging.h"

namespace process::game::systems {

    namespace {

        constexpr int noTimer{ -1 };
		
		using EntityHandle = wasp::ecs::entity::EntityHandle;

        bool isPlayerBomb(Scene& scene, PlayerData& playerData) {
            if (playerData.bombs > 0) {
                const auto& gameCommandChannel{
                    scene.getChannel(SceneTopics::gameCommands)
                };
                for (const auto& gameCommand : gameCommandChannel.getMessages()) {
                    if (gameCommand == GameCommands::bomb) {
                        return true;
                    }
                }
            }
            return false;
        }

        //Perfoms entry operations as defined by the CURRENT state of the state machine.
        void onEntry(
            Scene& scene, 
            const EntityHandle& playerHandle,
            PlayerData& playerData
        ) {

            scene.getChannel(SceneTopics::playerStateEntry).emplaceMessage(
                playerHandle,
                playerData.stateMachine.playerState
            );

            switch (playerData.stateMachine.playerState) {
                case PlayerStates::normal:
                case PlayerStates::gameOver:
                    playerData.stateMachine.timer = noTimer;
                    break;
                case PlayerStates::bombing:
                    playerData.stateMachine.timer = config::bombInvulnerabilityPeriod;
                    break;
                case PlayerStates::dead:
                    playerData.stateMachine.timer = config::deathPeriod;
                    break;
                case PlayerStates::respawning:
                    playerData.stateMachine.timer = config::respawnPeriod;
                    break;
                case PlayerStates::respawnInvulnerable:
                    playerData.stateMachine.timer = config::respawnInvulnerabilityPeriod;
                    break;
                case PlayerStates::none:
                    //do nothing
                    break;
                default:
                    throw std::runtime_error{
                        "unhandled player state in player state system onEntry()"
                    };
            }
        }

        //Updates the state machine and returns the next state.
        PlayerStates onUpdate(
            Scene& scene, 
            const EntityHandle& playerHandle,
            PlayerData& playerData
        ) {
            switch (playerData.stateMachine.playerState) {
                case PlayerStates::normal:
                    //first, check to see if we need to go to the bombing state
                    if (isPlayerBomb(scene, playerData)) {
                        return PlayerStates::bombing;
                    }

                    //otherwise, check to see if we are on a death clock
                    if (playerData.stateMachine.timer > 0) {
                        --playerData.stateMachine.timer;
                    }
                    else if (playerData.stateMachine.timer == 0) {
                        return PlayerStates::dead;
                    }
                    else if (scene.getChannel(SceneTopics::playerHits).hasMessages()) {
                        playerData.stateMachine.timer = config::deathBombPeriod;
                    }
                    return PlayerStates::normal;
                case PlayerStates::bombing:
                    //check to see if bomb state is over
                    if (playerData.stateMachine.timer > 0) {
                        --playerData.stateMachine.timer;
                    }
                    else if (playerData.stateMachine.timer == 0) {
                        return PlayerStates::normal;
                    }
                    return PlayerStates::bombing;
                case PlayerStates::dead:
                    //check to see if dead state is over
                    if (playerData.stateMachine.timer > 0) {
                        --playerData.stateMachine.timer;
                    }
                    else if (playerData.stateMachine.timer == 0) {
                        //check lives and continues to decide whether to respawn or not
                        if (playerData.lives <= 0 && playerData.continues <= 0) {
                            return PlayerStates::gameOver;
                        }
                        return PlayerStates::respawning;
                    }
                    return PlayerStates::dead;
                case PlayerStates::respawning:
                    //check to see if respawn state is over
                    if (playerData.stateMachine.timer > 0) {
                        --playerData.stateMachine.timer;
                    }
                    else if (playerData.stateMachine.timer == 0) {
                        return PlayerStates::respawnInvulnerable;
                    }
                    return PlayerStates::respawning;
                case PlayerStates::respawnInvulnerable:
                    //first, check to see if we need to go to the bombing state
                    if (isPlayerBomb(scene, playerData)) {
                        return PlayerStates::bombing;
                    }

                    //check to see if respawn invulnerable state is over
                    if (playerData.stateMachine.timer > 0) {
                        --playerData.stateMachine.timer;
                    }
                    else if (playerData.stateMachine.timer == 0) {
                        return PlayerStates::normal;
                    }
                    return PlayerStates::respawnInvulnerable;
                case PlayerStates::gameOver:
                    //this is a dead end, so just keep on sending to self
                    return PlayerStates::gameOver;
                case PlayerStates::none:
                    //send to normal state
                    return PlayerStates::normal;
                default:
                    throw std::runtime_error{
                        "unhandled player state in player state system onUpdate()"
                    };
            }
        }

        //Performs exit operations as defined by the CURRENT state of the state machine.
        void onExit(
            Scene& scene, 
            const EntityHandle& playerHandle,
            PlayerData& playerData
        ) {
            switch (playerData.stateMachine.playerState) {
                case PlayerStates::normal:
                case PlayerStates::bombing:
                case PlayerStates::dead:
                case PlayerStates::respawning:
                case PlayerStates::respawnInvulnerable:
                    playerData.stateMachine.timer = noTimer;
                    break;
                case PlayerStates::gameOver:
                    //do nothing
                    break;
                case PlayerStates::none:
                    //do nothing
                    break;
                default:
                    throw std::runtime_error{
                        "unhandled player state in player state system onExit()"
                    };
            }
        }

        void updatePlayerStateMachine(
            Scene& scene, 
            const EntityHandle& playerHandle,
            PlayerData& playerData
        ) {
            PlayerStates nextState{ onUpdate(scene, playerHandle, playerData) };
            if (nextState != playerData.stateMachine.playerState) {
                onExit(scene, playerHandle, playerData);
                playerData.stateMachine.playerState = nextState;
                onEntry(scene, playerHandle, playerData);

                //tail recursive call; if necessary can refactor this function to bool
                updatePlayerStateMachine(scene, playerHandle, playerData);
            }
        }
    }

	void PlayerStateSystem::operator()(Scene& scene) {

        //this system is responsible for clearing the playerStateEntry channel
        scene.getChannel(SceneTopics::playerStateEntry).clear();

        //get the group iterator for PlayerData
        static const Topic<wasp::ecs::component::Group*> groupPointerStorageTopic{};
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
            const EntityHandle& playerHandle{ groupIterator.getEntityID() };
            updatePlayerStateMachine(scene, playerHandle, playerData);
            ++groupIterator;
        }
	}
}