#pragma once

#include <stdexcept>

namespace wasp::windowsadaptor {
	
	//rationale: constructors taking HRESULTs
	
	class HResultError : public std::runtime_error {
	public:
		explicit HResultError(const std::string& what_arg)
			: std::runtime_error { what_arg } {
		}
		
		explicit HResultError(const char* what_arg)
			: std::runtime_error { what_arg } {
		}
		
		explicit HResultError(const std::string& what_arg, HRESULT result)
			: std::runtime_error { what_arg + ("\nHRESULT: " + result) } {
		}
		
		explicit HResultError(const char* what_arg, HRESULT result)
			: std::runtime_error { std::string { what_arg } + ("\nHRESULT: " + result) } {
		}
		
		explicit HResultError(HRESULT result)
			: std::runtime_error { "HRESULT Error " + result } {
		}
	};
}