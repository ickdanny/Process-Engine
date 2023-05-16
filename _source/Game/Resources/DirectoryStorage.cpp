#include "Resources/DirectoryStorage.h"

#include "Resource\ResourceBase.h"

namespace process::game::resources {
	
	namespace {
		using ResourceBase = wasp::resource::ResourceBase;
	}
	
	void DirectoryStorage::reload(const std::wstring& id) {
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
	
	ResourceBase* DirectoryStorage::loadFromFile(
		const resource::FileOrigin& fileOrigin,
		const resource::ResourceLoader& resourceLoader
	) {
		
		std::shared_ptr childListPointer { std::make_shared<resource::ChildList>() };
		file::forEachDirectoryEntry(
			fileOrigin.fileName,
			[&](const std::wstring& fileName) {
				childListPointer->push_back(resourceLoader.loadFile({ fileName }));
			}
		);
		
		const std::wstring& id { file::getFileName(fileOrigin.fileName) };
		if( resourceMap.find(id) != resourceMap.end() ) {
			throw std::runtime_error { "Error loaded pre-existing id" };
		}
		
		std::shared_ptr<resource::ChildListResource> resourceSharedPointer {
			std::make_shared<resource::ChildListResource>(
				id,
				fileOrigin,
				childListPointer
			)
		};
		
		for( ResourceBase* childPointer : *childListPointer ) {
			childPointer->setParentPointer(&*resourceSharedPointer);
		}
		
		resourceSharedPointer->setStoragePointer(this);
		
		resourceMap.insert({ id, resourceSharedPointer });
		return resourceSharedPointer.get();
	}
	
	ResourceBase* DirectoryStorage::loadFromManifest(
		const resource::ManifestOrigin& manifestOrigin,
		const resource::ResourceLoader& resourceLoader
	) {
		std::shared_ptr childListPointer { std::make_shared<resource::ChildList>() };
		
		const std::wstring& directoryName { manifestOrigin.manifestArguments[1] };
		
		file::forEachDirectoryEntry(
			directoryName,
			[&](const std::wstring& fileName) {
				childListPointer->push_back(resourceLoader.loadFile({ fileName }));
			}
		);
		
		const std::wstring& id { file::getFileName(directoryName) };
		if( resourceMap.find(id) != resourceMap.end() ) {
			throw std::runtime_error { "Error loaded pre-existing id" };
		}
		
		std::shared_ptr<resource::ChildListResource> resourceSharedPointer {
			std::make_shared<resource::ChildListResource>(
				id,
				manifestOrigin,
				childListPointer
			)
		};
		
		for( ResourceBase* childPointer : *childListPointer ) {
			childPointer->setParentPointer(&*resourceSharedPointer);
		}
		
		resourceSharedPointer->setStoragePointer(this);
		
		resourceMap.insert({ id, resourceSharedPointer });
		return resourceSharedPointer.get();
	}
}