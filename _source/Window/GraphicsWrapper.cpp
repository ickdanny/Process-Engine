#include "Window\GraphicsWrapper.h"
#include "Adaptor\HResultError.h"

//todo: temp test
#include "Graphics/TextureLoader.h"

namespace process::window {
	
	using HResultError = wasp::windowsadaptor::HResultError;
	
	GraphicsWrapper::GraphicsWrapper(
		int graphicsWidth,
		int graphicsHeight
	)
		: graphicsWidth { graphicsWidth }
		, graphicsHeight { graphicsHeight } {
	}
	
	void GraphicsWrapper::init(HWND windowHandle) {
		getDevice(windowHandle);
		getRenderTargetView();
		getDepthStencilView();
		setupPipeline();
	}
	
	void GraphicsWrapper::getDevice(HWND windowHandle) {
		DXGI_SWAP_CHAIN_DESC swapChainDesc{ makeSwapChainDesc(windowHandle) };
		
		#ifndef _DEBUG
		UINT flags{ 0u };
		#else
		UINT flags { D3D11_CREATE_DEVICE_DEBUG };
		#endif
		
		HRESULT result {};
		result = D3D11CreateDeviceAndSwapChain(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			flags,
			nullptr,    //todo: d3d feature lvl?
			0,
			D3D11_SDK_VERSION,  //todo: d3d sdk version?
			&swapChainDesc,
			&swapChainPointer,
			&devicePointer,
			nullptr,
			&contextPointer
		);
		if( FAILED(result) ) {
			throw HResultError{"Error creating d3d device and swap-chain" };
		}
	}
	
	DXGI_SWAP_CHAIN_DESC GraphicsWrapper::makeSwapChainDesc(HWND windowHandle) {
		DXGI_SWAP_CHAIN_DESC swapChainDesc {};
		swapChainDesc.BufferDesc.Width = graphicsWidth;
		swapChainDesc.BufferDesc.Height = graphicsHeight;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 0;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 1;
		swapChainDesc.OutputWindow = windowHandle;
		swapChainDesc.Windowed = TRUE; //todo: fullscreen?
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.Flags = 0;
		return swapChainDesc;
	}
	
	void GraphicsWrapper::getRenderTargetView() {
		HRESULT result{};
		
		//get pointer to the back buffer
		ComPtr<ID3D11Resource> backBufferPointer{};
		result = swapChainPointer->GetBuffer(
			0,
			__uuidof(ID3D11Resource),
			reinterpret_cast<void**>(backBufferPointer.GetAddressOf())
		);
		if(FAILED(result)){
			throw HResultError{ "Error getting back buffer from swap chain" };
		}
		
		//create the render target view of the back buffer
		result = devicePointer->CreateRenderTargetView(
			backBufferPointer.Get(),
			nullptr,
			&renderTargetViewPointer
		);
		if(FAILED(result)){
			throw HResultError{ "Error creating render target view of back buffer" };
		}
	}
	
	void GraphicsWrapper::getDepthStencilView() {
		HRESULT result{};
		
		//create the depth buffer texture
		ComPtr<ID3D11Texture2D> depthStencilPointer{};
		D3D11_TEXTURE2D_DESC depthDesc{};
		depthDesc.Width = graphicsWidth;
		depthDesc.Height = graphicsHeight;
		depthDesc.MipLevels = 1u;
		depthDesc.ArraySize = 1u;
		depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
		depthDesc.SampleDesc.Count = 1u;
		depthDesc.SampleDesc.Quality = 0u;
		depthDesc.Usage = D3D11_USAGE_DEFAULT;
		depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		
		result = devicePointer->CreateTexture2D(
			&depthDesc,
			nullptr,
			depthStencilPointer.GetAddressOf()
		);
		if(FAILED(result)){
			throw HResultError{ "Error creating depth buffer texture" };
		}
		
		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
		dsvDesc.Format = depthDesc.Format;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D.MipSlice = 0u;
		
		result = devicePointer->CreateDepthStencilView(
			depthStencilPointer.Get(),
			&dsvDesc,
			depthStencilViewPointer.GetAddressOf()
		);
		if(FAILED(result)){
			throw HResultError{ "Error creating depth stencil view" };
		}
	}
	
