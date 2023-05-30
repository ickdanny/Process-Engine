#include "Game/Systems/SpawnSystem.h"

#include <array>

namespace process::game::systems {

	using namespace wasp::ecs;
	using namespace wasp::ecs::entity;

	#define NODE_HANDLER_ARGS \
			Scene& scene, \
			EntityID entityID, \
			std::shared_ptr<SpawnNode>& currentSpawnNodePointer, \
			int tick, \
			SpawnList& spawnList

	SpawnSystem::SpawnSystem(wasp::channel::ChannelSet* globalChannelSetPointer)
		: globalChannelSetPointer{ globalChannelSetPointer } {
	}

	void SpawnSystem::operator()(Scene& scene) {
		//create our spawnlist
		SpawnList spawnList{};

		//get the group iterator for SpawnProgramList
		static const Topic<wasp::ecs::component::Group*> groupPointerStorageTopic{};
		auto groupPointer{
			getGroupPointer<SpawnProgramList>(
				scene,
				groupPointerStorageTopic
			)
		};
		auto groupIterator{ groupPointer->groupIterator<SpawnProgramList>() };

		//populate our spawnList with every spawn this tick
		while (groupIterator.isValid()) {
			auto [spawnProgramList] = *groupIterator;
			EntityID entityID{ groupIterator.getEntityID() };
			runSpawnProgramList(scene, spawnProgramList, entityID, spawnList);
			++groupIterator;
		}

		//add our spawnList to the ecs world
		auto& dataStorage{ scene.getDataStorage() };
		for (const auto& spawnPointer : spawnList) {
			spawnPointer->addTo(dataStorage);
		}
	}

	void SpawnSystem::runSpawnProgramList(
		Scene& scene,
		SpawnProgramList& spawnProgramList,
		EntityID entityID,
		SpawnList& spawnList
	) {
		for (auto itr{ spawnProgramList.begin() }; itr != spawnProgramList.end();) {
			if (runSpawnProgram(scene, *itr, entityID, spawnList)) {
				++itr;
			}
			else {
				itr = spawnProgramList.erase(itr);
			}
		}
	}

	bool SpawnSystem::runSpawnProgram(
		Scene& scene,
		SpawnProgram& spawnProgram,
		EntityID entityID,
		SpawnList& spawnList
	) {
		int tick{ spawnProgram.stepAndGetTick() };
		if (tick <= 0) {
			return false;
		}
		auto currentSpawnNodePointer{ spawnProgram.baseSpawnNodePointer };

		while (currentSpawnNodePointer) {
			runSpawnNode(
				scene,
				entityID,
				currentSpawnNodePointer,
				tick,
				spawnList
			);
		}

		return true;
	}

