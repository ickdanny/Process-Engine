#include "Graphics/BitmapLoader.h"
#include "HResultError.h"

#include <unordered_map>

namespace process::graphics {
	
	namespace {
		using wasp::windowsadaptor::HResultError;
		
		template <typename T>
		using ComPtr = Microsoft::WRL::ComPtr<T>;
		
		std::unordered_map<WICPixelFormatGUID, DXGI_FORMAT> wicToD3DFormatMap{
			{GUID_WICPixelFormat128bppRGBAFloat, DXGI_FORMAT_R32G32B32A32_FLOAT}
		};
		//todo: need a hash for GUID...
	}
	
	BitmapLoader::BitmapLoader() {
		init();
	}
	
	void BitmapLoader::init() {
		initWicFactory();
	}
	
	void BitmapLoader::initWicFactory() {
		HRESULT result{
			CoCreateInstance(
				CLSID_WICImagingFactory,
				nullptr,
				CLSCTX_INPROC_SERVER,
				IID_PPV_ARGS(&wicFactoryPointer)
			)
		};
		if (FAILED(result)) {
			throw HResultError{ "Error creating WIC imaging factory" };
		}
	}
	
	ComPtr<IWICBitmapFrameDecode> BitmapLoader::getWicBitmapFrameDecodePointer(
		const std::wstring& fileName
	) {
		ComPtr<IWICBitmapDecoder> bitmapDecoderPointer{
			getBitmapDecoderPointer(fileName)
		};
		
		// Retrieve the first frame of the image from the decoder
		ComPtr<IWICBitmapFrameDecode> bitmapFrameDecodePointer{};
		
		HRESULT result{
			bitmapDecoderPointer->GetFrame(0, &bitmapFrameDecodePointer)
		};
		if (FAILED(result)) {
			throw HResultError{ "Error getting frame" };
		}
		return bitmapFrameDecodePointer;
	}
	
	ComPtr<IWICBitmapDecoder> BitmapLoader::getBitmapDecoderPointer(
		const std::wstring& fileName
	) {
		ComPtr<IWICBitmapDecoder> bitmapDecoderPointer{};
		HRESULT result{ wicFactoryPointer->CreateDecoderFromFilename(
			fileName.c_str(),                	// Image to be decoded
			nullptr,                            // Do not prefer a particular vendor
			GENERIC_READ,                    // Desired read access to the file
			WICDecodeMetadataCacheOnDemand,  // Cache metadata when needed
			&bitmapDecoderPointer           		// Pointer to the decoder
		) };
		if (FAILED(result)) {
			throw HResultError{ "Error creating decoder" };
		}
		return bitmapDecoderPointer;
	}
	
	DXGI_FORMAT BitmapLoader::getD3DFormatFromWicBitcode(
		const ComPtr<IWICBitmapFrameDecode> framePointer
	) {
		WICPixelFormatGUID wicPixelFormat{};
		HRESULT result{ framePointer->GetPixelFormat(&wicPixelFormat) };
		if (FAILED(result)){
			throw HResultError{ "Error determining WIC pixel format" };
		}
		
	}
	
	ComPtr<ID2D1Bitmap> BitmapLoader::convertWicBitmapToD2D(
		const ComPtr<IWICFormatConverter> formatConverterPointer,
		const ComPtr<ID2D1HwndRenderTarget> renderTargetPointer
	) {
		ComPtr<ID2D1Bitmap> bitmapPointer{};
		HRESULT result{ renderTargetPointer->CreateBitmapFromWicBitmap(
			formatConverterPointer,
			NULL,
			&bitmapPointer
		) };
		if (FAILED(result)) {
			throw HResultError{ "Error creating bitmap from WIC bitmap" };
		}
		return bitmapPointer;
	}
	
	
	
	/*
	void BitmapLoader::initWicFormatConverter(
		const ComPtr<IWICFormatConverter> formatConverterPointer,
		const ComPtr<IWICBitmapFrameDecode> framePointer
	) {
		HRESULT result{
			formatConverterPointer->Initialize(
				framePointer.Get(),               	// Input bitmap to convert
				GUID_WICPixelFormat32bppPBGRA,  	// Destination pixel format
				WICBitmapDitherTypeNone,        		// Specified dither pattern
				nullptr,                          	// Specify a particular palette
				0.f,                    	// Alpha threshold
				WICBitmapPaletteTypeCustom   	// Palette translation type
			)
		};
		if (FAILED(result)) {
			throw HResultError{ "Error initiating WIC format converter with frame" };
		}
	}
	 */
}