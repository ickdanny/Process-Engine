#include "Utility\Scheduling.h"

#include <stdexcept>

#include "Adaptor\HResultError.h"
#include "Logging.h"

namespace wasp::utility {
	
	using windowsadaptor::HResultError;
	
	//not visible outside this translation unit
	class TimerHandle100ns {
	private:
		HANDLE timerHandle {};
	public:
		TimerHandle100ns()
			: timerHandle { CreateWaitableTimer(NULL, TRUE, NULL) } {
			if( !timerHandle ) {
				throw std::runtime_error { "Error failed to open timer handle" };
			}
		}
		
		void wait100ns(long long time100ns) {
			setTimer(time100ns);
			WaitForSingleObject(timerHandle, INFINITE);
		}
		
		void wait100nsWithEvent(long long time100ns, EventHandle& eventHandle) {
			setTimer(time100ns);
			HANDLE handleArray[2] { timerHandle, *(eventHandle.get()) };
			WaitForMultipleObjects(2, handleArray, FALSE, INFINITE);
		}
		
		//get pointer to the handle managed by this object
		HANDLE* get() {
			return &timerHandle;
		}
		
		~TimerHandle100ns() {
			CloseHandle(timerHandle);
		}
	
	private:
		void setTimer(long long time100ns) {
			LARGE_INTEGER time {};
			time.QuadPart = -time100ns;
			if( !SetWaitableTimer(timerHandle, &time, 0, NULL, NULL, FALSE) ) {
				throw std::runtime_error("Error setting timer");
			}
		}
	};
	
	EventHandle::EventHandle()
	//2nd parameter = manual reset true
		: eventHandle { CreateEvent(NULL, TRUE, FALSE, NULL) } {
		if( !eventHandle ) {
			throw std::runtime_error { "Error failed to open event handle" };
		}
	}
	
	EventHandle::~EventHandle() {
		try {
			if( !CloseHandle(eventHandle) ) {
				throw HResultError {
					HRESULT_FROM_WIN32(GetLastError())
				};
			}
		}
		catch( const std::exception& exception ) {
			debug::log(exception.what());
			//swallow error
		}
		catch( ... ) {
			debug::log("Exception caught of unknown type in EventHandle destructor");
			//swallow error
		}
	}
	
	void EventHandle::signal() {
		auto result { SetEvent(eventHandle) };
		if( !result ) {
			throw std::runtime_error { "Error failed to set event" };
		}
	}
	
	void EventHandle::unsignal() {
		auto result { ResetEvent(eventHandle) };
		if( !result ) {
			throw std::runtime_error { "Error failed to set event" };
		}
	}
	
	//get pointer to the handle managed by this object
	HANDLE* EventHandle::get() {
		return &eventHandle;
	}
	
	thread_local TimerHandle100ns timerHandle {};
	
	void sleep100ns(long long time100ns) {
		if( time100ns <= 0 ) {
			return;
		}
		
		timerHandle.wait100ns(time100ns);
	}
	
	void sleep100nsWithEvent(long long time100ns, EventHandle& eventHandle) {
		if( time100ns <= 0 ) {
			return;
		}
		
		timerHandle.wait100nsWithEvent(time100ns, eventHandle);
	}
}