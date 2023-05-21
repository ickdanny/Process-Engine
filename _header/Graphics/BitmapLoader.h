#pragma once

#include "windowsInclude.h"
#include "d3dInclude.h"
#include <wincodec.h>			//WIC

#include <string>

namespace process::graphics {
	class BitmapLoader {
	private:
		//typedefs
		template <typename T>
		using ComPtr = Microsoft::WRL::ComPtr<T>;
		
		ComPtr<IWICImagingFactory> wicFactoryPointer{};
	
	public:
		BitmapLoader();
		
		ComPtr<IWICBitmapFrameDecode> getWicBitmapFrameDecodePointer(
			const std::wstring& fileName
		);
		
		ComPtr<ID2D1Bitmap> convertWicBitmapToD2D(
			const ComPtr<IWICFormatConverter> formatConverterPointer,
			const ComPtr<ID2D1HwndRenderTarget> renderTargetPointer
		);
	
	private:
		void init();
		void initWicFactory();
		
		ComPtr<IWICBitmapDecoder> getBitmapDecoderPointer(
			const std::wstring& fileName
		);
		
		DXGI_FORMAT getD3DFormatFromWicBitcode(
			const ComPtr<IWICBitmapFrameDecode> framePointer
		);
		/*
		void initWicFormatConverter(
			const ComPtr<IWICFormatConverter> wicFormatConverterPointer,
			const ComPtr<IWICBitmapFrameDecode> framePointer
		);
		 */
	};
}