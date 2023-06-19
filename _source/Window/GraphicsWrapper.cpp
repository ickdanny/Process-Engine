#include "Window\GraphicsWrapper.h"

#include "Adaptor\HResultError.h"

#include "Logging.h"

namespace process::window {
	
	namespace{
		using HResultError = wasp::windowsadaptor::HResultError;
		using Point2 = wasp::math::Point2;
		using Vector2 = wasp::math::Vector2;
		
		constexpr Point2 convertPointToNDC(Point2 point, float width, float height){
			float halfWidth = width / 2.0f;
			float halfHeight = height / 2.0f;
			return {
				point.x / halfWidth - 1.0f,
				-(point.y / halfHeight - 1.0f)
			};
		}
		
		constexpr unsigned int numVertices{ 4u };
	}
	
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
		swapChainDesc.Windowed = TRUE;	//always run windowed, even if "fullscreen"
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.Flags = 0;
		
		#ifndef _DEBUG
		UINT flags{ 0u };
		#else
		UINT flags { D3D11_CREATE_DEVICE_DEBUG };
		#endif
		
		D3D_FEATURE_LEVEL featureLevels[]{
			D3D_FEATURE_LEVEL_10_1
		};
		
		HRESULT result{ D3D11CreateDeviceAndSwapChain(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			flags,
			featureLevels,
			sizeof(featureLevels) / sizeof(D3D_FEATURE_LEVEL),
			D3D11_SDK_VERSION,
			&swapChainDesc,
			&swapChainPointer,
			&devicePointer,
			nullptr,
			&contextPointer
		) };
		if( FAILED(result) ) {
			throw HResultError{"Error creating d3d device and swap-chain" };
		}
	}
	
	void GraphicsWrapper::getRenderTargetView() {
		//get pointer to the back buffer
		ComPtr<ID3D11Resource> backBufferPointer{};
		HRESULT result{ swapChainPointer->GetBuffer(
			0,
			__uuidof(ID3D11Resource),
			reinterpret_cast<void**>(backBufferPointer.GetAddressOf())
		) };
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
		
		HRESULT result{ devicePointer->CreateTexture2D(
			&depthDesc,
			nullptr,
			depthStencilPointer.GetAddressOf()
		) };
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
		auto vsBlobPointer{ setVertexShader() };
		setPixelShader();
		setSampler();
		setVSConstantBuffer();
		setInputLayout(vsBlobPointer);
		setVertexBuffer();
		setViewport();
		setDepthStencilState();
		setBlendState();
		setRenderTargets();
	}
	
	GraphicsWrapper::ComPtr<ID3DBlob> GraphicsWrapper::setVertexShader(){
		ComPtr<ID3DBlob> blobPointer{};
		ComPtr<ID3D11VertexShader> vsPointer{};
		D3DReadFileToBlob(L"VertexShader.cso", &blobPointer);
		HRESULT result{ devicePointer->CreateVertexShader(
			blobPointer->GetBufferPointer(),
			blobPointer->GetBufferSize(),
			nullptr,
			vsPointer.GetAddressOf()
		) };
		if(FAILED(result)){
			throw HResultError{ "Error creating vertex shader" };
		}
		contextPointer->VSSetShader(
			vsPointer.Get(),
			nullptr,
			0
		);
		return blobPointer;
	}
	
	void GraphicsWrapper::setPixelShader(){
		ComPtr<ID3DBlob> blobPointer{};
		ComPtr<ID3D11PixelShader> psPointer{};
		D3DReadFileToBlob(L"PixelShader.cso", &blobPointer);
		HRESULT result{ devicePointer->CreatePixelShader(
			blobPointer->GetBufferPointer(),
			blobPointer->GetBufferSize(),
			nullptr,
			psPointer.GetAddressOf()
		) };
		if(FAILED(result)){
			throw HResultError{ "Error creating pixel shader" };
		}
		contextPointer->PSSetShader(
			psPointer.Get(),
			nullptr,
			0
		);
	}
	
	void GraphicsWrapper::setSampler(){
		D3D11_SAMPLER_DESC samplerDesc{};
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		ComPtr<ID3D11SamplerState> samplerStatePointer{};
		HRESULT result{ devicePointer->CreateSamplerState(
			&samplerDesc,
			samplerStatePointer.GetAddressOf()
		) };
		if(FAILED(result)){
			throw HResultError{ "Failed to create sampler state" };
		}
		contextPointer->PSSetSamplers(
			0u,
			1u,
			samplerStatePointer.GetAddressOf()
		);
	}
	
	void GraphicsWrapper::setVSConstantBuffer(){
		D3D11_BUFFER_DESC cbDesc{};
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.Usage = D3D11_USAGE_DYNAMIC;
		cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbDesc.MiscFlags = 0u;
		cbDesc.ByteWidth = sizeof(VSConstantBuffer);
		cbDesc.StructureByteStride = 0u;
		HRESULT result{ devicePointer->CreateBuffer(
			&cbDesc,
			nullptr,
			VSConstantBufferPointer.GetAddressOf()
		) };
		if(FAILED(result)){
			throw HResultError{ "failed to create VS constant buffer" };
		}
		updateVSConstantBuffer();
	}
	
	void GraphicsWrapper::setInputLayout(const ComPtr<ID3DBlob>& vsBlobPointer){
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
		HRESULT result{ devicePointer->CreateInputLayout(
			inputElementDesc,
			(UINT)std::size(inputElementDesc),
			vsBlobPointer->GetBufferPointer(),
			vsBlobPointer->GetBufferSize(),
			inputLayoutPointer.GetAddressOf()
		) };
		if(FAILED(result)){
			throw HResultError{ "Error creating input layout" };
		}
		contextPointer->IASetInputLayout(inputLayoutPointer.Get());
	}
	
	void GraphicsWrapper::setVertexBuffer(float uLow, float uHigh, float vLow, float vHigh) {
		static constexpr float baseDepth{ 0.5f };
		const Vertex vertices[] {
			{-1.0f, -1.0f, baseDepth, uLow, vHigh },
			{-1.0f, 1.0f, baseDepth, uLow, vLow },
			{1.0f, -1.0f, baseDepth, uHigh, vHigh },
			{1.0f, 1.0f, baseDepth, uHigh, vLow },
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
		HRESULT result{ devicePointer->CreateBuffer(
			&bufferDesc,
			&initData,
			vertexBufferPointer.GetAddressOf()
		) };
		if(FAILED(result)){
			throw HResultError{ "Failed to create vertex buffer!" };
		}
		const UINT stride = bufferDesc.StructureByteStride;
		const UINT offset = 0u;
		contextPointer->IASetVertexBuffers(
			0u,
			1u,
			vertexBufferPointer.GetAddressOf(),
			&stride,
			&offset
		);
	}
	
	void GraphicsWrapper::setViewport(){
		D3D11_VIEWPORT viewport{};
		viewport.Width = (float)graphicsWidth;
		viewport.Height = (float)graphicsHeight;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;	//viewport depth ranges from 0.0 to 1.0
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
		contextPointer->RSSetViewports(1, &viewport);
	}
	
	void GraphicsWrapper::setDepthStencilState() {
		D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
		depthStencilDesc.DepthEnable = TRUE;
		depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthStencilDesc.DepthFunc = D3D11_COMPARISON_GREATER;
		
		ComPtr<ID3D11DepthStencilState> depthStencilStatePointer{};
		HRESULT result{ devicePointer->CreateDepthStencilState(
			&depthStencilDesc,
			depthStencilStatePointer.GetAddressOf()
		) };
		if(FAILED(result)){
			throw HResultError{ "Error creating depth stencil state" };
		}
		contextPointer->OMSetDepthStencilState(
			depthStencilStatePointer.Get(),
			0u
		);
	}
	
	void GraphicsWrapper::setBlendState(){
		D3D11_BLEND_DESC blendStateDesc{};
		blendStateDesc.AlphaToCoverageEnable = false;
		blendStateDesc.IndependentBlendEnable = false;
		auto& renderTargetBlendDesc{
			blendStateDesc.RenderTarget[0]
		};
		renderTargetBlendDesc.BlendEnable = true;
		renderTargetBlendDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
		renderTargetBlendDesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		renderTargetBlendDesc.BlendOp = D3D11_BLEND_OP_ADD;
		renderTargetBlendDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
		renderTargetBlendDesc.DestBlendAlpha = D3D11_BLEND_ONE;
		renderTargetBlendDesc.BlendOpAlpha = D3D11_BLEND_OP_MAX;
		renderTargetBlendDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		
		ComPtr<ID3D11BlendState> blendStatePointer{};
		HRESULT result{ devicePointer->CreateBlendState(
			&blendStateDesc,
			blendStatePointer.GetAddressOf()
		) };
		if(FAILED(result)){
			throw HResultError{ "Failed to create blend state" };
		}
		
		contextPointer->OMSetBlendState(
			blendStatePointer.Get(),
			nullptr,
			0xffffffff	//default value
		);
	}
	
	void GraphicsWrapper::setRenderTargets(){
		contextPointer->OMSetRenderTargets(
			1,
			renderTargetViewPointer.GetAddressOf(),
			depthStencilViewPointer.Get()
		);
	}
	
	void GraphicsWrapper::present() {
		bufferSwap();
		clearBuffer();
	}
	
	void GraphicsWrapper::clearDepth(){
		contextPointer->ClearDepthStencilView(
			depthStencilViewPointer.Get(),
			D3D11_CLEAR_DEPTH,
			std::numeric_limits<float>::min(),
			0u
		);
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
		clearDepth();
	}
	
	void GraphicsWrapper::drawSprite(
		const Point2 preOffsetCenter,
		const SpriteDrawInstruction& spriteDrawInstruction
	) {
		updatePSTexture(spriteDrawInstruction);
		
		VSConstantBuffer constantBuffer{makeTransform(
			preOffsetCenter,
			spriteDrawInstruction,
			static_cast<float>(spriteDrawInstruction.getSprite().width),
			static_cast<float>(spriteDrawInstruction.getSprite().height)
		) };
		mapVSConstantBuffer(&constantBuffer);
		
		contextPointer->Draw(numVertices, 0u);
	}
	
	void GraphicsWrapper::drawSubSprite(
		const Point2 preOffsetCenter,
		const SpriteDrawInstruction& spriteDrawInstruction,
		const Rectangle& sourceRectangle
	) {
		//if width and height are small, draw nothing
		if(sourceRectangle.width < 0.5f && sourceRectangle.height < 0.5f){
			return;
		}
		
		//change texture coordinates : recall that they range from 0 to 1
		float fullWidth{ static_cast<float>(spriteDrawInstruction.getSprite().width) };
		float fullHeight{ static_cast<float>(spriteDrawInstruction.getSprite().height) };
		float uLow{ sourceRectangle.x / fullWidth };
		float uHigh{ (sourceRectangle.x + sourceRectangle.width) / fullWidth };
		float vLow{ sourceRectangle.y / fullHeight };
		float vHigh{ (sourceRectangle.y + sourceRectangle.height) / fullHeight };
		setVertexBuffer(uLow, uHigh, vLow, vHigh);
		
		//find the new center for the quad
		Point2 subCenter{
			sourceRectangle.x + sourceRectangle.width / 2.0f,
			sourceRectangle.y + sourceRectangle.height / 2.0f
		};
		Point2 fullCenter{fullWidth / 2.0f, fullHeight / 2.0f};
		Vector2 subOffset{
			wasp::math::vectorFromAToB(fullCenter, subCenter)
		};
		Point2 newPreOffsetCenter{ preOffsetCenter + subOffset };
		
		//draw sprite at that new center and with updated texture coords
		updatePSTexture(spriteDrawInstruction);
		
		VSConstantBuffer constantBuffer{makeTransform(
			newPreOffsetCenter,
			spriteDrawInstruction,
			sourceRectangle.width,
			sourceRectangle.height
		) };
		mapVSConstantBuffer(&constantBuffer);
		
		contextPointer->Draw(numVertices, 0u);
		
		//historically, sub-sprite calls have been rare, so change texture coords back
		setVertexBuffer();
	}
	
	void GraphicsWrapper::updatePSTexture(const SpriteDrawInstruction& spriteDrawInstruction) {
		contextPointer->PSSetShaderResources(
			0u,
			1u,
			spriteDrawInstruction.getSprite().textureView.GetAddressOf()
		);
	}
	
	#pragma warning(suppress : 4068) //suppress unknown pragma
	#pragma clang diagnostic push
	#pragma warning(suppress : 4068) //suppress unknown pragma
	#pragma clang diagnostic ignored "-Wshadow"
	DirectX::XMMATRIX GraphicsWrapper::makeTransform(
		const Point2 preOffsetCenter,
		const SpriteDrawInstruction& spriteDrawInstruction,
		float quadWidthPixels,
		float quadHeightPixels
	) const {
		
		float graphicsWidth{ static_cast<float>(this->graphicsWidth) };
		float graphicsHeight{ static_cast<float>(this->graphicsHeight) };
		
		float aspect{ graphicsWidth / graphicsHeight };
		
		float widthScale{
			quadWidthPixels / graphicsWidth
		};
		float heightScale{
			quadHeightPixels / graphicsHeight
		};
		
		Point2 offsetCenter{ preOffsetCenter + spriteDrawInstruction.getOffset() };
		Point2 centerNDC{ convertPointToNDC(
			offsetCenter,
			graphicsWidth,
			graphicsHeight)
		};
		
		static constexpr float depthRange{
			SpriteDrawInstruction::maxDepth - SpriteDrawInstruction::minDepth + 1
		};
		float depthShift{ static_cast<float>(spriteDrawInstruction.getDepth()) / depthRange };

		return DirectX::XMMatrixTranspose(
			//reverse aspect correction
			//scale by sprite dimensions
			//scale by scale factor
			DirectX::XMMatrixScaling(
				aspect * widthScale * spriteDrawInstruction.getScale(),
				heightScale * spriteDrawInstruction.getScale(),
				1.0f
			) *
			//rotate on xy plane
			DirectX::XMMatrixRotationZ(
				spriteDrawInstruction.getRotation()
			) *
			//aspect correction
			DirectX::XMMatrixScaling(
				1.0f / aspect,
				1.0f,
				1.0f
			) *
			//translation last
			DirectX::XMMatrixTranslation(
				centerNDC.x,
				centerNDC.y,
				depthShift
			)
		);
	}
	#pragma warning(suppress : 4068) //suppress unknown pragma
	#pragma clang diagnostic pop
	
	void GraphicsWrapper::mapVSConstantBuffer(const VSConstantBuffer* const constantBuffer) {
		//disable GPU access
		D3D11_MAPPED_SUBRESOURCE mappedResource{};
		contextPointer->Map(
			VSConstantBufferPointer.Get(),
			0,
			D3D11_MAP_WRITE_DISCARD,
			0,
			&mappedResource
		);
		
		//update buffer
		memcpy(
			mappedResource.pData,
			constantBuffer,
			sizeof(VSConstantBuffer)
		);
		
		//re-enable GPU access
		contextPointer->Unmap(VSConstantBufferPointer.Get(), 0);
	}
	
	void GraphicsWrapper::updateVSConstantBuffer() {
		contextPointer->VSSetConstantBuffers(
			0u,
			1u,
			VSConstantBufferPointer.GetAddressOf()
		);
	}
}