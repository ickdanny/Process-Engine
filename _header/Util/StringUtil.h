#pragma once

#include <string>

namespace stringUtil {
	std::string convertFromWideString(const std::wstring& wideString);
	std::wstring convertToWideString(const std::string& string);
}