#pragma once

#include "windowsInclude.h"
#include "d3dInclude.h"
#include <wincodec.h>//WIC

#include <string>
#include <vector>

#include "Graphics/Sprite.h"

namespace process::graphics {
	class SpriteLoader {
	private:
		//typedefs
		template <typename T>
		using ComPtr = Microsoft::WRL::ComPtr<T>;
		
		ComPtr<IWICImagingFactory> wicFactoryPointer{};
	
	public:
		SpriteLoader();
		
		ComPtr<IWICBitmapFrameDecode> getWicFramePointer(
			const std::wstring& fileName
		);
		
		Sprite convertWicFrameToSprite(
			const ComPtr<IWICBitmapFrameDecode>& framePointer,
			const ComPtr<ID3D11Device>& devicePointer
		);
	
	private:
		void init();
		void initWicFactory();
		
		ComPtr<IWICBitmapDecoder> getBitmapDecoderPointer(
			const std::wstring& fileName
		);
		
		static DXGI_FORMAT getD3DFormatFromWicFrame(
			const ComPtr<IWICBitmapFrameDecode>& framePointer
		);
		
		struct PixelDataBuffer{
			std::vector<byte> buffer{};
			std::size_t sizeBytes{};
			std::size_t widthBytes{};
			std::size_t heightBytes{};
			unsigned int width{};
			unsigned int height{};
		};
		
		PixelDataBuffer getPixelDataBuffer(const ComPtr<IWICBitmapFrameDecode>& framePointer);
		
		uint_least32_t getBitsPerPixel(const WICPixelFormatGUID& format);
	};
}