#include "Game/Resources/ScriptStorage.h"

#include "File/FileUtil.h"

#include <fstream>

namespace process::game::resources {
	
	namespace {
		using ResourceBase = wasp::resource::ResourceBase;
	}
	
	void ScriptStorage::reload(const std::wstring& id) {
		if( resourceLoaderPointer ) {
			auto found { resourceMap.find(id) };
			if( found != resourceMap.end() ) {
				auto& [id, resourcePointer] = *found;
				const resource::ResourceOriginVariant origin {
					resourcePointer->getOrigin()
				};
				switch( origin.index() ) {
					case 0: {
						resource::FileOrigin const* fileTest {
							std::get_if<resource::FileOrigin>(&origin)
						};
						if( fileTest ) {
							remove(id);
							loadFromFile(*fileTest, *resourceLoaderPointer);
						}
						break;
					}
					case 1: {
						resource::ManifestOrigin const* manifestTest {
							std::get_if<resource::ManifestOrigin>(&origin)
						};
						if( manifestTest ) {
							remove(id);
							loadFromManifest(*manifestTest, *resourceLoaderPointer);
						}
						break;
					}
				}
			}
		}
		else {
			throw std::runtime_error { "Error trying to reload without loader" };
		}
	}
	
	ResourceBase* ScriptStorage::loadFromFile(
		const resource::FileOrigin& fileOrigin,
		const resource::ResourceLoader& resourceLoader
	) {
		
		const std::wstring& id { file::getFileName(fileOrigin.fileName) };
		if( resourceMap.find(id) != resourceMap.end() ) {
			throw std::runtime_error { "Error loaded pre-existing id" };
		}
		
		std::shared_ptr<ResourceType> resourceSharedPointer {
			std::make_shared<ResourceType>(
				id,
				fileOrigin,
				std::make_shared<Script>(
					parseScriptFile(fileOrigin.fileName)
				)
			)
		};
		
		resourceSharedPointer->setStoragePointer(this);
		
		resourceMap.insert({ id, resourceSharedPointer });
		return resourceSharedPointer.get();
	}
	
	ResourceBase* ScriptStorage::loadFromManifest(
		const resource::ManifestOrigin& manifestOrigin,
		const resource::ResourceLoader& resourceLoader
	) {
		
		const std::wstring& fileName { manifestOrigin.manifestArguments[1] };
		
		const std::wstring& id { file::getFileName(fileName) };
		if( resourceMap.find(id) != resourceMap.end() ) {
			throw std::runtime_error { "Error loaded pre-existing id" };
		}
		
		std::shared_ptr<ResourceType> resourceSharedPointer {
			std::make_shared<ResourceType>(
				id,
				manifestOrigin,
				std::make_shared<Script>(
					parseScriptFile(fileName)
				)
			)
		};
		
		resourceSharedPointer->setStoragePointer(this);
		
		resourceMap.insert({ id, resourceSharedPointer });
		return resourceSharedPointer.get();
	}
	
	ScriptStorage::Script ScriptStorage::parseScriptFile(const std::wstring& fileName) {
		std::ifstream inStream { fileName };
		if(inStream.fail()){
			throw std::runtime_error{ "failed to open script file" };
		}
		std::stringstream stringStream{};
		stringStream << inStream.rdbuf();
		inStream.close();
		return parser.parse(lexer.lex(stringStream.str()));
	}
}