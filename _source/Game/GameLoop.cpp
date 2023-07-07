#include "Game\GameLoop.h"

#include "Scheduling.h"

#include "Logging.h"

namespace process::game {
	
	namespace{
		constexpr uint64_t ratio100nsToSeconds { 10'000'000ull };
		using clockType = std::chrono::steady_clock;
		using ratioTimePointTo100ns = std::ratio<
			clockType::period::num * ratio100nsToSeconds,
			clockType::period::den
		>;
	}
	
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
				wasp::utility::sleep100ns(
					((nextUpdate - getCurrentTime()).count() * ratioTimePointTo100ns::num)
					/ ratioTimePointTo100ns::den
				);
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