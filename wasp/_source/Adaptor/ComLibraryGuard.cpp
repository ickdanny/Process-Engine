#include "Adaptor/ComLibraryGuard.h"

#include <combaseapi.h>

namespace wasp::windowsadaptor {
	ComLibraryGuard::ComLibraryGuard(DWORD threadingModel) {
		HRESULT result { CoInitializeEx(NULL, threadingModel) };
		if( FAILED(result) ) {
			throw HResultError { "Error initializing COM library" };
		}
	}
	
	ComLibraryGuard::~ComLibraryGuard() {
		CoUninitialize();
	}
}