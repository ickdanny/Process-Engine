#include "Game\Resources\SpriteStorage.h"

#include "File\FileUtil.h"

namespace process::game::resources {
	
	namespace{
		using ResourceBase = wasp::resource::ResourceBase;
		using Sprite = graphics::Sprite;
	}

	void SpriteStorage::reload(const std::wstring& id) {
		if (resourceLoaderPointer) {
			auto found{ resourceMap.find(id) };
			if (found != resourceMap.end()) {
				auto& [id, resourcePointer] = *found;
				const resource::ResourceOriginVariant origin{
					resourcePointer->getOrigin()
				};
				switch (origin.index()) {
					case 0: {
						resource::FileOrigin const* fileTest{
							std::get_if<resource::FileOrigin>(&origin) 
						};
						if (fileTest) {
							resourceMap.erase(found);
							loadFromFile(
								*fileTest,
								*resourceLoaderPointer)
							;
						}
						break;
					}
					case 1: {
						resource::ManifestOrigin const* manifestTest{
							std::get_if<resource::ManifestOrigin>(&origin)
						};
						if (manifestTest) {
							resourceMap.erase(found);
							loadFromManifest(
								*manifestTest,
								*resourceLoaderPointer
							);
						}
						break;
					}
				}
			}
		}
		else {
			throw std::runtime_error{ "Error trying to reload without loader" };
		}
	}

	ResourceBase* SpriteStorage::loadFromFile(
		const resource::FileOrigin& fileOrigin,
		const resource::ResourceLoader& resourceLoader
	) {
		ComPtr<IWICBitmapFrameDecode> wicFrame{
			spriteLoader.getWicFramePointer(fileOrigin.fileName)
		};

		Sprite sprite{};
		if (devicePointer) {
			sprite = spriteLoader.convertWicFrameToSprite(
				wicFrame,
				devicePointer
			);
		}

		const std::wstring& id{ file::getFileName(fileOrigin.fileName) };
		if (resourceMap.find(id) != resourceMap.end()) {
			throw std::runtime_error{ "Error loaded pre-existing id" };
		}

		ResourceSharedPointer resourceSharedPointer{
			std::make_shared<ResourceType>(
				id,
				fileOrigin,
				std::make_shared<WicFrameAndSprite>(
					WicFrameAndSprite{ wicFrame, sprite }
				)
			)
		};

		resourceSharedPointer->setStoragePointer(this);

		resourceMap.insert({ id, resourceSharedPointer });
		return resourceSharedPointer.get();
	}

	ResourceBase* SpriteStorage::loadFromManifest(
		const resource::ManifestOrigin& manifestOrigin,
		const resource::ResourceLoader& resourceLoader
	) {
		const std::wstring& id{ manifestOrigin.manifestArguments[1] };
		if (resourceMap.find(id) != resourceMap.end()) {
			throw std::runtime_error{ "Error loaded pre-existing id" };
		}
		
		const std::wstring& fileName{ manifestOrigin.manifestArguments[2] };
		
		ComPtr<IWICBitmapFrameDecode> wicFrame{
			spriteLoader.getWicFramePointer(fileName)
		};

		Sprite sprite{};
		if (devicePointer) {
			sprite = spriteLoader.convertWicFrameToSprite(
				wicFrame,
				devicePointer
			);
		}
		
		ResourceSharedPointer resourceSharedPointer{
			std::make_shared<ResourceType>(
				id,
				manifestOrigin,
				std::make_shared<WicFrameAndSprite>(
					WicFrameAndSprite{ wicFrame, sprite }
				)
			)
		};

		resourceSharedPointer->setStoragePointer(this);

		resourceMap.insert({ id, resourceSharedPointer });
		return resourceSharedPointer.get();
	}
	
	void SpriteStorage::setDevicePointerAndLoadD3DTextures(
		const ComPtr<ID3D11Device>& devicePointer
	){
		this->devicePointer = devicePointer;
		throwIfCannotConstructD3DTextures();
		forEach(
			[&](const ResourceSharedPointer& resourceSharedPointer) {
				loadD3DTexture(*resourceSharedPointer);
			}
		);
	}

	void SpriteStorage::loadD3DTexture(ResourceType& resource) {
		auto& data{ *resource.getDataPointerCopy() };
		data.sprite = spriteLoader.convertWicFrameToSprite(
			data.wicFrame,
			devicePointer
		);
	}

	void SpriteStorage::throwIfCannotConstructD3DTextures() {
		if (!devicePointer) {
			throw std::runtime_error{ "Error cannot create D2D Bitmaps" };
		}
	}
}