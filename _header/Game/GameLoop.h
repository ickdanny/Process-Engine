#pragma once

#include <functional>
#include <chrono>
#include <stdexcept>
#include <cstdint>

namespace process::game {
	
	class GameLoop {
	private:
		//typedefs
		using ClockType = std::chrono::steady_clock;
		using TimePointType = ClockType::time_point;
		using DurationType = ClockType::duration;
		
		//fields
		bool running {};
		int updatesPerSecond {};
		int maxUpdatesWithoutFrame {};
		std::function<void()> updateFunction {};
		std::function<void()> renderFunction {};
	
	public:
		GameLoop(
			int updatesPerSecond,
			int maxUpdatesWithoutFrame,
			const std::function<void()>& updateFunction,
			const std::function<void()>& renderFunction
		)
			: running { false }
			, updatesPerSecond { updatesPerSecond }
			, maxUpdatesWithoutFrame { maxUpdatesWithoutFrame }
			, updateFunction { updateFunction }
			, renderFunction { renderFunction } {
		};
		
		void run();
		
		void stop();
	
	private:
		static TimePointType getCurrentTime();
	};
}