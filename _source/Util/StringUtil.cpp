#include "StringUtil.h"

#include <locale>
#include <codecvt>

namespace stringUtil {
	#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
	std::string convertFromWideString(const std::wstring& wideString) {
		//setup converter
		using convert_type = std::codecvt_utf8<wchar_t>;
		std::wstring_convert<convert_type, wchar_t> converter {};
		
		return converter.to_bytes(wideString);
	}
	
	std::wstring convertToWideString(const std::string& string) {
		using convert_type = std::codecvt_utf8<wchar_t>;
		std::wstring_convert<convert_type, wchar_t> converter {};
		
		return converter.from_bytes(string);
	}
	#undef _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
}