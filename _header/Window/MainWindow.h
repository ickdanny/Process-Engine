#pragma once

#include <functional>

#include "Window/BaseWindow.h"
#include "WindowPainter.h"
#include "Window/WindowMode.h"

namespace process::window {
	class MainWindow : public BaseWindow<MainWindow> {
	private:
		//typedefs
		using WindowMode = wasp::window::WindowMode;
		
		std::wstring currentWindowModeName {};
		
		WindowPainter windowPainter; //uninitialized!
		
		std::function<void()> destroyCallback {};
		std::function<void(WPARAM wParam, LPARAM lParam)> keyDownCallback {};
		std::function<void(WPARAM wParam, LPARAM lParam)> keyUpCallback {};
		std::function<void()> outOfFocusCallback {};
	
	public:
		MainWindow(
			const WindowMode& initWindowMode,
			HINSTANCE instanceHandle,
			PCWSTR className,
			PCWSTR windowName,
			//graphics fields
			int graphicsWidth,
			int graphicsHeight/*,
                int fillColor,
                int textColor,
                wchar_t const* fontName,
                float fontSize/*,
                DWRITE_FONT_WEIGHT fontWeight, //todo: not sure if use dwrite
                DWRITE_FONT_STYLE fontStyle,
                DWRITE_FONT_STRETCH fontStretch,
                DWRITE_TEXT_ALIGNMENT textAlignment,
                DWRITE_PARAGRAPH_ALIGNMENT paragraphAlignment*/
		);
		
		LRESULT handleMessage(
			UINT messageCode,
			WPARAM wParam,
			LPARAM lParam
		) override;
		
		std::wstring& getCurrentWindowModeName() {
			return currentWindowModeName;
		}
		
		void changeWindowMode(const WindowMode& windowMode);
		
		WindowPainter& getWindowPainter() {
			return windowPainter;
		}
		
		void setDestroyCallback(const std::function<void()>& destroyCallback) {
			this->destroyCallback = destroyCallback;
		}
		
		void setKeyDownCallback(
			const std::function<void(WPARAM wParam, LPARAM lParam)>& keyDownCallback
		) {
			this->keyDownCallback = keyDownCallback;
		}
		
		void setKeyUpCallback(
			const std::function<void(WPARAM wParam, LPARAM lParam)>& keyUpCallback
		) {
			this->keyUpCallback = keyUpCallback;
		}
		
		void setOutOfFocusCallback(const std::function<void()>& outOfFocusCallback) {
			this->outOfFocusCallback = outOfFocusCallback;
		}
	};
}