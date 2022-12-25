#pragma once

#ifdef _DEBUG
#include <iostream>
#include <algorithm>
#endif

namespace wasp::debug {

    inline void log(const std::string& message) {
#ifdef _DEBUG
        std::cerr << message << '\n';
#endif
    }

    inline void log(const std::wstring& message) {
#ifdef _DEBUG
        std::string string(message.length(), 0);
		std::transform(
			message.begin(),
			message.end(),
			string.begin(),
			[](wchar_t wideChar) {
				return static_cast<char>(wideChar);
			}
		);
		log(string);
#endif
    }
}