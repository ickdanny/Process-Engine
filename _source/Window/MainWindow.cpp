#include "Window\MainWindow.h"

#include "Logging.h"

namespace process::window {
	
	MainWindow::MainWindow(
		const WindowMode& initWindowMode,
		HINSTANCE instanceHandle,
		PCWSTR className,
		PCWSTR windowName,
		int graphicsWidth,
		int graphicsHeight
	)
		: currentWindowModeName { initWindowMode.modeName }
		, graphicsWrapper { graphicsWidth, graphicsHeight }
	{
		std::pair<int, int> size { initWindowMode.sizeFunction() };
		std::pair<int, int> position { initWindowMode.positionFunction(size) };
		
		create(
			instanceHandle,
			className,
			windowName,
			initWindowMode.windowStyle,
			initWindowMode.windowExtraStyle,
			position.first,
			position.second,
			size.first,
			size.second
		);
	}
	
	LRESULT MainWindow::handleMessage(UINT messageCode, WPARAM wParam, LPARAM lParam) {
		switch( messageCode ) {
			case WM_CREATE: // gets recieved before main exits window.create
				graphicsWrapper.init(windowHandle);
				return 0;
				
			case WM_PAINT: {
				//dummy paint handling - d3d takes care of presenting frames
				PAINTSTRUCT paintStruct{};
				BeginPaint(windowHandle, &paintStruct);
				EndPaint(windowHandle, &paintStruct);
				return 0;
			}
			//do not handle WM_SIZE
			
			case WM_KEYDOWN:
				keyDownCallback(wParam, lParam);
				return 0;
			
			case WM_KEYUP:
				keyUpCallback(wParam, lParam);
				return 0;
			
			case WM_KILLFOCUS:
			case WM_ENTERSIZEMOVE:
				outOfFocusCallback();
				return 0;
			
			case WM_DESTROY:
				destroyCallback();
				PostQuitMessage(0);
				return 0;
			
			default:
				return DefWindowProc(windowHandle, messageCode, wParam, lParam);
		}
	}
	
	void MainWindow::changeWindowMode(const WindowMode& windowMode) {
		if( currentWindowModeName != windowMode.modeName ) {
			currentWindowModeName = windowMode.modeName;
			
			std::pair<int, int> size { windowMode.sizeFunction() };
			std::pair<int, int> position { windowMode.positionFunction(size) };
			
			SetWindowLong(
				windowHandle,
				GWL_STYLE,
				windowMode.windowStyle
			);
			SetWindowLong(
				windowHandle,
				GWL_EXSTYLE,
				windowMode.windowExtraStyle
			);
			SetWindowPos(
				windowHandle,
				nullptr,
				position.first,
				position.second,
				size.first,
				size.second,
				SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_FRAMECHANGED
			);
		}
	}
}