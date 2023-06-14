#include "Game/Systems/ScriptSystem.h"

#include "Logging.h"

namespace process::game::systems {

	using namespace wasp::ecs;
	using namespace wasp::ecs::entity;

	using ScriptInstructions = wasp::game::components::ScriptInstructions;
	using ScriptNode = wasp::game::components::ScriptNode;
	using ScriptProgram = wasp::game::components::ScriptProgram;
	template <typename Internal, typename External>
	using ScriptNodeData = wasp::game::components::ScriptNodeData<Internal, External>;
	
	using SpawnProgram = wasp::game::components::SpawnProgram;

	namespace {

		constexpr float angleEquivalenceEpsilon{ .05f };
		constexpr float pointEquivalenceEpsilon{ .5f };

		constexpr float gotoDeceleratingExponentBase{ 2.0f };
		constexpr float gotoDeceleratingHorizontalShift{ 0.1f };
		constexpr float gotoDeceleratingHorizontalStretch{ 7.0f };

		float calculateGotoDeceleratingSpeed(
			float currentDistance,
			float initDistance,
			float maxSpeed
		) {
			float distanceRatio{ currentDistance / initDistance };
			float exponent{			//a linear function based on distanceRatio; mx+b
				(gotoDeceleratingHorizontalStretch * distanceRatio) +
				gotoDeceleratingHorizontalShift
			};
			float speedMulti{
				1.0f - (1.0f / std::powf(gotoDeceleratingExponentBase, exponent))
			};
			return std::fminf(maxSpeed * speedMulti, currentDistance);
		}

		void clearExternalDataForNode(
			std::shared_ptr<ScriptNode>& currentScriptNodePointer,
			std::unordered_map<ScriptNode*, void*>& externalData
		) {
			if (externalData.find(currentScriptNodePointer.get())
				!= externalData.end()
			) {
				currentScriptNodePointer->clearData(
					externalData[currentScriptNodePointer.get()]
				);
				externalData.erase(currentScriptNodePointer.get());
			}
		}

		//goes to the node specified by nextIndex if it exists
		void gotoNextNode(
			std::shared_ptr<ScriptNode>& currentScriptNodePointer,
			std::size_t nextIndex
		) {
			if (currentScriptNodePointer->linkedNodePointers.size() > nextIndex) {
				currentScriptNodePointer
					= currentScriptNodePointer->linkedNodePointers[nextIndex];
			}
			else {
				currentScriptNodePointer = nullptr;
			}
		}

		#define NODE_HANDLER_ARGS \
			Scene& scene, \
			EntityID entityID, \
			std::shared_ptr<ScriptNode>& currentScriptNodePointer, \
			std::unordered_map<ScriptNode*, void*>& externalData, \
			ComponentOrderQueue& componentOrderQueue
	}

	ScriptSystem::ScriptSystem(wasp::channel::ChannelSet* globalChannelSetPointer)
		: globalChannelSetPointer{ globalChannelSetPointer } {
	}

	void ScriptSystem::operator()(Scene& scene) {
		//create our component order queue
		ComponentOrderQueue componentOrderQueue{};

		//get the group iterator for ScriptProgramList
		static const Topic<wasp::ecs::component::Group*> groupPointerStorageTopic{};
		auto groupPointer{
			getGroupPointer<ScriptProgramList>(
				scene,
				groupPointerStorageTopic
			)
		};
		auto groupIterator{ groupPointer->groupIterator<ScriptProgramList>() };

		//populate our component order queue with every component order this tick
		while (groupIterator.isValid()) {
			auto [scriptProgramList] = *groupIterator;
			EntityID entityID{ groupIterator.getEntityID() };
			runScriptProgramList(
				scene, 
				scriptProgramList, 
				entityID, 
				componentOrderQueue
			);
			++groupIterator;
		}

		//apply all our queued component orders to the ecs world
		componentOrderQueue.apply(scene.getDataStorage());
	}

	void ScriptSystem::runScriptProgramList(
		Scene& scene,
		ScriptProgramList& scriptProgramList,
		EntityID entityID,
		ComponentOrderQueue& componentOrderQueue
	) {
		for (auto itr{ scriptProgramList.begin() }; itr != scriptProgramList.end();) {
			auto& scriptProgram{ *itr };
			while (scriptProgram.currentNodePointer) {
				if (!runScriptNode(
					scene,
					entityID,
					scriptProgram.currentNodePointer,
					scriptProgram.externalData,
					componentOrderQueue
				)) {
					break;
				}
			}
			if (!scriptProgram.currentNodePointer) {
				itr = scriptProgramList.erase(itr);
			}
			else {
				++itr;
			}
		}
	}

