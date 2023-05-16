#pragma once

#include "windowsInclude.h"

namespace wasp::utility {
	
	//auto reset event
	class EventHandle {
	private:
		HANDLE eventHandle {};
	public:
		EventHandle();
		
		~EventHandle();
		
		void signal();
		
		void unsignal();
		
		HANDLE* get();
	};
	
	void sleep100ns(long long time100ns);
	
	void sleep100nsWithEvent(long long time100ns, EventHandle& eventHandle);
}