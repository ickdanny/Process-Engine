#pragma once

#include "Graphics/ISpriteDrawer.h"

#include "windowsInclude.h"
#include "d3dInclude.h"

namespace process::window {
	class GraphicsWrapper : public graphics::ISpriteDrawer
	{
	private:
		//typedefs
		using Point2 = wasp::math::Point2;
		using Rectangle = wasp::math::Rectangle;
		template <typename T>
		using ComPtr = Microsoft::WRL::ComPtr<T>;
		using SpriteDrawInstruction = graphics::SpriteDrawInstruction;
		
		struct VSConstantBuffer{
			[[maybe_unused]] //the following IS used by d3d
			DirectX::XMMATRIX transform{};
		};
		
		//fields
		int graphicsWidth {};
		int graphicsHeight {};
		
		ComPtr<ID3D11Device> devicePointer {};
		ComPtr<IDXGISwapChain> swapChainPointer {};
		ComPtr<ID3D11DeviceContext> contextPointer {};
		ComPtr<ID3D11RenderTargetView> renderTargetViewPointer {};
		ComPtr<ID3D11DepthStencilView> depthStencilViewPointer {};
		ComPtr<ID3D11Buffer> VSConstantBufferPointer{};
		
	public:
		GraphicsWrapper(
			int graphicsWidth,
			int graphicsHeight
		);
		~GraphicsWrapper() = default;
		
		void init(HWND windowHandle);
		
		void present();
		
		void clearDepth();
		
		ComPtr<ID3D11Device> getDevicePointer() {
			return devicePointer;
		}
		
		void drawSprite(
			Point2 preOffsetCenter,
			const SpriteDrawInstruction& spriteDrawInstruction
		) override;
		
		void drawSubSprite(
			Point2 preOffsetCenter,
			const SpriteDrawInstruction& spriteDrawInstruction,
			const Rectangle& sourceRectangle
		) override;
		
		void drawTileSprite(
			const Rectangle& drawRectangle,
			const SpriteDrawInstruction& spriteDrawInstruction,
			Point2 pixelOffset
		) override;
		
		struct Vertex{
			//the following ARE used by d3d
			[[maybe_unused]]
			float x{};
			[[maybe_unused]]
			float y{};
			[[maybe_unused]]
			float z{};
			[[maybe_unused]]
			float u{};
			[[maybe_unused]]
			float v{};
		};
		
	private:
		void getDevice(HWND windowHandle);
		void getRenderTargetView();
		void getDepthStencilView();
		
		void setupPipeline();
		ComPtr<ID3DBlob> setVertexShader();
		void setPixelShader();
		void setSampler();
		void setVSConstantBuffer();
		void setInputLayout(const ComPtr<ID3DBlob>& vsBlobPointer);
		void setVertexBuffer(
			float uLow = 0.0f,
			float uHigh = 1.0f,
			float vLow = 0.0f,
			float vHigh = 1.0f
		);
		void setViewport();
		void setDepthStencilState();
		void setBlendState();
		void setRenderTargets();
		
		void bufferSwap();
		void clearBuffer();
		void updatePSTexture(const SpriteDrawInstruction& spriteDrawInstruction);
		[[nodiscard]]
		DirectX::XMMATRIX makeTransform(
			GraphicsWrapper::Point2 preOffsetCenter,
			const SpriteDrawInstruction& spriteDrawInstruction,
			float quadWidthPixels,
			float quadHeightPixels
		) const;
		void mapVSConstantBuffer(const VSConstantBuffer* constantBuffer);
		void updateVSConstantBuffer();
	};
}