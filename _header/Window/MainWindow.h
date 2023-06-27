#pragma once

#include <functional>

#include "Window/BaseWindow.h"
#include "GraphicsWrapper.h"
#include "Window/WindowMode.h"

namespace process::window {
	class MainWindow : public BaseWindow<MainWindow> {
	private:
		
		std::wstring currentWindowModeName {};
		
		//fields
		GraphicsWrapper graphicsWrapper; //uninitialized!
		
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
			int graphicsHeight
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
		
		GraphicsWrapper& getGraphicsWrapper() {
			return graphicsWrapper;
		}
		
		#pragma warning(suppress : 4068) //suppress unknown pragma
		#pragma clang diagnostic push
		#pragma warning(suppress : 4068) //suppress unknown pragma
		#pragma clang diagnostic ignored "-Wshadow"
		
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
		
		#pragma warning(suppress : 4068) //suppress unknown pragma
		#pragma clang diagnostic pop
	};
}