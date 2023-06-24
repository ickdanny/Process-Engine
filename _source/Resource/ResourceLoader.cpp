#include "Resource\ResourceLoader.h"

#include <stdexcept>

#include "File\FileUtil.h"

#include "Logging.h"

namespace process::resource {
	
	void ResourceLoader::insertFileLoadableIntoMap(
		const std::wstring& key,
		FileLoadable* value
	) {
		if( fileExtensionMap.find(key) != fileExtensionMap.end() ) {
			throw std::runtime_error { "Error 2 types claiming same file extension" };
		}
		fileExtensionMap.insert({ key, value });
	}
	
	void ResourceLoader::insertManifestLoadableIntoMap(
		const std::wstring& key,
		ManifestLoadable* value
	) {
		if( manifestPrefixMap.find(key) != manifestPrefixMap.end() ) {
			throw std::runtime_error { "Error 2 types claiming same manifest prefix" };
		}
		manifestPrefixMap.insert({ key, value });
	}
	
	ResourceLoader::ResourceBase* ResourceLoader::loadFile(
		const FileOrigin& fileOrigin
	) const {
		const std::wstring& extension {
			file::getFileExtension(fileOrigin.fileName)
		};
		try {
			return fileExtensionMap.at(extension)->loadFromFile(fileOrigin, *this);
		}
		catch(...){
			wasp::debug::log(L"failed to load from file: " + fileOrigin.fileName);
			throw;
		}
	}
	
	ResourceLoader::ResourceBase* ResourceLoader::loadManifestEntry(
		const ManifestOrigin& manifestOrigin
	) const {
		const std::wstring& prefix { manifestOrigin.manifestArguments[0] };
		try {
			return manifestPrefixMap.at(prefix)->loadFromManifest(
				manifestOrigin,
				*this
			);
		}
		catch(...){
			wasp::debug::log(L"failed to load from manifest: ");
			for(const auto& arg : manifestOrigin.manifestArguments){
				wasp::debug::log(arg);
			}
			throw;
		}
	}
}

//ignoring possibility loading resources creates new resource types