#include "Window/WindowPainter.h"

namespace process::window {
    WindowPainter::WindowPainter(
            int graphicsWidth,
            int graphicsHeight
    )
        : graphicsWidth{ graphicsWidth }
        , graphicsHeight{ graphicsHeight } {
    }

    void WindowPainter::init(HWND windowHandle) {
        //todo: windowpainter init
    }

    void WindowPainter::paint(HWND windowHandle) {
        //todo: windowpainter paint
    }

    void WindowPainter::resize(HWND windowHandle) {
        //todo: windowpainter resize
    }
}

/*
 * #include "window\WindowPainter.h"

#include <cmath>

#include "Game\Config.h"
#include "adaptor\HResultError.h"
#include "Logging.h"

namespace wasp::window {

	using windowsadaptor::HResultError;

	WindowPainter::WindowPainter(
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

	void WindowPainter::init(HWND windowHandle) {
		getDeviceIndependentResources();
		getDeviceDependentResources(windowHandle);
	}

	void WindowPainter::getDeviceIndependentResources() {
		getD2dFactoryPointer();
		getTextFormatPointer();
	}

	void WindowPainter::getD2dFactoryPointer() {
		HRESULT result{ D2D1CreateFactory(
			D2D1_FACTORY_TYPE_SINGLE_THREADED,
			&d2dFactoryPointer
		) };
		if (FAILED(result)) {
			throw new HResultError("Error creating Direct2D factory");
		}
	}

	void WindowPainter::getTextFormatPointer() {
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

	CComPtr<IDWriteFactory> WindowPainter::getDirectWriteFactoryPointer() {
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

	void WindowPainter::getDeviceDependentResources(HWND windowHandle)
	{
		if (renderTargetPointer == nullptr) {
			getRenderTargetPointer(windowHandle);
			makeBufferRenderTargetPointer();
			makeTextBrushPointer();
		}
	}

	void WindowPainter::getRenderTargetPointer(HWND windowHandle) {
		RECT clientRect;
		GetClientRect(windowHandle, &clientRect);

		D2D1_SIZE_U size = D2D1::SizeU(clientRect.right, clientRect.bottom);

		HRESULT result{ d2dFactoryPointer->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(windowHandle, size),
			&renderTargetPointer
		) };

		if (FAILED(result)) {
			throw new HResultError("Error creating render target");
		}
	}

	void WindowPainter::makeBufferRenderTargetPointer() {

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

	void WindowPainter::makeTextBrushPointer() {
		HRESULT result{ bufferRenderTargetPointer->CreateSolidColorBrush(
			D2D1::ColorF{ static_cast<uint32_t>(textColor) },
			&textBrushPointer
		) };
	}

	void WindowPainter::discardDeviceDependentResources() {
		renderTargetPointer = nullptr;
		bufferRenderTargetPointer = nullptr;
		textBrushPointer = nullptr;
	}

	void WindowPainter::paint(HWND windowHandle)
	{
		getDeviceDependentResources(windowHandle);
		PAINTSTRUCT paintStruct;
		BeginPaint(windowHandle, &paintStruct);

		renderTargetPointer->BeginDraw();

		renderTargetPointer->Clear(D2D1::ColorF(D2D1::ColorF::SkyBlue));

		//draw buffer onto window
		if (bufferRenderTargetPointer) {
			D2D1_SIZE_F windowSize = renderTargetPointer->GetSize();

			// Create a rectangle same size of current window
			D2D1_RECT_F rectangle = D2D1::RectF(
				0.0f, 0.0f, windowSize.width, windowSize.height
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

	CComPtr<ID2D1Bitmap> WindowPainter::getBufferBitmap() {
		CComPtr<ID2D1Bitmap> toRet{};
		HRESULT result{ bufferRenderTargetPointer->GetBitmap(&toRet) };
		if (FAILED(result)) {
			throw HResultError{ "Error retrieving buffer bitmap from pointer" };
		}
		return toRet;
	}

	void WindowPainter::resize(HWND windowHandle)
	{
		if (renderTargetPointer != NULL){
			RECT rectangle;
			GetClientRect(windowHandle, &rectangle);

			D2D1_SIZE_U size = D2D1::SizeU(rectangle.right, rectangle.bottom);

			renderTargetPointer->Resize(size);
			InvalidateRect(windowHandle, NULL, FALSE); //what in the fuck
		}
		else {
			throw new std::exception{ "render target pointer is null in resize" };
		}
	}

	void WindowPainter::beginDraw() {
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

	void WindowPainter::drawBitmap(
		const math::Point2 preOffsetCenter,
		const graphics::BitmapDrawInstruction& bitmapDrawInstruction
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
					originalSize.width * bitmapDrawInstruction.getScale()
				};
				float scaledHeight{
					originalSize.height * bitmapDrawInstruction.getScale()
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
					center.x - (originalSize.width / 2),
					center.y - (originalSize.height / 2)
				};
				makeTransformBitmapDrawCall(
					bitmap,
					transform,
					upperLeft,
					originalSize.width,
					originalSize.height,
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
					originalSize.width * bitmapDrawInstruction.getScale()
			};
			float scaledHeight{
				originalSize.height * bitmapDrawInstruction.getScale()
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
				center.x - (originalSize.width / 2),
				center.y - (originalSize.height / 2)
			};
			makeBitmapDrawCall(
				bitmap,
				upperLeft,
				originalSize.width,
				originalSize.height,
				bitmapDrawInstruction.getOpacity()
			);
		}
	}

	void WindowPainter::drawSubBitmap(
		const math::Point2 preOffsetCenter,
		const graphics::BitmapDrawInstruction& bitmapDrawInstruction,
		const math::Rectangle& sourceRectangle
	) {
		//assume beginDraw has already been called

		math::Point2 center{ preOffsetCenter + bitmapDrawInstruction.getOffset() };

		ID2D1Bitmap& bitmap{ *bitmapDrawInstruction.getBitmap() };
		D2D1_SIZE_F originalSize = { sourceRectangle.width, sourceRectangle.height };

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
					originalSize.width * bitmapDrawInstruction.getScale()
				};
				float scaledHeight{
					originalSize.height * bitmapDrawInstruction.getScale()
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
					center.x - (originalSize.width / 2),
					center.y - (originalSize.height / 2)
				};
				makeTransformSubBitmapDrawCall(
					bitmap,
					transform,
					upperLeft,
					originalSize.width,
					originalSize.height,
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
					originalSize.width * bitmapDrawInstruction.getScale()
			};
			float scaledHeight{
				originalSize.height * bitmapDrawInstruction.getScale()
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
				center.x - (originalSize.width / 2),
				center.y - (originalSize.height / 2)
			};
			makeSubBitmapDrawCall(
				bitmap,
				upperLeft,
				originalSize.width,
				originalSize.height,
				bitmapDrawInstruction.getOpacity(),
				sourceRectangle
			);
		}
	}

	void WindowPainter::drawText(
		const math::Point2 pos,
		const std::wstring& text,
		const std::pair<float, float> bounds
	) {
		bufferRenderTargetPointer->DrawText(
			text.c_str(),
			text.size(),
			textFormatPointer,
			D2D1::RectF(pos.x, pos.y, pos.x + bounds.first, pos.y + bounds.second),
			textBrushPointer
		);
	}

	inline void WindowPainter::makeBitmapDrawCall(
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

	inline void WindowPainter::makeTransformBitmapDrawCall(
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

	inline void WindowPainter::makeSubBitmapDrawCall(
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
				sourceRectangle.x + sourceRectangle.width,
				sourceRectangle.y + sourceRectangle.height
			)
		);
	}

	inline void WindowPainter::makeTransformSubBitmapDrawCall(
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

	void WindowPainter::endDraw() {
		bufferRenderTargetPointer->EndDraw();
	}


}
 */