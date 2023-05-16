#pragma once

#include <string>
#include <stdexcept>

namespace wasp::resource {
	
	class IResourceStorage {
	public:
		IResourceStorage() = default;
		
		virtual ~IResourceStorage() = default;
		
		virtual void unload(const std::wstring& id) = 0;
		
		virtual void remove(const std::wstring& id) = 0;
		
		virtual void reload(const std::wstring& id) = 0;
		
		virtual void write(const std::wstring& id) const {
			throw std::runtime_error { "Error resource write unsupported" };
		};
	};
}