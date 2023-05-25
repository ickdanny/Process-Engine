#include "Graphics\SpriteLoader.h"
#include "HResultError.h"

#include <unordered_map>

template <>
struct std::hash<WICPixelFormatGUID> {
	std::size_t operator()(WICPixelFormatGUID const& format) const noexcept {
		std::size_t h1 = std::hash<unsigned long> {}(format.Data1);
		std::size_t h2 = std::hash<unsigned short> {}(format.Data2);
		std::size_t h3 = std::hash<unsigned short> {}(format.Data3);
		std::size_t h4 = hashCharArray(
			format.Data4,
			sizeof(format.Data4) / sizeof(unsigned char)
		);
		std::size_t hash { h1 };
		hash ^= (h2 << 5) + (h2 >> 3);
		hash ^= (h3 << 4) + (h3 >> 4);
		hash ^= (h4 << 2) + (h4 >> 8);
		return hash;
	}

private:
	static std::size_t hashCharArray(const unsigned char* charPointer, int length) {
		std::size_t hash { static_cast<size_t>(0xc82a320f8684e99f) };
		for( int index = 0; index < length; ++index ) {
			hash ^= std::hash<unsigned char> {}(charPointer[index])
				+ 0x9e3779b1
				+ (hash << 6)
				+ (hash >> 2);
		}
		return hash;
	}
};

namespace process::graphics {
	
	namespace {
		using wasp::windowsadaptor::HResultError;
		
		template <typename T>
		using ComPtr = Microsoft::WRL::ComPtr<T>;
		
		std::unordered_map<WICPixelFormatGUID, DXGI_FORMAT> wicToD3DFormatMap{
			{GUID_WICPixelFormat128bppRGBAFloat, DXGI_FORMAT_R32G32B32A32_FLOAT },
			{ GUID_WICPixelFormat64bppRGBAHalf ,DXGI_FORMAT_R16G16B16A16_FLOAT },
			{ GUID_WICPixelFormat64bppRGBA, DXGI_FORMAT_R16G16B16A16_UNORM },
			{ GUID_WICPixelFormat32bppRGBA, DXGI_FORMAT_R8G8B8A8_UNORM },
			{ GUID_WICPixelFormat32bppBGRA, DXGI_FORMAT_B8G8R8A8_UNORM },
			{ GUID_WICPixelFormat32bppBGR, DXGI_FORMAT_B8G8R8X8_UNORM },
			{ GUID_WICPixelFormat32bppRGBA1010102XR,
												 DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM },
			{ GUID_WICPixelFormat32bppRGBA1010102, DXGI_FORMAT_R10G10B10A2_UNORM },
			{ GUID_WICPixelFormat32bppRGBE, DXGI_FORMAT_R9G9B9E5_SHAREDEXP },
			{ GUID_WICPixelFormat16bppBGRA5551, DXGI_FORMAT_B5G5R5A1_UNORM },
			{ GUID_WICPixelFormat16bppBGR565, DXGI_FORMAT_B5G6R5_UNORM },
			{ GUID_WICPixelFormat32bppGrayFloat, DXGI_FORMAT_R32_FLOAT },
			{ GUID_WICPixelFormat16bppGrayHalf, DXGI_FORMAT_R16_FLOAT },
			{ GUID_WICPixelFormat16bppGray, DXGI_FORMAT_R16_UNORM },
			{ GUID_WICPixelFormat8bppGray, DXGI_FORMAT_R8_UNORM },
			{ GUID_WICPixelFormat8bppAlpha, DXGI_FORMAT_A8_UNORM },
			{ GUID_WICPixelFormat96bppRGBFloat, DXGI_FORMAT_R32G32B32_FLOAT }
		};
	}
	
	SpriteLoader::SpriteLoader() {
		init();
	}
	
	void SpriteLoader::init() {
		initWicFactory();
	}
	