	void GraphicsWrapper::setupPipeline() {
		HRESULT result{};
		ComPtr<ID3DBlob> blobPointer{};
		
		//assign pixel shader
		ComPtr<ID3D11PixelShader> psPointer{};
		D3DReadFileToBlob(L"PixelShader.cso", &blobPointer);
		result = devicePointer->CreatePixelShader(
			blobPointer->GetBufferPointer(),
			blobPointer->GetBufferSize(),
			nullptr,
			psPointer.GetAddressOf()
		);
		if(FAILED(result)){
			throw HResultError{ "Error creating pixel shader" };
		}
		contextPointer->PSSetShader(
			psPointer.Get(),
			nullptr,
			0
		);
		
		//specify sampler
		D3D11_SAMPLER_DESC samplerDesc{};
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		ComPtr<ID3D11SamplerState> samplerStatePointer{};
		result = devicePointer->CreateSamplerState(
			&samplerDesc,
			samplerStatePointer.GetAddressOf()
		);
		if(FAILED(result)){
			throw HResultError{ "Failed to create sampler state" };
		}
		contextPointer->PSSetSamplers(
			0u,
			1u,
			samplerStatePointer.GetAddressOf()
		);
		
		//assign vertex shader
		ComPtr<ID3D11VertexShader> vsPointer{};
		D3DReadFileToBlob(L"VertexShader.cso", &blobPointer);
		result = devicePointer->CreateVertexShader(
			blobPointer->GetBufferPointer(),
			blobPointer->GetBufferSize(),
			nullptr,
			vsPointer.GetAddressOf()
		);
		if(FAILED(result)){
			throw HResultError{ "Error creating vertex shader" };
		}
		contextPointer->VSSetShader(
			vsPointer.Get(),
			nullptr,
			0
		);
		
		//specify input layout
		ComPtr<ID3D11InputLayout> inputLayoutPointer{};
		const D3D11_INPUT_ELEMENT_DESC inputElementDesc[] {
			{
				"Position",
				0u,
				DXGI_FORMAT_R32G32B32_FLOAT,
				0u,
				0u,
				D3D11_INPUT_PER_VERTEX_DATA,
				0u
			},
			{
				"TexCoord",
				0u,
				DXGI_FORMAT_R32G32_FLOAT,
				0u,
				12u,
				D3D11_INPUT_PER_VERTEX_DATA,
				0u
			}
		};
		result = devicePointer->CreateInputLayout(
			inputElementDesc,
			(UINT)std::size(inputElementDesc),
			blobPointer->GetBufferPointer(),
			blobPointer->GetBufferSize(),
			inputLayoutPointer.GetAddressOf()
		);
		if(FAILED(result)){
			throw HResultError{ "Error creating input layout" };
		}
		contextPointer->IASetInputLayout(inputLayoutPointer.Get());
		
		//configure viewport
		D3D11_VIEWPORT viewport{};
		viewport.Width = (float)graphicsWidth;
		viewport.Height = (float)graphicsHeight;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
		contextPointer->RSSetViewports(1, &viewport);
		
		//bind depth stencil state to the output merger
		D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
		depthStencilDesc.DepthEnable = TRUE;
		depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthStencilDesc.DepthFunc = D3D11_COMPARISON_GREATER;
		
		ComPtr<ID3D11DepthStencilState> depthStencilStatePointer{};
		result = devicePointer->CreateDepthStencilState(
			&depthStencilDesc,
			depthStencilStatePointer.GetAddressOf()
		);
		if(FAILED(result)){
			throw HResultError{ "Error creating depth stencil state" };
		}
		contextPointer->OMSetDepthStencilState(
			depthStencilStatePointer.Get(),
			0u
		);
		
		//bind back buffer and depth buffer to the output merger
		contextPointer->OMSetRenderTargets(
			1,
			renderTargetViewPointer.GetAddressOf(),
			depthStencilViewPointer.Get()
		);
	}
	
