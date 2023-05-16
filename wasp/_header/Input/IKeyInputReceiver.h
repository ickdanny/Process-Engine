#pragma once

#include "windowsInclude.h"

//the interface exposed to the OS
namespace wasp::input {
	class IKeyInputReceiver {
	public:
		virtual void handleKeyDown(WPARAM wParam, LPARAM lParam) = 0;
		
		virtual void handleKeyUp(WPARAM wParam, LPARAM lParam) = 0;
		
		virtual void allKeysOff() = 0;
	};
}