	bool ScriptSystem::runScriptNode(NODE_HANDLER_ARGS) {
		wasp::debug::log(
			std::to_string(static_cast<int>(
				currentScriptNodePointer->scriptInstruction))
		);
		switch (currentScriptNodePointer->scriptInstruction) {
			case ScriptInstructions::error:
				throw std::runtime_error{ "error script instruction" };
			case ScriptInstructions::condition: {
				if (evaluatePredicateNode(
					scene,
					entityID,
					currentScriptNodePointer->linkedNodePointers[0],
					externalData,
					componentOrderQueue
				)) {
					currentScriptNodePointer
						= currentScriptNodePointer->linkedNodePointers[1];
					return true;
				}
				//this instruction ends the whole script segment if false
				else {
					currentScriptNodePointer = nullptr;
					return false;
				}
			}
			case ScriptInstructions::stallCondition: {
				if (evaluatePredicateNode(
					scene,
					entityID,
					currentScriptNodePointer->linkedNodePointers[0],
					externalData,
					componentOrderQueue
				)) {
					currentScriptNodePointer
						= currentScriptNodePointer->linkedNodePointers[1];
					return true;
				}
				//this instruction stalls until predicate is true
				else {
					return false;
				}
			}
			case ScriptInstructions::conditionElse: {
				if (evaluatePredicateNode(
					scene,
					entityID,
					currentScriptNodePointer->linkedNodePointers[0],
					externalData,
					componentOrderQueue
				)) {
					currentScriptNodePointer
						= currentScriptNodePointer->linkedNodePointers[1];
				}
				//this instruction either continues with trueNode or falseNode
				else {
					currentScriptNodePointer
						= currentScriptNodePointer->linkedNodePointers[2];
				}
				return true;
			}
			case ScriptInstructions::routine: {
				auto& scriptBaseNodePointer{
					currentScriptNodePointer->linkedNodePointers[0]
				};
				if (currentScriptNodePointer->linkedNodePointers.size() > 1) {
					//there is a predicate
					auto& predicateNodePointer{
						currentScriptNodePointer->linkedNodePointers[1]
					};
					if (predicateNodePointer && evaluatePredicateNode(
						scene,
						entityID,
						predicateNodePointer,
						externalData,
						componentOrderQueue
					)) {
						//if the predicate is true, we need to move on
						clearExternalDataForNode(
							currentScriptNodePointer,
							externalData
						);
						gotoNextNode(currentScriptNodePointer, 2);
						return true;
					}
				}
				//grab our stored script program
				auto dataNodePointer{
					dynamic_cast<ScriptNodeData<wasp::utility::Void, ScriptProgram>*>(
						currentScriptNodePointer.get()
					)
				};
				if (externalData.find(currentScriptNodePointer.get())
					== externalData.end()
					) {
					//initialize our temporary ScriptProgram if necessary
					externalData[currentScriptNodePointer.get()]
						= new ScriptProgram{ scriptBaseNodePointer };
				}

				auto& scriptProgram{
					*dataNodePointer->getDataPointer(
						externalData[currentScriptNodePointer.get()]
					)
				};
				//run our routine
				while (scriptProgram.currentNodePointer) {
					if (!runScriptNode(
						scene,
						entityID,
						scriptProgram.currentNodePointer,
						scriptProgram.externalData,
						componentOrderQueue
					)) {
						break;
					}
				}
				if (!scriptProgram.currentNodePointer) {
					//our routine is over
					clearExternalDataForNode(
						currentScriptNodePointer,
						externalData
					);
					gotoNextNode(currentScriptNodePointer, 2);
					return true;
				}
				else {
					//our routine is not over, just stalled
					return false;
				}
			}
			case ScriptInstructions::timer: {
				auto dataNodePointer{
					dynamic_cast<ScriptNodeData<int, wasp::utility::Ticker>*>(
						currentScriptNodePointer.get()
					)
				};

				if (externalData.find(currentScriptNodePointer.get())
					== externalData.end()
					) {
					int maxTick{ dataNodePointer->internalData };
					externalData[currentScriptNodePointer.get()]
						= new wasp::utility::Ticker{ maxTick, false };
				}
				wasp::utility::Ticker& ticker{
					*dataNodePointer->getDataPointer(
						externalData[currentScriptNodePointer.get()]
					)
				};
				//move on to next node if present
				if (ticker.stepAndGetTick() <= 0) {
					clearExternalDataForNode(currentScriptNodePointer, externalData);
					gotoNextNode(currentScriptNodePointer, 0);
					return true;
				}
				else {
					return false;
				}
			}
			case ScriptInstructions::removeVisible: {
				EntityHandle entityHandle{
					scene.getDataStorage().makeHandle(entityID)
				};
				componentOrderQueue.queueRemoveComponent<VisibleMarker>(
					entityHandle
					);
				gotoNextNode(currentScriptNodePointer, 0);
				return true;
			}
			case ScriptInstructions::setSpriteInstruction: {
				auto dataNodePointer{
					dynamic_cast<ScriptNodeData<SpriteInstruction, wasp::utility::Void>*>(
						currentScriptNodePointer.get()
					)
				};
				EntityHandle entityHandle{
					scene.getDataStorage().makeHandle(entityID)
				};
				componentOrderQueue.queueSetComponent<SpriteInstruction>(
					entityHandle,
					dataNodePointer->internalData
					);
				gotoNextNode(currentScriptNodePointer, 0);
				return true;
			}
			case ScriptInstructions::setCollidable: {
				EntityHandle entityHandle{
					scene.getDataStorage().makeHandle(entityID)
				};
				componentOrderQueue.queueSetComponent<CollidableMarker>(
					entityHandle, {}
				);
				gotoNextNode(currentScriptNodePointer, 0);
				return true;
			}
			case ScriptInstructions::removeCollidable: {
				EntityHandle entityHandle{
					scene.getDataStorage().makeHandle(entityID)
				};
				componentOrderQueue.queueRemoveComponent<CollidableMarker>(
					entityHandle
					);
				gotoNextNode(currentScriptNodePointer, 0);
				return true;
			}
			case ScriptInstructions::setHealth: {
				auto dataNodePointer{
					dynamic_cast<ScriptNodeData<int, wasp::utility::Void>*>(
						currentScriptNodePointer.get()
					)
				};
				EntityHandle entityHandle{
					scene.getDataStorage().makeHandle(entityID)
				};
				componentOrderQueue.queueSetComponent<Health>(
					entityHandle, { dataNodePointer->internalData }
				);
				gotoNextNode(currentScriptNodePointer, 0);
				return true;
			}
			case ScriptInstructions::removeHealth: {
				EntityHandle entityHandle{
					scene.getDataStorage().makeHandle(entityID)
				};
				componentOrderQueue.queueRemoveComponent<Health>(
					entityHandle
					);
				gotoNextNode(currentScriptNodePointer, 0);
				return true;
			}
			case ScriptInstructions::setDamage: {
				auto dataNodePointer{
					dynamic_cast<ScriptNodeData<int, wasp::utility::Void>*>(
						currentScriptNodePointer.get()
					)
				};
				EntityHandle entityHandle{
					scene.getDataStorage().makeHandle(entityID)
				};
				componentOrderQueue.queueSetComponent<Damage>(
					entityHandle, { dataNodePointer->internalData }
				);
				gotoNextNode(currentScriptNodePointer, 0);
				return true;
			}
			case ScriptInstructions::removeDamage: {
				EntityHandle entityHandle{
					scene.getDataStorage().makeHandle(entityID)
				};
				componentOrderQueue.queueRemoveComponent<Damage>(
					entityHandle
					);
				gotoNextNode(currentScriptNodePointer, 0);
				return true;
			}
			case ScriptInstructions::addSpawn: {
				if (scene.getDataStorage()
					.containsComponent<SpawnProgramList>(entityID)
					) {
					auto dataNodePointer{
						dynamic_cast<
							ScriptNodeData<wasp::game::components::SpawnProgram,
											wasp::utility::Void>*
						>(
							currentScriptNodePointer.get()
						)
					};
					scene.getDataStorage().getComponent<SpawnProgramList>(entityID)
						.push_back(dataNodePointer->internalData);
					gotoNextNode(currentScriptNodePointer, 0);
					return true;
				}
				//else fallthrough to setSpawn
			}
			case ScriptInstructions::setSpawn: {
				auto dataNodePointer{
					dynamic_cast<
						ScriptNodeData<SpawnProgram, wasp::utility::Void>*
					>(
						currentScriptNodePointer.get()
					)
				};
				EntityHandle entityHandle{
					scene.getDataStorage().makeHandle(entityID)
				};
				componentOrderQueue.queueSetComponent<SpawnProgramList>(
					entityHandle, { dataNodePointer->internalData }
				);
				gotoNextNode(currentScriptNodePointer, 0);
				return true;
			}
			case ScriptInstructions::clearSpawn: {
				if (scene.getDataStorage()
					.containsComponent<SpawnProgramList>(entityID)
					) {
					scene.getDataStorage().getComponent<SpawnProgramList>(entityID)
						.clear();
				}
				gotoNextNode(currentScriptNodePointer, 0);
				return true;
			}
			case ScriptInstructions::setVelocity: {
				Velocity velocity{
					evaluateVelocityNode(
						scene,
						entityID,
						currentScriptNodePointer->linkedNodePointers[0],
						externalData,
						componentOrderQueue
					)
				};
				EntityHandle entityHandle{
					scene.getDataStorage().makeHandle(entityID)
				};
				componentOrderQueue.queueSetComponent<Velocity>(
					entityHandle,
					velocity
					);
				gotoNextNode(currentScriptNodePointer, 1);
				return true;
			}
			case ScriptInstructions::setVelocityToPlayer: {
				auto dataNodePointer{
					dynamic_cast<ScriptNodeData<float, wasp::utility::Void>*>(
						currentScriptNodePointer.get()
					)
				};
				float speed{ dataNodePointer->internalData };
				EntityHandle entityHandle{
					scene.getDataStorage().makeHandle(entityID)
				};

				wasp::math::Point2 pos{
					scene.getDataStorage().getComponent<Position>(entityHandle)
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

				float angle{ 0.0f };
				if (playerGroupIterator.isValid()) {
					//just grab the first player
					const auto [playerPos] = *playerGroupIterator;
					angle = wasp::math::getAngleFromAToB(pos, playerPos);
				}
				Velocity velocity{ speed, angle };
				componentOrderQueue.queueSetComponent<Velocity>(
					entityHandle,
					velocity
					);
				gotoNextNode(currentScriptNodePointer, 0);
				return true;
			}
			case ScriptInstructions::setInbound: {
				auto dataNodePointer{
					dynamic_cast<ScriptNodeData<float, wasp::utility::Void>*>(
						currentScriptNodePointer.get()
					)
				};
				float inbound{ dataNodePointer->internalData };
				EntityHandle entityHandle{
					scene.getDataStorage().makeHandle(entityID)
				};
				componentOrderQueue.queueSetComponent<Inbound>(
					entityHandle,
					Inbound{ inbound }
				);
				gotoNextNode(currentScriptNodePointer, 0);
				return true;
			}
			case ScriptInstructions::removeInbound: {
				EntityHandle entityHandle{
					scene.getDataStorage().makeHandle(entityID)
				};
				componentOrderQueue.queueRemoveComponent<Inbound>(
					entityHandle
					);
				gotoNextNode(currentScriptNodePointer, 0);
				return true;
			}
			case ScriptInstructions::shiftSpeedPeriod: {
				Velocity& velocity{
					scene.getDataStorage().getComponent<Velocity>(entityID)
				};
				float oldMagnitude{ velocity.getMagnitude() };

				auto dataNodePointer{
					dynamic_cast<ScriptNodeData<std::tuple<float, int>, float>*>(
						currentScriptNodePointer.get()
					)
				};

				const auto [targetSpeed, ticks] = dataNodePointer->internalData;

				bool hasReachedSpeed{ false };

				//if we are at speed already, move to next instruction
				if (oldMagnitude == targetSpeed) {
					hasReachedSpeed = true;
				}
				else {
					//retrieve or create increment
					float increment{};
					if (externalData.find(currentScriptNodePointer.get())
						!= externalData.end()
						) {
						increment = *dataNodePointer->getDataPointer(
							externalData[currentScriptNodePointer.get()]
						);
					}
					else {
						float magnitudeDiff{ targetSpeed - oldMagnitude };
						increment = ticks > 0
							? magnitudeDiff / static_cast<float>(ticks)
							: magnitudeDiff;
						externalData[currentScriptNodePointer.get()]
							= new float{ increment };
					}

					//increment velocity
					float newMagnitude{ oldMagnitude + increment };

					//check to see if we have reached target speed
					if (increment > 0 && newMagnitude > targetSpeed) {
						hasReachedSpeed = true;
						velocity.setMagnitude(targetSpeed);
					}
					else if (increment < 0 && newMagnitude < targetSpeed) {
						hasReachedSpeed = true;
						velocity.setMagnitude(targetSpeed);
					}
					else {
						velocity.setMagnitude(newMagnitude);
					}
				}
				//move on to next node if present
				if (hasReachedSpeed) {
					clearExternalDataForNode(currentScriptNodePointer, externalData);
					gotoNextNode(currentScriptNodePointer, 0);
					return true;
				}
				else {
					return false;
				}
			}
			case ScriptInstructions::shiftVelocityPeriod: {
				Velocity& velocity{
					scene.getDataStorage().getComponent<Velocity>(entityID)
				};
				float oldMagnitude{ velocity.getMagnitude() };

				auto dataNodePointer{
					dynamic_cast<ScriptNodeData<std::tuple<Velocity, int>, float>*>(
						currentScriptNodePointer.get()
					)
				};

				const auto [targetVelocity, ticks] = dataNodePointer->internalData;
				float targetSpeed{ targetVelocity.getMagnitude() };

				//do not turn; set our angle to target angle
				if (externalData.find(currentScriptNodePointer.get())
					== externalData.end()
					) {
					velocity.setAngle(targetVelocity.getAngle());
				}

				bool hasReachedSpeed{ false };

				//if we are at speed already, move to next instruction
				if (oldMagnitude == targetSpeed) {
					hasReachedSpeed = true;
				}
				else {
					//retrieve or create increment
					float increment{};
					if (externalData.find(currentScriptNodePointer.get())
						!= externalData.end()
						) {
						increment = *dataNodePointer->getDataPointer(
							externalData[currentScriptNodePointer.get()]
						);
					}
					else {
						float magnitudeDiff{ targetSpeed - oldMagnitude };
						increment = ticks > 0
							? magnitudeDiff / static_cast<float>(ticks)
							: magnitudeDiff;
						externalData[currentScriptNodePointer.get()]
							= new float{ increment };
					}

					//increment velocity
					float newMagnitude{ oldMagnitude + increment };

					//check to see if we have reached target speed
					if (increment > 0 && newMagnitude > targetSpeed) {
						hasReachedSpeed = true;
						velocity.setMagnitude(targetSpeed);
					}
					else if (increment < 0 && newMagnitude < targetSpeed) {
						hasReachedSpeed = true;
						velocity.setMagnitude(targetSpeed);
					}
					else {
						velocity.setMagnitude(newMagnitude);
					}
				}
				//move on to next node if present
				if (hasReachedSpeed) {
					clearExternalDataForNode(currentScriptNodePointer, externalData);
					gotoNextNode(currentScriptNodePointer, 0);
					return true;
				}
				else {
					return false;
				}
			}
			case ScriptInstructions::shiftVelocityTurnPeriod: {
				Velocity& velocity{
					scene.getDataStorage().getComponent<Velocity>(entityID)
				};
				float oldMagnitude{ velocity.getMagnitude() };
				auto oldAngle{ velocity.getAngle() };

				auto dataNodePointer{
					dynamic_cast<ScriptNodeData<
						std::tuple<Velocity, wasp::math::Angle, int>,
						std::tuple<float, float>
					>*>(
						currentScriptNodePointer.get()
					)
				};

				const auto [targetVelocity, initAngle, ticks]
					= dataNodePointer->internalData;
				const float targetSpeed{ targetVelocity.getMagnitude() };
				const auto targetAngle{ targetVelocity.getAngle() };

				bool hasReachedSpeed{ false };
				bool hasReachedAngle{ false };

				if (oldMagnitude == targetSpeed) {
					hasReachedSpeed = true;
				}
				if (oldAngle == targetAngle) {
					hasReachedAngle = true;
				}

				if (!hasReachedSpeed || !hasReachedAngle) {
					//retrieve or create increment
					float speedIncrement{};
					float angleIncrement{};
					if (externalData.find(currentScriptNodePointer.get())
						!= externalData.end()
						) {
						auto [storedSpeedIncrement, storedAngleIncrement]
							= *dataNodePointer->getDataPointer(
								externalData[currentScriptNodePointer.get()]
							);
						speedIncrement = storedSpeedIncrement;
						angleIncrement = storedAngleIncrement;
					}
					else {
						//make sure old angle and init angle are the same
						oldAngle = initAngle;
						//create and store increments
						float magnitudeDiff{ targetSpeed - oldMagnitude };
						speedIncrement = ticks > 0
							? magnitudeDiff / static_cast<float>(ticks)
							: magnitudeDiff;

						float angleDiff{ targetAngle.smallerDifference(initAngle) };
						angleIncrement = ticks > 0
							? angleDiff / static_cast<float>(ticks)
							: angleDiff;

						externalData[currentScriptNodePointer.get()]
							= new std::tuple<float, float>{
								speedIncrement,
								angleIncrement
						};
					}

					if (!hasReachedSpeed) {
						//increment speed
						float newMagnitude{ oldMagnitude + speedIncrement };

						//check to see if we have reached target speed
						if (speedIncrement > 0 && newMagnitude > targetSpeed) {
							hasReachedSpeed = true;
							velocity.setMagnitude(targetSpeed);
						}
						else if (speedIncrement < 0 && newMagnitude < targetSpeed) {
							speedIncrement = true;
							velocity.setMagnitude(targetSpeed);
						}
						else {
							velocity.setMagnitude(newMagnitude);
						}
					}

					if (!hasReachedAngle) {
						//increment angle
						wasp::math::Angle newAngle{
							static_cast<float>(oldAngle) + angleIncrement
						};

						//check to see if we have reached target angle
						float angleDiff{
							std::abs(static_cast<float>(
								targetAngle.smallerDifference(newAngle)
							))
						};
						if (angleDiff <= angleEquivalenceEpsilon) {
							hasReachedAngle = true;
							velocity.setAngle(targetAngle);
						}
						else {
							velocity.setAngle(newAngle);
						}
					}
				}
				//move on to next node if present
				if (hasReachedSpeed && hasReachedAngle) {
					clearExternalDataForNode(currentScriptNodePointer, externalData);
					gotoNextNode(currentScriptNodePointer, 0);
					return true;
				}
				else {
					return false;
				}
			}
			case ScriptInstructions::shiftVelocityTurnLongPeriod: {
				Velocity& velocity{
					scene.getDataStorage().getComponent<Velocity>(entityID)
				};
				float oldMagnitude{ velocity.getMagnitude() };
				auto oldAngle{ velocity.getAngle() };

				auto dataNodePointer{
					dynamic_cast<ScriptNodeData<
						std::tuple<Velocity, wasp::math::Angle, int>,
						std::tuple<float, float>
					>*>(
						currentScriptNodePointer.get()
					)
				};

				const auto [targetVelocity, initAngle, ticks]
					= dataNodePointer->internalData;
				const float targetSpeed{ targetVelocity.getMagnitude() };
				const auto targetAngle{ targetVelocity.getAngle() };

				bool hasReachedSpeed{ false };
				bool hasReachedAngle{ false };

				if (oldMagnitude == targetSpeed) {
					hasReachedSpeed = true;
				}
				if (oldAngle == targetAngle) {
					hasReachedAngle = true;
				}

				if (!hasReachedSpeed || !hasReachedAngle) {
					//retrieve or create increment
					float speedIncrement{};
					float angleIncrement{};
					if (externalData.find(currentScriptNodePointer.get())
						!= externalData.end()
						) {
						auto [storedSpeedIncrement, storedAngleIncrement]
							= *dataNodePointer->getDataPointer(
								externalData[currentScriptNodePointer.get()]
							);
						speedIncrement = storedSpeedIncrement;
						angleIncrement = storedAngleIncrement;
					}
					else {
						//make sure old angle and init angle are the same
						oldAngle = initAngle;
						//create and store increments
						float magnitudeDiff{ targetSpeed - oldMagnitude };
						speedIncrement = ticks > 0
							? magnitudeDiff / static_cast<float>(ticks)
							: magnitudeDiff;

						float angleDiff{ targetAngle.largerDifference(initAngle) };
						angleIncrement = ticks > 0
							? angleDiff / static_cast<float>(ticks)
							: angleDiff;

						externalData[currentScriptNodePointer.get()]
							= new std::tuple<float, float>{
								speedIncrement,
								angleIncrement
						};
					}

					if (!hasReachedSpeed) {
						//increment speed
						float newMagnitude{ oldMagnitude + speedIncrement };

						//check to see if we have reached target speed
						if (speedIncrement > 0 && newMagnitude > targetSpeed) {
							hasReachedSpeed = true;
							velocity.setMagnitude(targetSpeed);
						}
						else if (speedIncrement < 0 && newMagnitude < targetSpeed) {
							speedIncrement = true;
							velocity.setMagnitude(targetSpeed);
						}
						else {
							velocity.setMagnitude(newMagnitude);
						}
					}

					if (!hasReachedAngle) {
						//increment angle
						wasp::math::Angle newAngle{
							static_cast<float>(oldAngle) + angleIncrement
						};

						//check to see if we have reached target angle
						float angleDiff{
							std::abs(static_cast<float>(
								targetAngle.smallerDifference(newAngle)
							))
						};
						if (angleDiff <= angleEquivalenceEpsilon) {
							hasReachedAngle = true;
							velocity.setAngle(targetAngle);
						}
						else {
							velocity.setAngle(newAngle);
						}
					}
				}
				//move on to next node if present
				if (hasReachedSpeed && hasReachedAngle) {
					clearExternalDataForNode(currentScriptNodePointer, externalData);
					gotoNextNode(currentScriptNodePointer, 0);
					return true;
				}
				else {
					return false;
				}
			}
			case ScriptInstructions::shiftAnglePeriod: {
				Velocity& velocity{
					scene.getDataStorage().getComponent<Velocity>(entityID)
				};
				auto oldAngle{ velocity.getAngle() };

				auto dataNodePointer{
					dynamic_cast<ScriptNodeData<
						std::tuple<wasp::math::Angle, wasp::math::Angle, int>,
						float
					>*>(
						currentScriptNodePointer.get()
					)
				};

				const auto [targetAngle, initAngle, ticks]
					= dataNodePointer->internalData;

				bool hasReachedAngle{ false };

				if (oldAngle == targetAngle) {
					hasReachedAngle = true;
				}

				if (!hasReachedAngle) {
					//retrieve or create increment
					float angleIncrement{};
					if (externalData.find(currentScriptNodePointer.get())
						!= externalData.end()
						) {
						float storedAngleIncrement{
							*dataNodePointer->getDataPointer(
								externalData[currentScriptNodePointer.get()]
							)
						};
						angleIncrement = storedAngleIncrement;
					}
					else {
						//make sure old angle and init angle are the same
						oldAngle = initAngle;

						//create and store increments
						float angleDiff{ targetAngle.smallerDifference(initAngle) };
						angleIncrement = ticks > 0
							? angleDiff / static_cast<float>(ticks)
							: angleDiff;

						externalData[currentScriptNodePointer.get()]
							= new float{ angleIncrement };
					}

					//increment angle
					wasp::math::Angle newAngle{
						static_cast<float>(oldAngle) + angleIncrement
					};

					//check to see if we have reached target angle
					float angleDiff{
						std::abs(static_cast<float>(
							targetAngle.smallerDifference(newAngle)
						))
					};
					if (angleDiff <= angleEquivalenceEpsilon) {
						hasReachedAngle = true;
						velocity.setAngle(targetAngle);
					}
					else {
						velocity.setAngle(newAngle);
					}
				}

				//move on to next node if present
				if (hasReachedAngle) {
					clearExternalDataForNode(currentScriptNodePointer, externalData);
					gotoNextNode(currentScriptNodePointer, 0);
					return true;
				}
				else {
					return false;
				}
			}
			case ScriptInstructions::shiftAngleLongPeriod: {
				Velocity& velocity{
					scene.getDataStorage().getComponent<Velocity>(entityID)
				};
				auto oldAngle{ velocity.getAngle() };

				auto dataNodePointer{
					dynamic_cast<ScriptNodeData<
						std::tuple<wasp::math::Angle, wasp::math::Angle, int>,
						float
					>*>(
						currentScriptNodePointer.get()
					)
				};

				const auto [targetAngle, initAngle, ticks]
					= dataNodePointer->internalData;

				bool hasReachedAngle{ false };

				if (oldAngle == targetAngle) {
					hasReachedAngle = true;
				}

				if (!hasReachedAngle) {
					//retrieve or create increment
					float angleIncrement{};
					if (externalData.find(currentScriptNodePointer.get())
						!= externalData.end()
						) {
						float storedAngleIncrement{
							*dataNodePointer->getDataPointer(
								externalData[currentScriptNodePointer.get()]
							)
						};
						angleIncrement = storedAngleIncrement;
					}
					else {
						//make sure old angle and init angle are the same
						oldAngle = initAngle;

						//create and store increments
						float angleDiff{ targetAngle.largerDifference(initAngle) };
						angleIncrement = ticks > 0
							? angleDiff / static_cast<float>(ticks)
							: angleDiff;

						externalData[currentScriptNodePointer.get()]
							= new float{ angleIncrement };
					}

					//increment angle
					wasp::math::Angle newAngle{
						static_cast<float>(oldAngle) + angleIncrement
					};

					//check to see if we have reached target angle
					float angleDiff{
						std::abs(static_cast<float>(
							targetAngle.smallerDifference(newAngle)
						))
					};
					if (angleDiff <= angleEquivalenceEpsilon) {
						hasReachedAngle = true;
						velocity.setAngle(targetAngle);
					}
					else {
						velocity.setAngle(newAngle);
					}
				}

				//move on to next node if present
				if (hasReachedAngle) {
					clearExternalDataForNode(currentScriptNodePointer, externalData);
					gotoNextNode(currentScriptNodePointer, 0);
					return true;
				}
				else {
					return false;
				}
			}
			case ScriptInstructions::shiftSpeedIncrement: {
				Velocity& velocity{
					scene.getDataStorage().getComponent<Velocity>(entityID)
				};
				float oldMagnitude{ velocity.getMagnitude() };

				auto dataNodePointer{
					dynamic_cast<ScriptNodeData<
						std::tuple<float, float>,
						wasp::utility::Void
					>*>(
						currentScriptNodePointer.get()
					)
				};

				auto [targetSpeed, increment] = dataNodePointer->internalData;

				bool hasReachedSpeed{ false };

				//if we are at speed already, move to next instruction
				if (oldMagnitude == targetSpeed) {
					hasReachedSpeed = true;
				}
				else {
					//increment velocity
					float newMagnitude{ oldMagnitude + increment };

					//check to see if we have reached target speed
					if (increment > 0 && newMagnitude > targetSpeed) {
						hasReachedSpeed = true;
						velocity.setMagnitude(targetSpeed);
					}
					else if (increment < 0 && newMagnitude < targetSpeed) {
						hasReachedSpeed = true;
						velocity.setMagnitude(targetSpeed);
					}
					else {
						velocity.setMagnitude(newMagnitude);
					}
				}
				//move on to next node if present
				if (hasReachedSpeed) {
					clearExternalDataForNode(currentScriptNodePointer, externalData);
					gotoNextNode(currentScriptNodePointer, 0);
					return true;
				}
				else {
					return false;
				}
			}
			case ScriptInstructions::gotoDecelerating: {
				auto& dataStorage{ scene.getDataStorage() };
				auto& position{
					dataStorage.getComponent<Position>(entityID)
				};

				auto dataNodePointer{
					dynamic_cast<ScriptNodeData<
						std::tuple<wasp::math::Point2, float>,
						float
					>*>(
						currentScriptNodePointer.get()
					)
				};

				auto& [targetPos, maxSpeed] = dataNodePointer->internalData;

				bool hasReachedPos{ false };

				//test if we have already reached our target position
				float currentDistance{ wasp::math::distanceFromAToB(position, targetPos) };
				if (currentDistance < pointEquivalenceEpsilon) {
					position = targetPos;
					hasReachedPos = true;
				}
				else {
					//set or retrieve our initial distance
					float initDistance{};
					if (externalData.find(currentScriptNodePointer.get())
						!= externalData.end()
						) {
						float storedInitDistance{
							*dataNodePointer->getDataPointer(
								externalData[currentScriptNodePointer.get()]
							)
						};
						initDistance = storedInitDistance;
					}
					else {
						//create and store init dist
						initDistance = currentDistance;
						externalData[currentScriptNodePointer.get()]
							= new float{ initDistance };
					}
					const auto& angle{ wasp::math::getAngleFromAToB(position, targetPos) };
					float speed{ calculateGotoDeceleratingSpeed(
						currentDistance,
						initDistance,
						maxSpeed
					) };

					wasp::debug::log("goto decelerating angle speed " +
						std::to_string(angle) + std::to_string(speed));

					//alter velocity
					if (dataStorage.containsComponent<Velocity>(entityID)) {
						auto& velocity{
							dataStorage.getComponent<Velocity>(entityID)
						};
						velocity = Velocity{ speed, angle };
					}
					else {
						EntityHandle handle{ dataStorage.makeHandle(entityID) };
						componentOrderQueue.queueSetComponent(
							handle,
							Velocity{ speed, angle }
						);
					}
				}
				if (hasReachedPos) {
					//first, clear velocity
					if (dataStorage.containsComponent<Velocity>(entityID)) {
						auto& velocity{
							dataStorage.getComponent<Velocity>(entityID)
						};
						velocity = Velocity{};
					}
					//then, move on to next node if present
					clearExternalDataForNode(currentScriptNodePointer, externalData);
					gotoNextNode(currentScriptNodePointer, 0);
					return true;
				}
				else {
					return false;
				}
			}
			case ScriptInstructions::boundRadiusGotoDecelerating: {
				auto& dataStorage{ scene.getDataStorage() };
				auto& position{
					dataStorage.getComponent<Position>(entityID)
				};

				auto dataNodePointer{
					dynamic_cast<ScriptNodeData<
						std::tuple<wasp::math::AABB, float, float, float>,
						std::tuple<wasp::math::Point2, float>
					>*>(
						currentScriptNodePointer.get()
					)
				};

				auto& [bounds, minRadius, maxRadius, maxSpeed] =
					dataNodePointer->internalData;

				//set or retrieve our target and initDist
				wasp::math::Point2 targetPos{};
				float initDistance{};

				if (externalData.find(currentScriptNodePointer.get())
					!= externalData.end()
					) {
					auto& [storedTargetPos, storedInitDistance] =
						*dataNodePointer->getDataPointer(
							externalData[currentScriptNodePointer.get()]
						);
					targetPos = storedTargetPos;
					initDistance = storedInitDistance;
				}
				else {
					config::PrngType& prng{
						scene.getChannel(SceneTopics::random).getMessages()[0]
					};
					std::uniform_real_distribution<float> radiusDistribution{
						minRadius, maxRadius
					};
					static std::uniform_real_distribution<float> angleDistribution{
						0.0f, 360.0f
					};
					//create and store target pos and init dist
					//nubDotDev's algorithm, take max of 2 random for radius
					do {
						float radius{
							std::max(
								radiusDistribution(prng),
								radiusDistribution(prng)
							)
						};
						wasp::math::Angle angle{
								angleDistribution(prng)
						};
						targetPos = position + Velocity{ radius, angle };
					} while (!wasp::math::isPointWithinAABB(targetPos, bounds));

					float currentDistance{
						wasp::math::distanceFromAToB(position, targetPos)
					};

					initDistance = currentDistance;
					externalData[currentScriptNodePointer.get()]
						= new std::tuple<wasp::math::Point2, float>{
							targetPos,
							initDistance
					};

				}

				bool hasReachedPos{ false };

				//test if we have already reached our target position
				float currentDistance{	//recalculating it here but whatever
					wasp::math::distanceFromAToB(position, targetPos)
				};
				if (currentDistance < pointEquivalenceEpsilon) {
					position = targetPos;
					hasReachedPos = true;
				}
				else {
					const auto& angle{ wasp::math::getAngleFromAToB(position, targetPos) };
					float speed{ calculateGotoDeceleratingSpeed(
						currentDistance,
						initDistance,
						maxSpeed
					) };

					//alter velocity
					if (dataStorage.containsComponent<Velocity>(entityID)) {
						auto& velocity{
							dataStorage.getComponent<Velocity>(entityID)
						};
						velocity = Velocity{ speed, angle };
					}
					else {
						EntityHandle handle{ dataStorage.makeHandle(entityID) };
						componentOrderQueue.queueSetComponent(
							handle,
							Velocity{ speed, angle }
						);
					}
				}
				if (hasReachedPos) {
					//first, clear velocity
					if (dataStorage.containsComponent<Velocity>(entityID)) {
						auto& velocity{
							dataStorage.getComponent<Velocity>(entityID)
						};
						velocity = Velocity{};
					}
					//then, move on to next node if present
					clearExternalDataForNode(currentScriptNodePointer, externalData);
					gotoNextNode(currentScriptNodePointer, 0);
					return true;
				}
				else {
					return false;
				}
			}
			case ScriptInstructions::die: {
				EntityHandle entityHandle{ scene.getDataStorage().makeHandle(entityID) };
				scene.getChannel(SceneTopics::deaths).addMessage(entityHandle);
				gotoNextNode(currentScriptNodePointer, 0);
				return true;
			}
			case ScriptInstructions::removeEntity: {
				componentOrderQueue.queueRemoveEntity(
					scene.getDataStorage().makeHandle(entityID)
				);
				gotoNextNode(currentScriptNodePointer, 0);
				return true;
			}
			case ScriptInstructions::showDialogue: {
				auto dataNodePointer{
					dynamic_cast<ScriptNodeData<std::wstring, wasp::utility::Void>*>(
						currentScriptNodePointer.get()
					)
				};
				//add message to sceneEntry and startDialogue topics
				globalChannelSetPointer->getChannel(GlobalTopics::sceneEntry).addMessage(
					SceneNames::dialogue
				);
				globalChannelSetPointer->getChannel(GlobalTopics::startDialogue)
					.addMessage(
						dataNodePointer->internalData
					);
				gotoNextNode(currentScriptNodePointer, 0);
				return true;
			}
			case ScriptInstructions::win: {
				scene.getChannel(SceneTopics::winFlag).addMessage();
				gotoNextNode(currentScriptNodePointer, 0);
				return true;
			}
			case ScriptInstructions::endStage: {
				globalChannelSetPointer->getChannel(GlobalTopics::stopMusicFlag)
					.addMessage();
				auto& gameStateChannel{
					globalChannelSetPointer->getChannel(GlobalTopics::gameState)
				};
				GameState& gameState{ gameStateChannel.getMessages()[0] };
				//send us back to the correct menu
				SceneNames backTo{};
				switch (gameState.gameMode) {
					case GameMode::campaign:
						backTo = SceneNames::main;
						break;
					case GameMode::practice:
						backTo = SceneNames::stage;
						break;
					default:
						throw std::runtime_error{
							"default case reached in GameOverSystem.gameOver()!"
						};
				}

				globalChannelSetPointer->getChannel(GlobalTopics::sceneExitTo)
					.addMessage(backTo);
				if(gameState.gameMode == GameMode::campaign) {
					auto& sceneEntryChannel{
						globalChannelSetPointer->getChannel(GlobalTopics::sceneEntry)
					};
					switch (gameState.stage) {
						case 1:
						case 2:
						case 3:
						case 4:
						{
							++gameState.stage;
							//immediately pop up a new game and loading screen
							sceneEntryChannel.addMessage(SceneNames::game);
							sceneEntryChannel.addMessage(SceneNames::load);

							//send player data to global
							static const Topic<wasp::ecs::component::Group*>
								playerGroupPointerStorageTopic{};

							auto playerGroupPointer{
								getGroupPointer<PlayerData>(
									scene,
									playerGroupPointerStorageTopic
								)
							};

							auto playerGroupIterator{
								playerGroupPointer->groupIterator<PlayerData>()
							};

							PlayerData playerData{
								ShotType::shotA,
								-1,
								-1,
								-1,
								-1
							};
							while (playerGroupIterator.isValid()) {
								playerData = std::get<0>(*playerGroupIterator);
								break;
							}
							auto& playerDataChannel{
								globalChannelSetPointer->getChannel(
									GlobalTopics::playerData
								)
							};
							playerDataChannel.clear();
							playerDataChannel.addMessage(playerData);
							break;
						}
						case 5:
							//go to credits screen
							sceneEntryChannel.addMessage(SceneNames::credits);
							globalChannelSetPointer->getChannel(
								GlobalTopics::startMusic
							).addMessage(L"12");
							break;
					}
				}
				else {
					//if we are in practice, then we go back to menu, thus menu track
					globalChannelSetPointer->getChannel(GlobalTopics::startMusic)
						.addMessage(L"01");
				}
				return false;
			}
			default:
				throw std::runtime_error{ "unhandled script instruction!" };
		}
	}

	Velocity ScriptSystem::evaluateVelocityNode(NODE_HANDLER_ARGS) {
		switch (currentScriptNodePointer->scriptInstruction) {
			case ScriptInstructions::value: {
				auto dataNodePointer{
					dynamic_cast<ScriptNodeData<Velocity, wasp::utility::Void>*>(
						currentScriptNodePointer.get()
					)
				};
				return dataNodePointer->internalData;
			}
			default:
				throw std::runtime_error{ "not a velocity instruction!" };
		}
	}

	bool ScriptSystem::evaluatePredicateNode(NODE_HANDLER_ARGS) {
		switch (currentScriptNodePointer->scriptInstruction) {
			case ScriptInstructions::isSpawning: {
				const auto& dataStorage{ scene.getDataStorage() };
				if (dataStorage.containsComponent<SpawnProgramList>(entityID)) {
					const auto& spawnProgramList{
						dataStorage.getComponent<SpawnProgramList>(entityID)
					};
					return spawnProgramList.size() > 0;
				}
				return false;
			}
			case ScriptInstructions::isNotSpawning: {
				const auto& dataStorage{ scene.getDataStorage() };
				if (dataStorage.containsComponent<SpawnProgramList>(entityID)) {
					const auto& spawnProgramList{
						dataStorage.getComponent<SpawnProgramList>(entityID)
					};
					return spawnProgramList.size() == 0;
				}
				return true;
			}
			case ScriptInstructions::isBossDead: {
				if (scene.getChannel(SceneTopics::bossDeaths).hasMessages()) {
					scene.getChannel(SceneTopics::bossDeaths).clear();
					return true;
				}
				return false;
			}
			case ScriptInstructions::isDialogueOver: {
				auto& endDialogueFlagChannel{
					globalChannelSetPointer->getChannel(GlobalTopics::endDialogueFlag)
				};
				if (endDialogueFlagChannel.hasMessages()) {
					endDialogueFlagChannel.clear();
					return true;
				}
				return false;
			}
			case ScriptInstructions::isWin: {
				auto& winFlagChannel{
					scene.getChannel(SceneTopics::winFlag)
				};
				if (winFlagChannel.hasMessages()) {
					winFlagChannel.clear();
					return true;
				}
				return false;
			}
			case ScriptInstructions::boundaryYLow: {
				const auto& dataStorage{ scene.getDataStorage() };
				if (dataStorage.containsComponent<Position>(entityID)) {
					auto& position{ dataStorage.getComponent<Position>(entityID) };
					auto dataNodePointer{
						dynamic_cast<ScriptNodeData<float, wasp::utility::Void>*>(
							currentScriptNodePointer.get()
						)
					};
					float boundary{ dataNodePointer->internalData };
					return position.y < boundary;
				}
				return false;
			}
			case ScriptInstructions::boundaryYHigh: {
				const auto& dataStorage{ scene.getDataStorage() };
				if (dataStorage.containsComponent<Position>(entityID)) {
					auto& position{ dataStorage.getComponent<Position>(entityID) };
					auto dataNodePointer{
						dynamic_cast<ScriptNodeData<float, wasp::utility::Void>*>(
							currentScriptNodePointer.get()
						)
					};
					float boundary{ dataNodePointer->internalData };
					return position.y > boundary;
				}
				return false;
			}
			case ScriptInstructions::boundaryXLow: {
				const auto& dataStorage{ scene.getDataStorage() };
				if (dataStorage.containsComponent<Position>(entityID)) {
					auto& position{ dataStorage.getComponent<Position>(entityID) };
					auto dataNodePointer{
						dynamic_cast<ScriptNodeData<float, wasp::utility::Void>*>(
							currentScriptNodePointer.get()
						)
					};
					float boundary{ dataNodePointer->internalData };
					return position.x < boundary;
				}
				return false;
			}
			case ScriptInstructions::boundaryXHigh: {
				const auto& dataStorage{ scene.getDataStorage() };
				if (dataStorage.containsComponent<Position>(entityID)) {
					auto& position{ dataStorage.getComponent<Position>(entityID) };
					auto dataNodePointer{
						dynamic_cast<ScriptNodeData<float, wasp::utility::Void>*>(
							currentScriptNodePointer.get()
						)
					};
					float boundary{ dataNodePointer->internalData };
					return position.x > boundary;
				}
				return false;
			}
			default:
				throw std::runtime_error{ "not a predicate instruction!" };
		}
	}

	#undef NODE_HANDLER_ARGS
}