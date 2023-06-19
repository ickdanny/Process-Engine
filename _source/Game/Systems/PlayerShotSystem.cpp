#include "Game/Systems/PlayerShotSystem.h"

//#include "Game/Systems/Programs/PlayerPrograms.h"

namespace process::game::systems {

	void PlayerShotSystem::operator()(Scene& scene) {

		//this system only works if there are game commands
		if (scene.hasChannel(SceneTopics::gameCommands)) {
			auto& gameCommandChannel{ scene.getChannel(SceneTopics::gameCommands) };
			if (gameCommandChannel.hasMessages()) {

				//if there is a shoot command, add player shot
				for (auto& gameCommand : gameCommandChannel.getMessages()) {
					if (gameCommand == GameCommands::shoot) {
						addPlayerShot(scene);
						break;
					}
				}
			}
		}
	}

	void PlayerShotSystem::addPlayerShot(Scene& scene) {
		//get the iterator for players
		static const Topic<wasp::ecs::component::Group*> groupPointerStorageTopic{};
		auto groupPointer{
			getGroupPointer<PlayerData, Position, ScriptList>(
				scene,
				groupPointerStorageTopic
			)
		};
		auto groupIterator{ 
			groupPointer->groupIterator<PlayerData, ScriptList>()
		};
		while (groupIterator.isValid()) {
			auto [playerData, scriptList] = *groupIterator;

			//check to see if the player already has a shot spawn program by
			//checking each active spawn program's max tick
			bool isPlayerAlreadyShooting{ false };
			for (const auto& scriptContainer : scriptList) {
				//todo: check for player shot program
				/*
				if (spawnProgram.getMaxTick() == config::playerShotMaxTick) {
					isPlayerAlreadyShooting = true;
					break;
				}
				 */
			}

			//if there is no pre-existing shot program, add one
			if (!isPlayerAlreadyShooting) {
				//todo: player shot programs
				/*
				if (playerData.shotType == ShotType::shotA) {
					spawnProgramList.push_back(
						{ programsPointer->playerPrograms.shotASpawnProgram }
					);
				}
				else if (playerData.shotType == ShotType::shotB) {
					spawnProgramList.push_back(
						{ programsPointer->playerPrograms.shotBSpawnProgram }
					);
				}
				else {
					throw std::runtime_error("unexpected player shot type!");
				}
				 */
			}
			++groupIterator;
		}
	}
}