	void GraphicsWrapper::paint(HWND windowHandle) {
		bufferSwap();
		clearBuffer();
		
		graphics::TextureLoader bitmapLoader{};
		auto framePointer{
			bitmapLoader.getWicFramePointer(L"res\\test.png")
		};
		auto textureViewPointer{
			bitmapLoader.convertWicFrameToD3DTextureView(framePointer, devicePointer)
		};
		contextPointer->PSSetShaderResources(
			0u,
			1u,
			textureViewPointer.GetAddressOf()
		);
		
		const Vertex vertices[] {
			{-0.9f, -0.9f, 0.5f, 0.0f, 1.0f },
			{-0.9f, 0.9f, 0.5f, 0.0f, 0.0f },
			{0.9f, -0.9f, 0.5f, 1.0f, 1.0f },
			{0.9f, 0.9f, 0.5f, 1.0f, 0.0f },
		};
		
		contextPointer->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		
		ComPtr<ID3D11Buffer> vertexBufferPointer{};
		D3D11_BUFFER_DESC bufferDesc{};
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.CPUAccessFlags = 0u;
		bufferDesc.MiscFlags = 0u;
		bufferDesc.ByteWidth = sizeof(vertices);
		bufferDesc.StructureByteStride = sizeof(Vertex);
		D3D11_SUBRESOURCE_DATA initData{};
		initData.pSysMem = vertices;
		devicePointer->CreateBuffer(
			&bufferDesc,
			&initData,
			vertexBufferPointer.GetAddressOf()
		);
		const UINT stride = bufferDesc.StructureByteStride;
		const UINT offset = 0u;
		contextPointer->IASetVertexBuffers(
			0u,
			1u,
			vertexBufferPointer.GetAddressOf(),
			&stride,
			&offset
		);
		contextPointer->Draw(4u, 0u);
	}
	
	void GraphicsWrapper::bufferSwap() {
		swapChainPointer->Present(1u, 0u);
	}
	
	void GraphicsWrapper::clearBuffer() {
		static float color[] { 1.0f, 0.5f, 0.0f, 1.0f };
		contextPointer->ClearRenderTargetView(
			renderTargetViewPointer.Get(),
			color
		);
		contextPointer->ClearDepthStencilView(
			depthStencilViewPointer.Get(),
			D3D11_CLEAR_DEPTH,
			std::numeric_limits<float>::min(),
			0u
		);
	}
	
	void GraphicsWrapper::resize(HWND windowHandle) {
		//todo: windowpainter resize
	}
	
	
	
	
}

