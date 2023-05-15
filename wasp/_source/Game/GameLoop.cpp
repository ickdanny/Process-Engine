#include "Game\GameLoop.h"

namespace wasp::game {

	void GameLoop::run() {
		DurationType timeBetweenUpdates{
			static_cast<DurationType::rep>(
				((1.0 / updatesPerSecond) * ClockType::period::den)
				/ ClockType::period::num
			)
		};

		TimePointType nextUpdate{ getCurrentTime() };
		TimePointType timeOfLastUpdate{ getCurrentTime() };
		int updatesWithoutFrame{ 0 };

		running = true;
		while (running) {
			//force draw every few updates
			if (updatesWithoutFrame >= maxUpdatesWithoutFrame) {
				renderFunction(
					calcDeltaTime(timeOfLastUpdate, timeBetweenUpdates)
				);
				updatesWithoutFrame = 0;
			}
			//update if time
			if (getCurrentTime() >= nextUpdate) {
				updateFunction();
				nextUpdate += timeBetweenUpdates;
				if (nextUpdate < getCurrentTime()) {
					nextUpdate = getCurrentTime();
				}
				timeOfLastUpdate = getCurrentTime();
			}
			//draw frames if possible
			if (getCurrentTime() < nextUpdate) {
				while (getCurrentTime() < nextUpdate && running) {
					renderFunction(
						calcDeltaTime(timeOfLastUpdate, timeBetweenUpdates)
					);
				}
			}
			else {
				++updatesWithoutFrame;
			}
		}
	}

	void GameLoop::stop() {
		running = false;
	}

	//static
	GameLoop::TimePointType GameLoop::getCurrentTime() {
		return ClockType::now();
	}

	//static
	float GameLoop::calcDeltaTime(
		TimePointType timeOfLastUpdate,
		DurationType timeBetweenUpdates
	) {
		DurationType timeSinceLastUpdate{ calcTimeSinceLastUpdate(timeOfLastUpdate) };
		float deltaTime{
			static_cast<float>(
				DurationType{timeSinceLastUpdate / timeBetweenUpdates}.count()
			)
		};
		if (deltaTime < 0.0) {
			throw std::runtime_error{ "Error deltaTime < 0" };
		}
		if (deltaTime > 1.0) {
			return 1.0;
		}
		return deltaTime;
	}

	//static
	GameLoop::DurationType GameLoop::calcTimeSinceLastUpdate(
		TimePointType timeOfLastUpdate
	) {
		return getCurrentTime() - timeOfLastUpdate;
	}
}