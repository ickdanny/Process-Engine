#pragma once

#include <memory>
#include "windowsInclude.h"

#include "Resource\ResourceStorage.h"
#include "Resource\ResourceBase.h"
#include "Graphics\SpriteLoader.h"
#include "Graphics\Sprite.h"

#pragma warning(disable : 4250) //suppress inherit via dominance

namespace process::game::resources {

	struct WicFrameAndSprite {
		Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> wicFrame{};
		graphics::Sprite sprite{};
	};

	class SpriteStorage
		: public resource::ResourceStorage<WicFrameAndSprite>
		, public resource::FileLoadable
		, public resource::ManifestLoadable
	{
	private:
		using TextureLoader = graphics::SpriteLoader;
	public:
		using ResourceType = resource::Resource<WicFrameAndSprite>;
		using ResourceSharedPointer = std::shared_ptr<ResourceType>;
	private:
		using ResourceBase = wasp::resource::ResourceBase;
		template <typename T>
		using ComPtr = Microsoft::WRL::ComPtr<T>;
		
		TextureLoader spriteLoader{};
		ComPtr<ID3D11Device> devicePointer{};

	public:
		SpriteStorage()
			: FileLoadable{ {L"png"} }
			, ManifestLoadable{ {L"image"} } {
		}

		void reload(const std::wstring& id) override;

		ResourceBase* loadFromFile(
			const resource::FileOrigin& fileOrigin,
			const resource::ResourceLoader& resourceLoader
		) override;

		ResourceBase* loadFromManifest(
			const resource::ManifestOrigin& manifestOrigin,
			const resource::ResourceLoader& resourceLoader
		) override;

		void setDevicePointerAndLoadD3DTextures(const ComPtr<ID3D11Device>& devicePointer);
		
	private:
		void loadD3DTexture(ResourceType& resource);
		void throwIfCannotConstructD3DTextures();
	};
}