	void SpawnSystem::runSpawnNode(NODE_HANDLER_ARGS) {
		switch (currentSpawnNodePointer->spawnInstruction) {
			case SpawnInstructions::error:
				//throw an error
				throw std::runtime_error{ "spawn program error instruction" };
			case SpawnInstructions::list: {
				//run every node contained in the list
				for (auto linkedNodeSharedPointer	//intentional copy
					: currentSpawnNodePointer->linkedNodePointers
				) {
					while (linkedNodeSharedPointer) {
						runSpawnNode(
							scene,
							entityID,
							linkedNodeSharedPointer,
							tick,
							spawnList
						);
					}
				}
				currentSpawnNodePointer = nullptr;
				break;
			}
			case SpawnInstructions::repeat: {
				int times{ evaluateIntNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[0],
					tick,
					spawnList
				) };
				for (int i{ 0 }; i < times; ++i) {
					auto linkedNodeSharedPointer{	//intentional copy
						currentSpawnNodePointer->linkedNodePointers[1]
					};
					while (linkedNodeSharedPointer) {
						runSpawnNode(
							scene,
							entityID,
							linkedNodeSharedPointer,
							tick,
							spawnList
						);
					}
				}
				currentSpawnNodePointer = nullptr;
				break;
			}
			case SpawnInstructions::condition: {
				//if our predicate is met, set the current pointer to trueNode
				if (evaluatePredicateNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[0],
					tick,
					spawnList
				)) {
					currentSpawnNodePointer =
						currentSpawnNodePointer->linkedNodePointers[1];
				}
				else {
					currentSpawnNodePointer = nullptr;
				}
				break;
			}
			case SpawnInstructions::conditionElse: {
				//if our predicate is met, set the current pointer to trueNode
				if (evaluatePredicateNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[0],
					tick,
					spawnList
				)) {
					currentSpawnNodePointer =
						currentSpawnNodePointer->linkedNodePointers[1];
				}
				//otherwise, set the current pointer to falseNode
				else {
					currentSpawnNodePointer =
						currentSpawnNodePointer->linkedNodePointers[2];
				}
				break;
			}
			case SpawnInstructions::playerPowerSplit: {
				//chooses node based on player power
				//buckets are as follows
				//0  - 7	bucket0
				//8  - 15	bucket1
				//16 - 31	bucket2
				//32 - 47	bucket3
				//48 - 63	bucket4
				//64 - 79	bucket5
				//80		bucket6
				const PlayerData& playerData{
					scene.getDataStorage().getComponent<PlayerData>(entityID)
				};
				int bucket{ -1 };
				switch (playerData.power / 8) {
					case 0:
						bucket = 0;
						break;
					case 1:
						bucket = 1;
						break;
					case 2:
					case 3:
						bucket = 2;
						break;
					case 4:
					case 5:
						bucket = 3;
						break;
					case 6:
					case 7:
						bucket = 4;
						break;
					case 8:
					case 9:
						bucket = 5;
						break;
					case 10:
						bucket = 6;
						break;
					default:
						throw std::runtime_error{
							"unexpected power: " + std::to_string(playerData.power)
						};
				}
				currentSpawnNodePointer =
					currentSpawnNodePointer->linkedNodePointers[bucket];
				break;
			}
			case SpawnInstructions::entityPosition: {
				wasp::math::Point2 pos{
					scene.getDataStorage().getComponent<Position>(entityID)
				};

				auto posConsumerSharedPointer{
					currentSpawnNodePointer->linkedNodePointers[0]
				};
				while (posConsumerSharedPointer) {
					runSpawnNodePassingPos(
						scene,
						entityID,
						posConsumerSharedPointer,
						tick,
						spawnList,
						pos
					);
				}

				currentSpawnNodePointer = nullptr;
				break;
			}
			case SpawnInstructions::entityOffset: {
				wasp::math::Point2 basePos{
					scene.getDataStorage().getComponent<Position>(entityID)
				};
				Velocity offset{ evaluateVelocityNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[0],
					tick,
					spawnList
				) };

				wasp::math::Point2 pos{ basePos + offset };
				
				auto posConsumerSharedPointer{
					currentSpawnNodePointer->linkedNodePointers[1]
				};
				while (posConsumerSharedPointer) {
					runSpawnNodePassingPos(
						scene,
						entityID,
						posConsumerSharedPointer,
						tick,
						spawnList,
						pos
					);
				}

				currentSpawnNodePointer = nullptr;
				break;
			}
			case SpawnInstructions::pointFromFloats: {
				float x{ evaluateFloatNode(
						scene,
						entityID,
						currentSpawnNodePointer->linkedNodePointers[0],
						tick,
						spawnList
				) };
				float y{ evaluateFloatNode(
						scene,
						entityID,
						currentSpawnNodePointer->linkedNodePointers[1],
						tick,
						spawnList
				) };
				wasp::math::Point2 pos{ x, y };

				auto posConsumerSharedPointer{
					currentSpawnNodePointer->linkedNodePointers[2]
				};
				while (posConsumerSharedPointer) {
					runSpawnNodePassingPos(
						scene,
						entityID,
						posConsumerSharedPointer,
						tick,
						spawnList,
						pos
					);
				}

				currentSpawnNodePointer = nullptr;
				break;
			}
			case SpawnInstructions::spawn: {
				//cast the node data to a ComponentTupleSharedPtr and add to our list
				const auto& [componentTupleBaseSharedPtr] =
					dynamic_cast<
					const SpawnNodeData<
					std::shared_ptr<ComponentTupleBase>
					>*
					>(
						currentSpawnNodePointer.get()
						)->data;
				spawnList.emplace_back(componentTupleBaseSharedPtr);
				currentSpawnNodePointer = nullptr;
				break;
			}
			case SpawnInstructions::spawnPos: {
				//cast the node data to a ComponentTupleSharedPtr
				const auto& [componentTupleBaseSharedPtr] =
					dynamic_cast<
					const SpawnNodeData<
					std::shared_ptr<ComponentTupleBase>
					>*
					>(
						currentSpawnNodePointer.get()
						)->data;
				//get the position
				wasp::math::Point2 pos{ evaluatePointNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[0],
					tick,
					spawnList
				) };
				//add position
				spawnList.emplace_back(
					componentTupleBaseSharedPtr->addPosition(pos)
				);
				currentSpawnNodePointer = nullptr;
				break;
			}
			case SpawnInstructions::spawnPosVel: {
				//cast the node data to a ComponentTupleSharedPtr
				const auto& [componentTupleBaseSharedPtr] =
					dynamic_cast<
					const SpawnNodeData<
					std::shared_ptr<ComponentTupleBase>
					>*
					>(
						currentSpawnNodePointer.get()
						)->data;
				//get the position
				wasp::math::Point2 pos{ evaluatePointNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[0],
					tick,
					spawnList
				) };
				//get the velocity
				Velocity vel{ evaluateVelocityNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[1],
					tick,
					spawnList
				) };
				//add position and velocity
				spawnList.emplace_back(
					componentTupleBaseSharedPtr->addPosVel(pos, vel)
				);
				currentSpawnNodePointer = nullptr;
				break;
			}
			case SpawnInstructions::mirrorFormation: {
				//get the base position
				wasp::math::Point2 basePos{ evaluatePointNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[0],
					tick,
					spawnList
				) };
				//get the base velocity
				Velocity baseVel{ evaluateVelocityNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[1],
					tick,
					spawnList
				) };
				//get the axis of symmetry (x coordinate to reflect around)
				float axis{ evaluateFloatNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[2],
					tick,
					spawnList
				) };

				//in point slope form, our new X n follows the equation
				// n - a = -(o - a)
				// where a is our axis and o is the old X
				wasp::math::Point2 mirrorPos{ -basePos.x + (2 * axis), basePos.y };
				Velocity mirrorVel{
					baseVel.getMagnitude(),
					baseVel.getAngle().flipY()
				};

				//pass both normal and reflected pos/vel pairs
				auto posVelConsumerSharedPointer{
					currentSpawnNodePointer->linkedNodePointers[3]
				};
				while (posVelConsumerSharedPointer) {
					runSpawnNodePassingPosVel(
						scene,
						entityID,
						posVelConsumerSharedPointer,
						tick,
						spawnList,
						basePos,
						baseVel
					);
				}
				posVelConsumerSharedPointer =
					currentSpawnNodePointer->linkedNodePointers[3];
				while (posVelConsumerSharedPointer) {
					runSpawnNodePassingPosVel(
						scene,
						entityID,
						posVelConsumerSharedPointer,
						tick,
						spawnList,
						mirrorPos,
						mirrorVel
					);
				}

				currentSpawnNodePointer = nullptr;
				break;
			}
			case SpawnInstructions::mirrorPosFormation: {
				//get the base position
				wasp::math::Point2 basePos{ evaluatePointNode(
					scene,
					entityID,
					{ currentSpawnNodePointer->linkedNodePointers[0] },
					tick,
					spawnList
				) };
				//get the axis of symmetry (x coordinate to reflect around)
				float axis{ evaluateFloatNode(
					scene,
					entityID,
					{ currentSpawnNodePointer->linkedNodePointers[1] },
					tick,
					spawnList
				) };

				//in point slope form, our new X n follows the equation
				// n - a = -(o - a)
				// where a is our axis and o is the old X
				wasp::math::Point2 mirrorPos{ -basePos.x + (2 * axis), basePos.y };

				//pass both normal and reflected pos
				auto posConsumerSharedPointer{
					currentSpawnNodePointer->linkedNodePointers[2]
				};
				while (posConsumerSharedPointer) {
					runSpawnNodePassingPos(
						scene,
						entityID,
						posConsumerSharedPointer,
						tick,
						spawnList,
						basePos
					);
				}
				posConsumerSharedPointer =
					currentSpawnNodePointer->linkedNodePointers[2];
				while (posConsumerSharedPointer) {
					runSpawnNodePassingPos(
						scene,
						entityID,
						posConsumerSharedPointer,
						tick,
						spawnList,
						mirrorPos
					);
				}

				currentSpawnNodePointer = nullptr;
				break;
			}
			case SpawnInstructions::arcFormation: {
				//get the base velocity
				Velocity baseVel{ evaluateVelocityNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[0],
					tick,
					spawnList
				) };
				int symmetry{ evaluateIntNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[1],
					tick,
					spawnList
				) };
				float angleIncrement{ evaluateFloatNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[2],
					tick,
					spawnList
				) };
				float speed{ baseVel.getMagnitude() };
				wasp::math::Angle baseAngle{ baseVel.getAngle() };
				wasp::math::Angle angle{
					static_cast<float>(baseAngle) -
					((symmetry - 1) * angleIncrement / 2.0f)
				};
				for (int i{ 0 }; i < symmetry; ++i) {
					auto velConsumerSharedPointer{
						currentSpawnNodePointer->linkedNodePointers[3]
					};
					while (velConsumerSharedPointer) {
						runSpawnNodePassingVel(
							scene,
							entityID,
							velConsumerSharedPointer,
							tick,
							spawnList,
							Velocity{ speed, angle }
						);
					}
					angle += angleIncrement;
				}

				currentSpawnNodePointer = nullptr;
				break;
			}
			case SpawnInstructions::ringFormation: {
				//get the base velocity
				Velocity baseVel{ evaluateVelocityNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[0],
					tick,
					spawnList
				) };
				int symmetry{ evaluateIntNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[1],
					tick,
					spawnList
				) };

				float speed{ baseVel.getMagnitude() };
				wasp::math::Angle angle{ baseVel.getAngle() };

				float angleIncrement{ wasp::math::fullAngleDivide(symmetry) };

				for (int i{ 0 }; i < symmetry; ++i) {
					auto velConsumerSharedPointer{
						currentSpawnNodePointer->linkedNodePointers[2]
					};
					while (velConsumerSharedPointer) {
						runSpawnNodePassingVel(
							scene,
							entityID,
							velConsumerSharedPointer,
							tick,
							spawnList,
							Velocity{ speed, angle }
						);
					}
					angle += angleIncrement;
				}

				currentSpawnNodePointer = nullptr;
				break;
			}
			case SpawnInstructions::columnFormation: {
				Velocity baseVel{ evaluateVelocityNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[0],
					tick,
					spawnList
				) };
				int count{ evaluateIntNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[1],
					tick,
					spawnList
				) };
				float increment{ evaluateFloatNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[2],
					tick,
					spawnList
				) };

				Velocity velocity{ baseVel };
				for (int i{ 0 }; i < count; ++i) {
					auto velConsumerSharedPointer{
						currentSpawnNodePointer->linkedNodePointers[3]
					};
					while (velConsumerSharedPointer) {
						runSpawnNodePassingVel(
							scene,
							entityID,
							velConsumerSharedPointer,
							tick,
							spawnList,
							velocity
						);
					}
					velocity.setMagnitude(velocity.getMagnitude() + increment);
				}
				
				currentSpawnNodePointer = nullptr;
				break;
			}
			default:
				throw std::runtime_error{ "unknown spawn instruction" };
		}
	}

	void SpawnSystem::runSpawnNodePassingPos(
		NODE_HANDLER_ARGS, 
		const wasp::math::Point2& pos
	) {
		switch (currentSpawnNodePointer->spawnInstruction) {
			case SpawnInstructions::error:
				//throw an error
				throw std::runtime_error{ "spawn program error instruction" };
			case SpawnInstructions::spawnPos: {
				//cast the node data to a ComponentTupleSharedPtr
				const auto& [componentTupleBaseSharedPtr] =
					dynamic_cast<
					const SpawnNodeData<
					std::shared_ptr<ComponentTupleBase>
					>*
					>(
						currentSpawnNodePointer.get()
						)->data;

				//add position
				spawnList.emplace_back(
					componentTupleBaseSharedPtr->addPosition(pos)
				);

				currentSpawnNodePointer = nullptr;
				break;
			}
			case SpawnInstructions::ringFormation: {
				//get the base velocity
				Velocity baseVel{ evaluateVelocityNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[0],
					tick,
					spawnList
				) };
				int symmetry{ evaluateIntNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[1],
					tick,
					spawnList
				) };

				float speed{ baseVel.getMagnitude() };
				wasp::math::Angle angle{ baseVel.getAngle() };

				float angleIncrement{ wasp::math::fullAngleDivide(symmetry) };

				for (int i{ 0 }; i < symmetry; ++i) {
					auto velConsumerSharedPointer{
						currentSpawnNodePointer->linkedNodePointers[2]
					};
					while (velConsumerSharedPointer) {
						runSpawnNodePassingPosVel(
							scene,
							entityID,
							velConsumerSharedPointer,
							tick,
							spawnList,
							pos,
							Velocity{ speed, angle }
						);
					}
					angle += angleIncrement;
				}

				currentSpawnNodePointer = nullptr;
				break;
			}
			default:
				throw std::runtime_error{ "cannot pass pos!" };
		}
	}

	void SpawnSystem::runSpawnNodePassingVel(NODE_HANDLER_ARGS, const Velocity& vel) {
		switch (currentSpawnNodePointer->spawnInstruction) {
			case SpawnInstructions::error:
				//throw an error
				throw std::runtime_error{ "spawn program error instruction" };
			case SpawnInstructions::spawnPosVel: {
				//cast the node data to a ComponentTupleSharedPtr
				const auto& [componentTupleBaseSharedPtr] =
					dynamic_cast<
						const SpawnNodeData<
							std::shared_ptr<ComponentTupleBase>
						>*
					>(
						currentSpawnNodePointer.get()
					)->data;

				//evaluate pos node
				wasp::math::Point2 pos{ evaluatePointNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[0],
					tick,
					spawnList
				) };

				//add position and velocity
				spawnList.emplace_back(
					componentTupleBaseSharedPtr->addPosVel(pos, vel)
				);

				currentSpawnNodePointer = nullptr;
				break;
			}
			case SpawnInstructions::add: {
				Velocity toAdd{ evaluateVelocityNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[0],
					tick,
					spawnList
				) };
				Velocity toPass{ toAdd + vel };

				auto posConsumerSharedPointer{
					currentSpawnNodePointer->linkedNodePointers[1]
				};
				while (posConsumerSharedPointer) {
					runSpawnNodePassingVel(
						scene,
						entityID,
						posConsumerSharedPointer,
						tick,
						spawnList,
						toPass
					);
				}

				currentSpawnNodePointer = nullptr;
				break;
			}
			case SpawnInstructions::entityOffset: {
				wasp::math::Point2 basePos{
					scene.getDataStorage().getComponent<Position>(entityID)
				};
				wasp::math::Point2 pos{ basePos + vel };

				auto posConsumerSharedPointer{
					currentSpawnNodePointer->linkedNodePointers[0]
				};
				while (posConsumerSharedPointer) {
					runSpawnNodePassingPos(
						scene,
						entityID,
						posConsumerSharedPointer,
						tick,
						spawnList,
						pos
					);
				}

				currentSpawnNodePointer = nullptr;
				break;
			}
			case SpawnInstructions::columnFormation: {
				int count{ evaluateIntNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[0],
					tick,
					spawnList
				) };
				float increment{ evaluateFloatNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[1],
					tick,
					spawnList
				) };

				Velocity velocity{ vel };
				for (int i{ 0 }; i < count; ++i) {
					auto velConsumerSharedPointer{
						currentSpawnNodePointer->linkedNodePointers[2]
					};
					while (velConsumerSharedPointer) {
						runSpawnNodePassingVel(
							scene,
							entityID,
							velConsumerSharedPointer,
							tick,
							spawnList,
							velocity
						);
					}
					velocity.setMagnitude(velocity.getMagnitude() + increment);
				}

				currentSpawnNodePointer = nullptr;
				break;
			}
			case SpawnInstructions::mirrorFormation: {
				//get the base position
				wasp::math::Point2 basePos{ evaluatePointNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[0],
					tick,
					spawnList
				) };
				//get the axis of symmetry (x coordinate to reflect around)
				float axis{ evaluateFloatNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[1],
					tick,
					spawnList
				) };

				//in point slope form, our new X n follows the equation
				// n - a = -(o - a)
				// where a is our axis and o is the old X
				wasp::math::Point2 mirrorPos{ -basePos.x + (2 * axis), basePos.y };
				Velocity mirrorVel{
					vel.getMagnitude(),
					vel.getAngle().flipY()
				};

				//pass both normal and reflected pos/vel pairs
				auto posVelConsumerSharedPointer{
					currentSpawnNodePointer->linkedNodePointers[2]
				};
				while (posVelConsumerSharedPointer) {
					runSpawnNodePassingPosVel(
						scene,
						entityID,
						posVelConsumerSharedPointer,
						tick,
						spawnList,
						basePos,
						vel
					);
				}
				posVelConsumerSharedPointer =
					currentSpawnNodePointer->linkedNodePointers[2];
				while (posVelConsumerSharedPointer) {
					runSpawnNodePassingPosVel(
						scene,
						entityID,
						posVelConsumerSharedPointer,
						tick,
						spawnList,
						mirrorPos,
						mirrorVel
					);
				}

				currentSpawnNodePointer = nullptr;
				break;
			}
			case SpawnInstructions::arcFormation: {
				int symmetry{ evaluateIntNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[0],
					tick,
					spawnList
				) };
				float angleIncrement{ evaluateFloatNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[1],
					tick,
					spawnList
				) };
				float speed{ vel.getMagnitude() };
				wasp::math::Angle baseAngle{ vel.getAngle() };
				wasp::math::Angle angle{
					static_cast<float>(baseAngle) -
					((symmetry - 1) * angleIncrement / 2.0f)
				};
				for (int i{ 0 }; i < symmetry; ++i) {
					auto velConsumerSharedPointer{
						currentSpawnNodePointer->linkedNodePointers[2]
					};
					while (velConsumerSharedPointer) {
						runSpawnNodePassingVel(
							scene,
							entityID,
							velConsumerSharedPointer,
							tick,
							spawnList,
							Velocity{ speed, angle }
						);
					}
					angle += angleIncrement;
				}

				currentSpawnNodePointer = nullptr;
				break;
			}
			case SpawnInstructions::ringFormation: {
				int symmetry{ evaluateIntNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[0],
					tick,
					spawnList
				) };

				float speed{ vel.getMagnitude() };
				wasp::math::Angle angle{ vel.getAngle() };

				float angleIncrement{ wasp::math::fullAngleDivide(symmetry) };

				for (int i{ 0 }; i < symmetry; ++i) {
					auto velConsumerSharedPointer{
						currentSpawnNodePointer->linkedNodePointers[1]
					};
					while (velConsumerSharedPointer) {
						runSpawnNodePassingVel(
							scene,
							entityID,
							velConsumerSharedPointer,
							tick,
							spawnList,
							Velocity{ speed, angle }
						);
					}
					angle += angleIncrement;
				}

				currentSpawnNodePointer = nullptr;
				break;
			}

			default:
				throw std::runtime_error{ "cannot pass pos!" };
		}
	}

	void SpawnSystem::runSpawnNodePassingPosVel(
		NODE_HANDLER_ARGS,
		const wasp::math::Point2& pos,
		const Velocity& vel
	) {
		switch (currentSpawnNodePointer->spawnInstruction) {
			case SpawnInstructions::error:
				//throw an error
				throw std::runtime_error{ "spawn program error instruction" };
			case SpawnInstructions::spawnPosVel: {
				//cast the node data to a ComponentTupleSharedPtr
				const auto& [componentTupleBaseSharedPtr] =
					dynamic_cast<
					const SpawnNodeData<
					std::shared_ptr<ComponentTupleBase>
					>*
					>(
						currentSpawnNodePointer.get()
						)->data;
				//add position and velocity
				spawnList.emplace_back(
					componentTupleBaseSharedPtr->addPosVel(pos, vel)
				);

				currentSpawnNodePointer = nullptr;
				break;
			}
			case SpawnInstructions::mirrorFormation: {
				//get the axis of symmetry (x coordinate to reflect around)
				float axis{ evaluateFloatNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[0],
					tick,
					spawnList
				) };

				//in point slope form, our new X n follows the equation
				// n - a = -(o - a)
				// where a is our axis and o is the old X
				wasp::math::Point2 mirrorPos{ -pos.x + (2 * axis), pos.y };
				Velocity mirrorVel{
					vel.getMagnitude(),
					vel.getAngle().flipY()
				};

				//pass both normal and reflected pos/vel pairs
				auto posVelConsumerSharedPointer{
					currentSpawnNodePointer->linkedNodePointers[1]
				};
				while (posVelConsumerSharedPointer) {
					runSpawnNodePassingPosVel(
						scene,
						entityID,
						posVelConsumerSharedPointer,
						tick,
						spawnList,
						pos,
						vel
					);
				}
				posVelConsumerSharedPointer =
					currentSpawnNodePointer->linkedNodePointers[1];
				while (posVelConsumerSharedPointer) {
					runSpawnNodePassingPosVel(
						scene,
						entityID,
						posVelConsumerSharedPointer,
						tick,
						spawnList,
						mirrorPos,
						mirrorVel
					);
				}

				currentSpawnNodePointer = nullptr;
				break;
			}
			default:
				throw std::runtime_error{ "cannot pass pos and vel!" };
		}
	}

	int SpawnSystem::evaluateIntNode(NODE_HANDLER_ARGS) {
		switch (currentSpawnNodePointer->spawnInstruction) {
			case SpawnInstructions::value: {
				const auto [value] =
					dynamic_cast<const SpawnNodeData<int>*>(
						currentSpawnNodePointer.get()
						)->data;
				return value;
			}
			case SpawnInstructions::valueDifficulty: {
				const auto& [valueArray] =
					dynamic_cast<
					const SpawnNodeData<std::array<int, 4>>*
					>(
						currentSpawnNodePointer.get()
						)->data;
				return valueArray[static_cast<int>(getDifficulty())];
			}
			case SpawnInstructions::add: {
				int a{ evaluateIntNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[0],
					tick,
					spawnList
				) };
				int b{ evaluateIntNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[1],
					tick,
					spawnList
				) };
				return a + b;
			}
			case SpawnInstructions::entityUniformRandom: {
				int min{ evaluateIntNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[0],
					tick,
					spawnList
				) };
				int max{ evaluateIntNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[1],
					tick,
					spawnList
				) };

				//make a prng based on entityID
				config::PrngType prng{ entityID };
				std::uniform_int_distribution<int> distribution{ min, max };

				//roll it a couple times to make it more random
				#pragma warning(suppress : 4834)	//suppress no discard
				distribution(prng);
				#pragma warning(suppress : 4834)	//suppress no discard
				distribution(prng);
				return distribution(prng);
			}
			default:
				throw std::runtime_error{ "not an int instruction!" };
		}
	}

	float SpawnSystem::evaluateFloatNode(NODE_HANDLER_ARGS) {
		switch (currentSpawnNodePointer->spawnInstruction) {
			case SpawnInstructions::value: {
				const auto [value] =
					dynamic_cast<const SpawnNodeData<float>*>(
						currentSpawnNodePointer.get()
					)->data;
				return value;
			}
			case SpawnInstructions::valueDifficulty: {
				const auto& [valueArray] =
					dynamic_cast<
					const SpawnNodeData<std::array<float, 4>>*
					>(
						currentSpawnNodePointer.get()
						)->data;
				return valueArray[static_cast<int>(getDifficulty())];
			}
			case SpawnInstructions::add: {
				float a{ evaluateFloatNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[0],
					tick,
					spawnList
				) };
				float b{ evaluateFloatNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[1],
					tick,
					spawnList
				) };
				return a + b;
			}
			case SpawnInstructions::max: {
				float a{ evaluateFloatNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[0],
					tick,
					spawnList
				) };
				float b{ evaluateFloatNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[1],
					tick,
					spawnList
				) };
				return std::max(a, b);
			}
			case SpawnInstructions::entityX: {
				wasp::math::Point2 pos{
					scene.getDataStorage().getComponent<Position>(entityID)
				};
				return pos.x;
			}
			case SpawnInstructions::entityY: {
				wasp::math::Point2 pos{
					scene.getDataStorage().getComponent<Position>(entityID)
				};
				return pos.y;
			}
			case SpawnInstructions::entityAngle: {
				const Velocity& vel{
					scene.getDataStorage().getComponent<Velocity>(entityID)
				};
				return vel.getAngle();
			}
			case SpawnInstructions::uniformRandom: {
				float min{ evaluateFloatNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[0],
					tick,
					spawnList
				) };
				float max{ evaluateFloatNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[1],
					tick,
					spawnList
				) };
				auto& prng{ scene.getChannel(SceneTopics::random).getMessages()[0] };
				std::uniform_real_distribution<float> distribution{ min, max };
				return distribution(prng);
			}
			case SpawnInstructions::entityUniformRandom: {
				float min{ evaluateFloatNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[0],
					tick,
					spawnList
				) };
				float max{ evaluateFloatNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[1],
					tick,
					spawnList
				) };

				//make a prng based on entityID
				config::PrngType prng{ entityID };
				std::uniform_real_distribution<float> distribution{ min, max };

				//roll it a couple times to make it more random
				#pragma warning(suppress : 4834)	//suppress no discard
				distribution(prng);
				#pragma warning(suppress : 4834)	//suppress no discard
				distribution(prng);
				return distribution(prng);
			}
			case SpawnInstructions::pickupInitSpeed: {
				wasp::math::Point2 pos{
					scene.getDataStorage().getComponent<Position>(entityID)
				};

				//higher = lower speed
				float heightRatio{
					(pos.y - config::gameOffset.y) / config::gameHeight
				};
				float speedMulti{ 1 + (heightRatio * config::pickupInitSpeedMulti) };
				return config::pickupInitSpeedBase * speedMulti;
			}
			case SpawnInstructions::angleToPlayer: {
				wasp::math::Point2 pos{
					scene.getDataStorage().getComponent<Position>(entityID)
				};

				//get the iterator for players
				static const Topic<wasp::ecs::component::Group*>
					playerGroupPointerStorageTopic{};

				auto playerGroupPointer{
					getGroupPointer<PlayerData, Position>(
						scene,
						playerGroupPointerStorageTopic
					)
				};
				auto playerGroupIterator{
					playerGroupPointer->groupIterator<Position>()
				};

				if (playerGroupIterator.isValid()) {
					//just grab the first player
					const auto [playerPos] = *playerGroupIterator;
					return wasp::math::getAngleFromAToB(pos, playerPos);
				}
				else {
					return 0.0f;
				}
			}
			case SpawnInstructions::spiral: {
				int maxTick{ evaluateIntNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[0],
					tick,
					spawnList
				) };
				float baseAngle{ std::fmod(evaluateFloatNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[1],
					tick,
					spawnList
				), 360.0f) };
				float angularVelocity{ std::fmod(evaluateFloatNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[2],
					tick,
					spawnList
				), 360.0f) };
				int tickFromZero{ maxTick - tick };
				return baseAngle + (static_cast<float>(tickFromZero) * angularVelocity);
			}
			case SpawnInstructions::whip: {
				int maxTick{ evaluateIntNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[0],
					tick,
					spawnList
				) };
				float speedLow{ evaluateFloatNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[1],
					tick,
					spawnList
				) };
				float speedHigh{ evaluateFloatNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[2],
					tick,
					spawnList
				) };
				int tickFromZero{ maxTick - tick };
				float tickRatio{ static_cast<float>(tickFromZero) / maxTick };
				float speedDifference{ speedHigh - speedLow };
				return speedLow + (tickRatio * speedDifference);
			}
			case SpawnInstructions::conditionElse: {
				//if our predicate is met, evaluate truenode
				if (evaluatePredicateNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[0],
					tick,
					spawnList
				)) {
					return evaluateFloatNode(
						scene,
						entityID,
						currentSpawnNodePointer->linkedNodePointers[1],
						tick,
						spawnList
					);
				}
				//otherwise, evaluate falseNode
				else {
					return evaluateFloatNode(
						scene,
						entityID,
						currentSpawnNodePointer->linkedNodePointers[2],
						tick,
						spawnList
					);
				}
				break;
			}
			default:
				throw std::runtime_error{ "not a float instruction!" };
		}
	}

	bool SpawnSystem::evaluatePredicateNode(NODE_HANDLER_ARGS) {
		switch (currentSpawnNodePointer->spawnInstruction) {
			case SpawnInstructions::tickMod: {
				//returns (tick + int1) % int2 == 0
				int add = evaluateIntNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[0],
					tick,
					spawnList
				);
				int mod = evaluateIntNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[1],
					tick,
					spawnList
				);
				return (tick + add) % mod == 0;
			}
			case SpawnInstructions::isLastTick: {
				//returns true if this is the last tick of the SpawnProgram
				return tick == 1;
			}
			case SpawnInstructions::isPlayerFocused: {
				//returns true if the focus gameCommand is found
				const auto& gameCommandChannel{
					scene.getChannel(SceneTopics::gameCommands)
				};
				for (GameCommands gameCommand : gameCommandChannel.getMessages()) {
					if (gameCommand == GameCommands::focus) {
						return true;
					}
				}
				return false;
			}
			case SpawnInstructions::isDifficulty: {
				const auto [difficulty] =
					dynamic_cast<const SpawnNodeData<Difficulty>*>(
						currentSpawnNodePointer.get()
					)->data;
				return static_cast<int>(getDifficulty()) >= static_cast<int>(difficulty);
			}
			case SpawnInstructions::chance: {
				float chance{ evaluateFloatNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[0],
					tick,
					spawnList
				) };
				auto& prng{ scene.getChannel(SceneTopics::random).getMessages()[0] };
				static std::uniform_real_distribution<float> distribution{ 0.0f, 1.0f };
				return distribution(prng) < chance;
			}
			default:
				throw std::runtime_error{ "not a predicate instruction!" };
		}
	}

	Velocity SpawnSystem::evaluateVelocityNode(NODE_HANDLER_ARGS) {
		switch (currentSpawnNodePointer->spawnInstruction) {
			case SpawnInstructions::value: {
				const auto [value] =
					dynamic_cast<const SpawnNodeData<Velocity>*>(
						currentSpawnNodePointer.get()
						)->data;
				return value;
			}
			case SpawnInstructions::valueDifficulty: {
				const auto& [valueArray] =
					dynamic_cast<
					const SpawnNodeData<std::array<Velocity, 4>>*
					>(
						currentSpawnNodePointer.get()
						)->data;
				return valueArray[static_cast<int>(getDifficulty())];
			}
			case SpawnInstructions::add: {
				Velocity a{ evaluateVelocityNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[0],
					tick,
					spawnList
				) };
				Velocity b{ evaluateVelocityNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[1],
					tick,
					spawnList
				) };
				return a + b;
			}
			case SpawnInstructions::multiply: {
				Velocity a{ evaluateVelocityNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[0],
					tick,
					spawnList
				) };
				float b{ evaluateFloatNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[1],
					tick,
					spawnList
				) };
				return a * b;
			}
			case SpawnInstructions::velocityFromPolar: {
				float magnitude{ evaluateFloatNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[0],
					tick,
					spawnList
				) };
				float angle{ evaluateFloatNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[1],
					tick,
					spawnList
				) };
				return Velocity{ magnitude, angle };
			}
			case SpawnInstructions::velocityToPoint: {
				wasp::math::Point2 a{ evaluatePointNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[0],
					tick,
					spawnList
				) };
				wasp::math::Point2 b{ evaluatePointNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[1],
					tick,
					spawnList
				) };
				return wasp::math::vectorFromAToB(a, b);
			}
			case SpawnInstructions::conditionElse: {
				//if our predicate is met, evaluate truenode
				if (evaluatePredicateNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[0],
					tick,
					spawnList
				)) {
					return evaluateVelocityNode(
						scene,
						entityID,
						currentSpawnNodePointer->linkedNodePointers[1],
						tick,
						spawnList
					);
				}
				//otherwise, evaluate falseNode
				else {
					return evaluateVelocityNode(
						scene,
						entityID,
						currentSpawnNodePointer->linkedNodePointers[2],
						tick,
						spawnList
					);
				}
				break;
			}
			default:
				throw std::runtime_error{ "not a velocity instruction!" };
		}
	}

	SpawnSystem::Point2 SpawnSystem::evaluatePointNode(NODE_HANDLER_ARGS) {
		switch (currentSpawnNodePointer->spawnInstruction) {
			case SpawnInstructions::value: {
				const auto [value] =
					dynamic_cast<const SpawnNodeData<wasp::math::Point2>*>(
						currentSpawnNodePointer.get()
						)->data;
				return value;
			}
			case SpawnInstructions::valueDifficulty: {
				const auto& [valueArray] =
					dynamic_cast<
					const SpawnNodeData<std::array<wasp::math::Point2, 4>>*
					>(
						currentSpawnNodePointer.get()
						)->data;
				return valueArray[static_cast<int>(getDifficulty())];
			}
			case SpawnInstructions::entityPosition: {
				return scene.getDataStorage().getComponent<Position>(entityID);
			}
			case SpawnInstructions::entityOffset: {
				wasp::math::Point2 basePos{
					scene.getDataStorage().getComponent<Position>(entityID)
				};
				Velocity offset{ evaluateVelocityNode(
					scene,
					entityID,
					currentSpawnNodePointer->linkedNodePointers[0],
					tick,
					spawnList
				) };
				return basePos + offset;
			}
			case SpawnInstructions::pointFromFloats: {
				float x{ evaluateFloatNode(
						scene,
						entityID,
						currentSpawnNodePointer->linkedNodePointers[0],
						tick,
						spawnList
				) };
				float y{ evaluateFloatNode(
						scene,
						entityID,
						currentSpawnNodePointer->linkedNodePointers[1],
						tick,
						spawnList
				) };
				return wasp::math::Point2{ x, y };
			}
			default:
				throw std::runtime_error{ "not a point instruction!" };
		}
	}

	Difficulty SpawnSystem::getDifficulty() {
		return globalChannelSetPointer
			->getChannel(GlobalTopics::gameState).getMessages()[0].difficulty;
	}
}