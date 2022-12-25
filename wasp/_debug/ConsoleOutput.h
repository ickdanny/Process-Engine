#pragma once

#ifdef _DEBUG

#include "windowsInclude.h"

#include <io.h>
#include <iostream>
#include <fcntl.h>

#endif

namespace wasp::debug {

    //courtesy of tochka
    void initConsoleOutput()
    {
#ifdef _DEBUG

        // for some reason console is patrially allocated on GUI window creation, but
		// isn't well defined to be shown by ShowWindow() or interacted with, 
		// so, we just make sure that it will be freshly initialized
		if (!FreeConsole())
			throw std::runtime_error("error freeing console");

		if (!AllocConsole())
			throw std::runtime_error("error allocating console");

		FILE* fDummy{};
		freopen_s(&fDummy, "CONIN$", "r", stdin);
		freopen_s(&fDummy, "CONOUT$", "w", stderr);
		freopen_s(&fDummy, "CONOUT$", "w", stdout);

#endif
    }
}