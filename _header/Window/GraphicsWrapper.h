#pragma once

#include "windowsInclude.h"
#include "d3dInclude.h"

namespace process::window {
	class GraphicsWrapper
		//todo: needs to extend bitmap drawer?
	{
	private:
		//typedefs
		template <typename T>
		using ComPtr = Microsoft::WRL::ComPtr<T>;
		
		//fields
		int graphicsWidth {};
		int graphicsHeight {};
		
		ComPtr<ID3D11Device> devicePointer {};
		ComPtr<IDXGISwapChain> swapChainPointer {};
		ComPtr<ID3D11DeviceContext> contextPointer {};
		ComPtr<ID3D11RenderTargetView> renderTargetViewPointer {};
		ComPtr<ID3D11DepthStencilView> depthStencilViewPointer {};
	
	public:
		GraphicsWrapper(
			int graphicsWidth,
			int graphicsHeight
		);
		~GraphicsWrapper() = default;
		
		void init(HWND windowHandle);
		void paint(HWND windowHandle);
		void resize(HWND windowHandle);
		
		struct Vertex{
			float x{};
			float y{};
			float z{};
			float u{};
			float v{};
		};
		
	private:
		void getDevice(HWND windowHandle);
		DXGI_SWAP_CHAIN_DESC makeSwapChainDesc(HWND windowHandle);
		void getRenderTargetView();
		void getDepthStencilView();
		void setupPipeline();
		void bufferSwap();
		void clearBuffer();
	};
}

/*
 * #pragma once

#include "windowsInclude.h"
#include "windowsDWriteInclude.h"
//#include <utility>

#include "Graphics\IBitmapDrawer.h"
#include "Graphics\ITextDrawer.h"

namespace wasp::window {
    class GraphicsWrapper
        : public graphics::IBitmapDrawer
        , public graphics::ITextDrawer
    {
    private:
        CComPtr<ID2D1Factory> d2dFactoryPointer{};
        CComPtr<ID2D1HwndRenderTarget> renderTargetPointer{};

        CComPtr<ID2D1BitmapRenderTarget> bufferRenderTargetPointer{};
        CComPtr<IDWriteTextFormat> textFormatPointer{};
        CComPtr<ID2D1SolidColorBrush> textBrushPointer{};

        int graphicsWidth{};
        int graphicsHeight{};
        int fillColor{};
        int textColor{};
        wchar_t const* fontName{};
        float fontSize{};
        DWRITE_FONT_WEIGHT fontWeight{};
        DWRITE_FONT_STYLE fontStyle{};
        DWRITE_FONT_STRETCH fontStretch{};
        DWRITE_TEXT_ALIGNMENT textAlignment{};
        DWRITE_PARAGRAPH_ALIGNMENT paragraphAlignment{};

    public:
        GraphicsWrapper(
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
        );

        GraphicsWrapperer() = default;

        void init(HWND windowHandle);

        void paint(HWND windowHandle);
        void resize(HWND windowHandle);

        CComPtr<ID2D1HwndRenderTarget> getRenderTargetPointer() {
            return renderTargetPointer;
        }

        void beginDraw() override;

        void drawBitmap(
            const math::Point2 preOffsetCenter,
            const graphics::BitmapDrawInstruction& bitmapDrawInstruction
        ) override;

        void drawSubBitmap(
            const math::Point2 preOffsetCenter,
            const graphics::BitmapDrawInstruction& bitmapDrawInstruction,
            const math::Rectangle& sourceRectangle
        ) override;

        void drawText(
            const math::Point2 pos,
            const std::wstring& text,
            const std::pair<float, float> bounds
        ) override;

        void endDraw() override;

    private:
        inline void makeBitmapDrawCall(
            ID2D1Bitmap& bitmap,
            const math::Point2 upperLeft,
            float scaledWidth,
            float scaledHeight,
            float opacity
        );

        inline void makeTransformBitmapDrawCall(
            ID2D1Bitmap& bitmap,
            const D2D1::Matrix3x2F& transform,
            const math::Point2 upperLeft,
            float scaledWidth,
            float scaledHeight,
            float opacity
        );

        inline void makeSubBitmapDrawCall(
            ID2D1Bitmap& bitmap,
            const math::Point2 upperLeft,
            float scaledWidth,
            float scaledHeight,
            float opacity,
            const math::Rectangle& sourceRectangle
        );

        inline void makeTransformSubBitmapDrawCall(
            ID2D1Bitmap& bitmap,
            const D2D1::Matrix3x2F& transform,
            const math::Point2 upperLeft,
            float scaledWidth,
            float scaledHeight,
            float opacity,
            const math::Rectangle& sourceRectangle
        );

        CComPtr<ID2D1Bitmap> getBufferBitmap();

        void getDeviceIndependentResources();
        void getD2dFactoryPointer();
        void getTextFormatPointer();
        CComPtr<IDWriteFactory> getDirectWriteFactoryPointer();

        void getDeviceDependentResources(HWND windowHandle);
        void getRenderTargetPointer(HWND windowHandle);
        void makeBufferRenderTargetPointer();
        void makeTextBrushPointer();

        void discardDeviceDependentResources();
    };
}
 */