/*
 * #include "window\GraphicsWrapper.h"

#include <cmath>

#include "Game\Config.h"
#include "adaptor\HResultError.h"
#include "Logging.h"

namespace wasp::window {

	using windowsadaptor::HResultError;

	GraphicsWrapper::GraphicsWrapper(
		int graphicsWidth,
		int graphicsHeight,
		int fillColor,
		int textColor,
		wchar_t const* fontName,
		float fontSize,
		DWRITE_FONT_WEIGHT fontWeight,
		DWRITE_FONT_STYLE fontStyle,
		DWRITE_FONT_STRETCH fontStretch,
		DWRITE_TEXT_ALIGNMENT textAlignment,
		DWRITE_PARAGRAPH_ALIGNMENT paragraphAlignment
	)
		: d2dFactoryPointer{ nullptr }
		, renderTargetPointer{ nullptr }
		, graphicsWidth{ graphicsWidth }
		, graphicsHeight{ graphicsHeight }
		, fillColor{ fillColor }
		, textColor{ textColor }
		, fontName{ fontName }
		, fontSize{ fontSize }
		, fontWeight{ fontWeight }
		, fontStyle{ fontStyle }
		, fontStretch{ fontStretch }
		, textAlignment{ textAlignment }
		, paragraphAlignment{ paragraphAlignment } {
	}

	void GraphicsWrapper::init(HWND windowHandle) {
		getDeviceIndependentResources();
		getDeviceDependentResources(windowHandle);
	}

	void GraphicsWrapper::getDeviceIndependentResources() {
		getD2dFactoryPointer();
		getTextFormatPointer();
	}

	void GraphicsWrapper::getD2dFactoryPointer() {
		HRESULT result{ D2D1CreateFactory(
			D2D1_FACTORY_TYPE_SINGLE_THREADED,
			&d2dFactoryPointer
		) };
		if (FAILED(result)) {
			throw new HResultError("Error creating Direct2D factory");
		}
	}

	void GraphicsWrapper::getTextFormatPointer() {
		HRESULT result{ getDirectWriteFactoryPointer()->CreateTextFormat(
			fontName,
			NULL,
			fontWeight,
			fontStyle,
			fontStretch,
			fontSize,
			L"", //locale
			&textFormatPointer
		) };
		if (FAILED(result)) {
			throw new HResultError("Error creating text format");
		}

		textFormatPointer->SetTextAlignment(textAlignment);
		textFormatPointer->SetParagraphAlignment(paragraphAlignment);
	}

	CComPtr<IDWriteFactory> GraphicsWrapper::getDirectWriteFactoryPointer() {
		IDWriteFactory* rawPointer{};
		HRESULT result{DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(rawPointer),
			reinterpret_cast<IUnknown**>(&rawPointer)
		) };
		if (FAILED(result)) {
			throw HResultError{ "Error failed to get Direct Write Factory" };
		}
		CComPtr<IDWriteFactory> toRet{};
		toRet.Attach(rawPointer);
		return toRet;
	}

	void GraphicsWrapper::getDeviceDependentResources(HWND windowHandle)
	{
		if (renderTargetPointer == nullptr) {
			getRenderTargetPointer(windowHandle);
			makeBufferRenderTargetPointer();
			makeTextBrushPointer();
		}
	}

	void GraphicsWrapper::getRenderTargetPointer(HWND windowHandle) {
		RECT clientRect;
		GetClientRect(windowHandle, &clientRect);

		D2D1_SIZE_U sizeBytes = D2D1::SizeU(clientRect.right, clientRect.bottom);

		HRESULT result{ d2dFactoryPointer->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(windowHandle, sizeBytes),
			&renderTargetPointer
		) };

		if (FAILED(result)) {
			throw new HResultError("Error creating render target");
		}
	}

	void GraphicsWrapper::makeBufferRenderTargetPointer() {

		HRESULT result{ renderTargetPointer->CreateCompatibleRenderTarget(
			D2D1_SIZE_F{
				static_cast<float>(graphicsWidth),
				static_cast<float>(graphicsHeight)
			},
			&bufferRenderTargetPointer
		) };

		if (FAILED(result)) {
			throw HResultError{ "Error creating buffer render target" };
		}

		bufferRenderTargetPointer->SetTextAntialiasMode(
			D2D1_TEXT_ANTIALIAS_MODE_ALIASED
		);
	}

	void GraphicsWrapper::makeTextBrushPointer() {
		HRESULT result{ bufferRenderTargetPointer->CreateSolidColorBrush(
			D2D1::ColorF{ static_cast<uint32_t>(textColor) },
			&textBrushPointer
		) };
	}

	void GraphicsWrapper::discardDeviceDependentResources() {
		renderTargetPointer = nullptr;
		bufferRenderTargetPointer = nullptr;
		textBrushPointer = nullptr;
	}

	void GraphicsWrapper::paint(HWND windowHandle)
	{
		getDeviceDependentResources(windowHandle);
		PAINTSTRUCT paintStruct;
		BeginPaint(windowHandle, &paintStruct);

		renderTargetPointer->BeginDraw();

		renderTargetPointer->Clear(D2D1::ColorF(D2D1::ColorF::SkyBlue));

		//draw buffer onto window
		if (bufferRenderTargetPointer) {
			D2D1_SIZE_F windowSize = renderTargetPointer->GetSize();

			// Create a rectangle same sizeBytes of current window
			D2D1_RECT_F rectangle = D2D1::RectF(
				0.0f, 0.0f, windowSize.widthBytes, windowSize.heightBytes
			);

			renderTargetPointer->DrawBitmap(
				getBufferBitmap(),
				rectangle
			);
		}

		HRESULT result = renderTargetPointer->EndDraw();
		if (FAILED(result)){
			debug::log("failed EndDraw(), doing nothing");
			//discardDeviceDependentResources();
		}
		if (result == D2DERR_RECREATE_TARGET) {
			debug::log("EndDraw() caused D2DERR_RECREATE_TARGET, discarding device resources");
			discardDeviceDependentResources();
		}
		EndPaint(windowHandle, &paintStruct);
	}

	CComPtr<ID2D1Bitmap> GraphicsWrapper::getBufferBitmap() {
		CComPtr<ID2D1Bitmap> toRet{};
		HRESULT result{ bufferRenderTargetPointer->GetBitmap(&toRet) };
		if (FAILED(result)) {
			throw HResultError{ "Error retrieving buffer bitmap from pointer" };
		}
		return toRet;
	}

	void GraphicsWrapper::resize(HWND windowHandle)
	{
		if (renderTargetPointer != NULL){
			RECT rectangle;
			GetClientRect(windowHandle, &rectangle);

			D2D1_SIZE_U sizeBytes = D2D1::SizeU(rectangle.right, rectangle.bottom);

			renderTargetPointer->Resize(sizeBytes);
			InvalidateRect(windowHandle, NULL, FALSE); //what in the fuck
		}
		else {
			throw new std::exception{ "render target pointer is null in resize" };
		}
	}

	void GraphicsWrapper::beginDraw() {
		bufferRenderTargetPointer->BeginDraw();
		bufferRenderTargetPointer->Clear(D2D1::ColorF{ static_cast<uint32_t>(fillColor) });
	}

	static D2D1::Matrix3x2F makeRotationMatrix(
		float rotationDegrees,
		D2D1_POINT_2F center
	) {
		return D2D1::Matrix3x2F::Rotation(
			rotationDegrees,
			center
		);
	}

	static D2D1::Matrix3x2F makeScaleMatrix(float scale, D2D1_POINT_2F center) {
		return D2D1::Matrix3x2F::Scale(
			scale,
			scale,
			center
		);
	}

	void GraphicsWrapper::drawBitmap(
		const math::Point2 preOffsetCenter,
		const graphics::SpriteDrawInstruction& bitmapDrawInstruction
	) {
		//assume beginDraw has already been called

		math::Point2 center{ preOffsetCenter + bitmapDrawInstruction.getOffset() };

		ID2D1Bitmap& bitmap{ *bitmapDrawInstruction.getBitmap() };
		D2D1_SIZE_F originalSize = bitmap.GetSize();

		//only rotation or both rotation and scale
		if (bitmapDrawInstruction.requiresRotation()) {
			D2D1_POINT_2F d2dCenter{ center.x, center.y };
			D2D1::Matrix3x2F transform = makeRotationMatrix(
				bitmapDrawInstruction.getRotation(),
				d2dCenter
			);
			//both rotation and scale
			if (bitmapDrawInstruction.requiresScale()) {
				transform = transform * makeScaleMatrix(
					bitmapDrawInstruction.getScale(),
					d2dCenter
				);
				float scaledWidth{
					originalSize.widthBytes * bitmapDrawInstruction.getScale()
				};
				float scaledHeight{
					originalSize.heightBytes * bitmapDrawInstruction.getScale()
				};
				const math::Point2& upperLeft{
					center.x - (scaledWidth / 2),
					center.y - (scaledHeight / 2)
				};
				makeTransformBitmapDrawCall(
					bitmap,
					transform,
					upperLeft,
					scaledWidth,
					scaledHeight,
					bitmapDrawInstruction.getOpacity()
				);
			}
			//only rotation
			else {
				const math::Point2& upperLeft{
					center.x - (originalSize.widthBytes / 2),
					center.y - (originalSize.heightBytes / 2)
				};
				makeTransformBitmapDrawCall(
					bitmap,
					transform,
					upperLeft,
					originalSize.widthBytes,
					originalSize.heightBytes,
					bitmapDrawInstruction.getOpacity()
				);
			}
		}
		//only scale
		else if (bitmapDrawInstruction.requiresScale()) {
			D2D1_POINT_2F d2dCenter{ center.x, center.y };
			D2D1::Matrix3x2F transform = makeScaleMatrix(
				bitmapDrawInstruction.getScale(),
				d2dCenter
			);
			float scaledWidth{
					originalSize.widthBytes * bitmapDrawInstruction.getScale()
			};
			float scaledHeight{
				originalSize.heightBytes * bitmapDrawInstruction.getScale()
			};
			const math::Point2& upperLeft{
				center.x - (scaledWidth / 2),
				center.y - (scaledHeight / 2)
			};
			makeTransformBitmapDrawCall(
				bitmap,
				transform,
				upperLeft,
				scaledWidth,
				scaledHeight,
				bitmapDrawInstruction.getOpacity()
			);
		}
		//normal draw call
		else {
			const math::Point2& upperLeft{
				center.x - (originalSize.widthBytes / 2),
				center.y - (originalSize.heightBytes / 2)
			};
			makeBitmapDrawCall(
				bitmap,
				upperLeft,
				originalSize.widthBytes,
				originalSize.heightBytes,
				bitmapDrawInstruction.getOpacity()
			);
		}
	}

	void GraphicsWrapper::drawSubBitmap(
		const math::Point2 preOffsetCenter,
		const graphics::SpriteDrawInstruction& bitmapDrawInstruction,
		const math::Rectangle& sourceRectangle
	) {
		//assume beginDraw has already been called

		math::Point2 center{ preOffsetCenter + bitmapDrawInstruction.getOffset() };

		ID2D1Bitmap& bitmap{ *bitmapDrawInstruction.getBitmap() };
		D2D1_SIZE_F originalSize = { sourceRectangle.widthBytes, sourceRectangle.heightBytes };

		//only rotation or both rotation and scale
		if (bitmapDrawInstruction.requiresRotation()) {
			D2D1_POINT_2F d2dCenter{ center.x, center.y };
			D2D1::Matrix3x2F transform = makeRotationMatrix(
				bitmapDrawInstruction.getRotation(),
				d2dCenter
			);
			//both rotation and scale
			if (bitmapDrawInstruction.requiresScale()) {
				transform = transform * makeScaleMatrix(
					bitmapDrawInstruction.getScale(),
					d2dCenter
				);
				float scaledWidth{
					originalSize.widthBytes * bitmapDrawInstruction.getScale()
				};
				float scaledHeight{
					originalSize.heightBytes * bitmapDrawInstruction.getScale()
				};
				const math::Point2& upperLeft{
					center.x - (scaledWidth / 2),
					center.y - (scaledHeight / 2)
				};
				makeTransformSubBitmapDrawCall(
					bitmap,
					transform,
					upperLeft,
					scaledWidth,
					scaledHeight,
					bitmapDrawInstruction.getOpacity(),
					sourceRectangle
				);
			}
			//only rotation
			else {
				const math::Point2& upperLeft{
					center.x - (originalSize.widthBytes / 2),
					center.y - (originalSize.heightBytes / 2)
				};
				makeTransformSubBitmapDrawCall(
					bitmap,
					transform,
					upperLeft,
					originalSize.widthBytes,
					originalSize.heightBytes,
					bitmapDrawInstruction.getOpacity(),
					sourceRectangle
				);
			}
		}
		//only scale
		else if (bitmapDrawInstruction.requiresScale()) {
			D2D1_POINT_2F d2dCenter{ center.x, center.y };
			D2D1::Matrix3x2F transform = makeScaleMatrix(
				bitmapDrawInstruction.getScale(),
				d2dCenter
			);
			float scaledWidth{
					originalSize.widthBytes * bitmapDrawInstruction.getScale()
			};
			float scaledHeight{
				originalSize.heightBytes * bitmapDrawInstruction.getScale()
			};
			const math::Point2& upperLeft{
				center.x - (scaledWidth / 2),
				center.y - (scaledHeight / 2)
			};
			makeTransformSubBitmapDrawCall(
				bitmap,
				transform,
				upperLeft,
				scaledWidth,
				scaledHeight,
				bitmapDrawInstruction.getOpacity(),
				sourceRectangle
			);
		}
		//normal draw call
		else {
			const math::Point2& upperLeft{
				center.x - (originalSize.widthBytes / 2),
				center.y - (originalSize.heightBytes / 2)
			};
			makeSubBitmapDrawCall(
				bitmap,
				upperLeft,
				originalSize.widthBytes,
				originalSize.heightBytes,
				bitmapDrawInstruction.getOpacity(),
				sourceRectangle
			);
		}
	}

	void GraphicsWrapper::drawText(
		const math::Point2 pos,
		const std::wstring& text,
		const std::pair<float, float> bounds
	) {
		bufferRenderTargetPointer->DrawText(
			text.c_str(),
			text.sizeBytes(),
			textFormatPointer,
			D2D1::RectF(pos.x, pos.y, pos.x + bounds.first, pos.y + bounds.second),
			textBrushPointer
		);
	}

	inline void GraphicsWrapper::makeBitmapDrawCall(
		ID2D1Bitmap& bitmap,
		const math::Point2 upperLeft,
		float scaledWidth,
		float scaledHeight,
		float opacity
	) {
		bufferRenderTargetPointer->DrawBitmap(
			&bitmap,
			D2D1::RectF(
				std::round(upperLeft.x),
				std::round(upperLeft.y),
				std::round(upperLeft.x + scaledWidth),
				std::round(upperLeft.y + scaledHeight)
			),
			opacity,
			game::config::interpolationMode
		);
	}

	inline void GraphicsWrapper::makeTransformBitmapDrawCall(
		ID2D1Bitmap& bitmap,
		const D2D1::Matrix3x2F& transform,
		const math::Point2 upperLeft,
		float scaledWidth,
		float scaledHeight,
		float opacity
	) {
		bufferRenderTargetPointer->SetTransform(transform);
		makeBitmapDrawCall(bitmap, upperLeft, scaledWidth, scaledHeight, opacity);
		bufferRenderTargetPointer->SetTransform(D2D1::Matrix3x2F::Identity());
	}

	inline void GraphicsWrapper::makeSubBitmapDrawCall(
		ID2D1Bitmap& bitmap,
		const math::Point2 upperLeft,
		float scaledWidth,
		float scaledHeight,
		float opacity,
		const math::Rectangle& sourceRectangle
	) {
		bufferRenderTargetPointer->DrawBitmap(
			&bitmap,
			D2D1::RectF(
				upperLeft.x,
				upperLeft.y,
				upperLeft.x + scaledWidth,
				upperLeft.y + scaledHeight
			),
			opacity,
			game::config::interpolationMode,
			D2D1::RectF(
				sourceRectangle.x,
				sourceRectangle.y,
				sourceRectangle.x + sourceRectangle.widthBytes,
				sourceRectangle.y + sourceRectangle.heightBytes
			)
		);
	}

	inline void GraphicsWrapper::makeTransformSubBitmapDrawCall(
		ID2D1Bitmap& bitmap,
		const D2D1::Matrix3x2F& transform,
		const math::Point2 upperLeft,
		float scaledWidth,
		float scaledHeight,
		float opacity,
		const math::Rectangle& sourceRectangle
	) {
		bufferRenderTargetPointer->SetTransform(transform);
		makeSubBitmapDrawCall(
			bitmap,
			upperLeft,
			scaledWidth,
			scaledHeight,
			opacity,
			sourceRectangle
		);
		bufferRenderTargetPointer->SetTransform(D2D1::Matrix3x2F::Identity());
	}

	void GraphicsWrapper::endDraw() {
		bufferRenderTargetPointer->EndDraw();
	}


}
 */