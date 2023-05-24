#pragma once

#include <memory>
#include "windowsInclude.h"

#include "Resource\ResourceStorage.h"
#include "Resource\ResourceBase.h"
#include "Graphics\TextureLoader.h"

#pragma warning(disable : 4250) //suppress inherit via dominance

namespace process::game::resources {

	struct WicFrameAndD3DTextureView {
		Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> wicFrame{};
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> d3dTextureView{};
	};

	class TextureStorage
		: public resource::ResourceStorage<WicFrameAndD3DTextureView>
		, public resource::FileLoadable
		, public resource::ManifestLoadable
	{
	private:
		using TextureLoader = graphics::TextureLoader;
		using ResourceType = resource::Resource<WicFrameAndD3DTextureView>;
		using ResourceBase = wasp::resource::ResourceBase;
		template <typename T>
		using ComPtr = Microsoft::WRL::ComPtr<T>;
		
		TextureLoader textureLoader{};
		ComPtr<ID3D11Device> devicePointer{};

	public:
		TextureStorage()
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

		void setdevicePointerAndLoadD3DTextures(const ComPtr<ID3D11Device>& devicePointer);
		
	private:
		void loadD3DTexture(ResourceType& resource);
		void throwIfCannotConstructD3DTextures();
	};
}