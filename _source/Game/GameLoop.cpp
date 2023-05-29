#include "Game\GameLoop.h"

namespace process::game {
	
	void GameLoop::run() {
		DurationType timeBetweenUpdates {
			static_cast<DurationType::rep>(
				((1.0 / updatesPerSecond) * ClockType::period::den)
					/ ClockType::period::num
			)
		};
		
		TimePointType nextUpdate { getCurrentTime() };
		int updatesWithoutFrame { 0 };
		
		running = true;
		while( running ) {
			//force draw every few updates
			if( updatesWithoutFrame >= maxUpdatesWithoutFrame ) {
				renderFunction();
				updatesWithoutFrame = 0;
			}
			//update if time
			if( getCurrentTime() >= nextUpdate ) {
				updateFunction();
				nextUpdate += timeBetweenUpdates;
				if( nextUpdate < getCurrentTime() ) {
					nextUpdate = getCurrentTime();
				}
			}
			//draw frames if possible
			if( getCurrentTime() < nextUpdate ) {
				renderFunction();
				while( getCurrentTime() < nextUpdate && running ) {
					//do nothing
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
}