	void SpriteLoader::initWicFactory() {
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
	
	ComPtr<IWICBitmapFrameDecode> SpriteLoader::getWicFramePointer(
		const std::wstring& fileName
	) {
		ComPtr<IWICBitmapDecoder> bitmapDecoderPointer{
			getBitmapDecoderPointer(fileName)
		};
		
		// Retrieve the first frame of the image from the decoder
		ComPtr<IWICBitmapFrameDecode> framePointer{};
		
		HRESULT result{
			bitmapDecoderPointer->GetFrame(0, &framePointer)
		};
		if (FAILED(result)) {
			throw HResultError{ "Error getting frame" };
		}
		return framePointer;
	}
	
	ComPtr<IWICBitmapDecoder> SpriteLoader::getBitmapDecoderPointer(
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
	
	Sprite SpriteLoader::convertWicFrameToSprite(
		const ComPtr<IWICBitmapFrameDecode>& framePointer,
		const ComPtr<ID3D11Device>& devicePointer
	) {
		PixelDataBuffer pixelDataBuffer{ getPixelDataBuffer(framePointer) };
		DXGI_FORMAT format{ getD3DFormatFromWicFrame(framePointer) };
		// Create texture
		D3D11_TEXTURE2D_DESC desc{};
		desc.Width = pixelDataBuffer.width;
		desc.Height = pixelDataBuffer.height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = format;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		
		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = pixelDataBuffer.buffer.data();
		initData.SysMemPitch = static_cast<UINT>( pixelDataBuffer.widthBytes );
		initData.SysMemSlicePitch = static_cast<UINT>( pixelDataBuffer.sizeBytes );
		
		ComPtr<ID3D11Texture2D> texturePointer{};
		HRESULT result{ devicePointer->CreateTexture2D(
			&desc,
			&initData,
			texturePointer.GetAddressOf()
		) };
		if(FAILED(result)){
			throw HResultError{ "Error creating Texture2D" };
		}
		
		//create view
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0u;
		srvDesc.Texture2D.MipLevels = 1u;
		
		ComPtr<ID3D11ShaderResourceView> viewPointer{};
		result = devicePointer->CreateShaderResourceView(
			texturePointer.Get(),
			&srvDesc,
			viewPointer.GetAddressOf()
		);
		if(FAILED(result)){
			throw HResultError{ "Error creating view for Texture2D" };
		}
		return {
			viewPointer,
			pixelDataBuffer.width,
			pixelDataBuffer.height
		};
	}
	
	DXGI_FORMAT SpriteLoader::getD3DFormatFromWicFrame(
		const ComPtr<IWICBitmapFrameDecode>& framePointer
	) {
		WICPixelFormatGUID wicPixelFormat{};
		HRESULT result{ framePointer->GetPixelFormat(&wicPixelFormat) };
		if (FAILED(result)){
			throw HResultError{ "Error determining WIC pixel format" };
		}
		auto search = wicToD3DFormatMap.find(wicPixelFormat);
		if(search != wicToD3DFormatMap.end()){
			return search->second;
		}
		else{
			throw std::runtime_error{ "Could not convert wic to d3d" };
		}
	}
	
	SpriteLoader::PixelDataBuffer SpriteLoader::getPixelDataBuffer(
		const ComPtr<IWICBitmapFrameDecode>& framePointer
	) {
		//https://stackoverflow.com/questions/25797536/getting-a-bitmap-bitsperpixel-from
		//-iwicbitmapsource-iwicbitmap-iwicbitmapdecod
		
		WICPixelFormatGUID format{};
		HRESULT result{ framePointer->GetPixelFormat(&format) };
		if (FAILED(result)){
			throw HResultError{ "Error determining WIC pixel format" };
		}
		
		auto bitsPerPixel { getBitsPerPixel(format) };
		
		UINT width{};
		UINT height{};
		result = framePointer->GetSize(&width, &height);
		if(FAILED(result)){
			throw HResultError{ "Error determining WIC pixel size" };
		}
		
		//+7 forces to next byte if needed
		auto widthBits { (float)(bitsPerPixel * width + 7) };
		auto widthBytes{ (UINT)(widthBits / 8.0f) };
		auto heightBits{ (float)(bitsPerPixel * height + 7) };
		auto heightBytes{ (UINT)(heightBits / 8.0f) };
		
		std::size_t bufferSize{ widthBytes * heightBytes };
		std::vector<byte> buffer(bufferSize);
		
		result = framePointer->CopyPixels(
			nullptr,
			widthBytes,
			bufferSize,
			buffer.data()
		);
		
		if(FAILED(result)){
			throw HResultError{ "Error copying data into buffer" };
		}
		
		return { buffer, bufferSize, widthBytes, heightBytes, width, height };
	}
	
	uint_least32_t SpriteLoader::getBitsPerPixel(const WICPixelFormatGUID& format){
		//get pointer to an instance of the Pixel Format
		ComPtr<IWICComponentInfo> componentInfoPointer {};
		HRESULT result { wicFactoryPointer->CreateComponentInfo(
			format,
			componentInfoPointer.GetAddressOf()
		) };
		if(FAILED(result)){
			throw HResultError{ "Failed to create WIC component info" };
		}
		
		//get IWICPixelFormatInfo from IWICComponentInfo
		ComPtr<IWICPixelFormatInfo> pixelFormatInfoPointer;
		
		result = componentInfoPointer->QueryInterface(
			__uuidof(IWICPixelFormatInfo),
			reinterpret_cast<void**>(pixelFormatInfoPointer.GetAddressOf())
		);
		if(FAILED(result)){
			throw HResultError{ "Failed to convert component info to pixel info" };
		}
		
		// get bits per pixel
		uint_least32_t bitsPerPixel{};
		result = pixelFormatInfoPointer->GetBitsPerPixel(&bitsPerPixel);
		if(FAILED(result)){
			throw HResultError{ "Failed to get bits per pixel " };
		}
		return bitsPerPixel;
	}
}