#pragma once

#include "windowsInclude.h"
#include "HResultError.h"

namespace wasp::windowsadaptor {
	class ComLibraryGuard {
	public:
		ComLibraryGuard(DWORD threadingModel);
		~ComLibraryGuard();
	};
}