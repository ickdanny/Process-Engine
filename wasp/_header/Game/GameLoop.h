#pragma once

#include <functional>
#include <chrono>
#include <stdexcept>
#include <cstdint>

namespace wasp::game {

	//making this a class because i want to be able to stop it
	class GameLoop {
	private:
		using ClockType = std::chrono::steady_clock;
		using TimePointType = ClockType::time_point;
		using DurationType = ClockType::duration;

		bool running{};
		int updatesPerSecond{};
		int maxUpdatesWithoutFrame{};
		std::function<void()> updateFunction{};
		std::function<void(float)> renderFunction{};

	public:
		GameLoop(
			int updatesPerSecond, 
			int maxUpdatesWithoutFrame,
			const std::function<void()>& updateFunction,
			const std::function<void(float)>& renderFunction
		)
			: running{ false }
			, updatesPerSecond { updatesPerSecond}
			, maxUpdatesWithoutFrame{ maxUpdatesWithoutFrame }
			, updateFunction{ updateFunction }
			, renderFunction{ renderFunction }{
		};

		void run();

		void stop();

	private:
		static TimePointType getCurrentTime();

		//returns a float between 0 and 1 inclusive representing the delta time in terms
		//of the timestep
		static float calcDeltaTime(
			TimePointType timeOfLastUpdate,
			DurationType timeBetweenUpdates
		);

		static DurationType calcTimeSinceLastUpdate(TimePointType timeOfLastUpdate);
	};
}