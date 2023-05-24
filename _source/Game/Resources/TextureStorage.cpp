#include "Game\Resources\TextureStorage.h"

#include "File\FileUtil.h"

namespace process::game::resources {
	
	namespace{
		using ResourceBase = wasp::resource::ResourceBase;
	}

	void TextureStorage::reload(const std::wstring& id) {
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

	ResourceBase* TextureStorage::loadFromFile(
		const resource::FileOrigin& fileOrigin,
		const resource::ResourceLoader& resourceLoader
	) {
		ComPtr<IWICBitmapFrameDecode> wicFrame{
			textureLoader.getWicFramePointer(fileOrigin.fileName)
		};

		ComPtr<ID3D11ShaderResourceView> d3dTextureView{};
		if (devicePointer) {
			d3dTextureView = textureLoader.convertWicFrameToD3DTextureView(
				wicFrame,
				devicePointer
			);
		}

		const std::wstring& id{ file::getFileName(fileOrigin.fileName) };
		if (resourceMap.find(id) != resourceMap.end()) {
			throw std::runtime_error{ "Error loaded pre-existing id" };
		}

		std::shared_ptr<resource::Resource<WicFrameAndD3DTextureView>> resourceSharedPointer{
			std::make_shared<resource::Resource<WicFrameAndD3DTextureView>>(
				id,
				fileOrigin,
				std::make_shared<WicFrameAndD3DTextureView>(
					WicFrameAndD3DTextureView{ wicFrame, d3dTextureView }
				)
			)
		};

		resourceSharedPointer->setStoragePointer(this);

		resourceMap.insert({ id, resourceSharedPointer });
		return resourceSharedPointer.get();
	}

	ResourceBase* TextureStorage::loadFromManifest(
		const resource::ManifestOrigin& manifestOrigin,
		const resource::ResourceLoader& resourceLoader
	) {
		const std::wstring& fileName{ manifestOrigin.manifestArguments[1] };
		ComPtr<IWICBitmapFrameDecode> wicFrame{
			textureLoader.getWicFramePointer(fileName)
		};

		ComPtr<ID3D11ShaderResourceView> d3dTextureView{};
		if (devicePointer) {
			d3dTextureView = textureLoader.convertWicFrameToD3DTextureView(
				wicFrame,
				devicePointer
			);
		}

		const std::wstring& id{ file::getFileName(fileName) };
		if (resourceMap.find(id) != resourceMap.end()) {
			throw std::runtime_error{ "Error loaded pre-existing id" };
		}

		std::shared_ptr<resource::Resource<WicFrameAndD3DTextureView>> resourceSharedPointer{
			std::make_shared<resource::Resource<WicFrameAndD3DTextureView>>(
				id,
				manifestOrigin,
				std::make_shared<WicFrameAndD3DTextureView>(
					WicFrameAndD3DTextureView{ wicFrame, d3dTextureView }
				)
			)
		};

		resourceSharedPointer->setStoragePointer(this);

		resourceMap.insert({ id, resourceSharedPointer });
		return resourceSharedPointer.get();
	}
	
	void TextureStorage::setdevicePointerAndLoadD3DTextures(
		const ComPtr<ID3D11Device>& devicePointer
	){
		this->devicePointer = devicePointer;
		throwIfCannotConstructD3DTextures();
		forEach(
			[&](std::shared_ptr<ResourceType> resourceSharedPointer) {
				loadD3DTexture(*resourceSharedPointer);
			}
		);
	}

	void TextureStorage::loadD3DTexture(ResourceType& resource) {
		auto& data{ *resource.getDataPointerCopy() };
		data.d3dTextureView = textureLoader.convertWicFrameToD3DTextureView(
			data.wicFrame,
			devicePointer
		);
	}

	void TextureStorage::throwIfCannotConstructD3DTextures() {
		if (!devicePointer) {
			throw std::runtime_error{ "Error cannot create D2D Bitmaps" };
		}
	}
}