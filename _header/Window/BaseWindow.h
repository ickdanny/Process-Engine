#pragma once

#ifndef UNICODE
#define UNICODE
#endif

#include "windowsInclude.h" //includes window.h and others
#include <stdexcept>

namespace process::window {
	template <class DerivedClass>
	class BaseWindow {
	
	protected:
		//set in WindowProc : derivedInstancePointer->windowHandle = windowHandle;
		HWND windowHandle;
	
	public:
		static LRESULT CALLBACK WindowProc(
			HWND windowHandle,
			UINT messageCode,
			WPARAM wParam,
			LPARAM lParam
		) {
			DerivedClass* derivedInstancePointer = NULL;
			
			if( messageCode == WM_NCCREATE ) {
				auto* createStructPointer = (CREATESTRUCT*) lParam;
				
				derivedInstancePointer = (DerivedClass*) createStructPointer->lpCreateParams;
				
				SetWindowLongPtr(
					windowHandle,
					GWLP_USERDATA,
					(LONG_PTR) derivedInstancePointer
				);
				
				derivedInstancePointer->windowHandle = windowHandle;
			}
			else {
				derivedInstancePointer
					= (DerivedClass*) GetWindowLongPtr(windowHandle, GWLP_USERDATA);
			}
			
			if( derivedInstancePointer ) {
				return derivedInstancePointer->handleMessage(
					messageCode,
					wParam,
					lParam
				);
			}
			else {
				return DefWindowProc(windowHandle, messageCode, wParam, lParam);
			}
		}
		
		BaseWindow()
			: windowHandle { nullptr } {
		}
		
		virtual ~BaseWindow() = default;
		
		HWND getWindowHandle() const { return windowHandle; }
		
		void show(int windowShowMode) {
			ShowWindow(windowHandle, windowShowMode);
		}
	
	protected:
		void create(
			HINSTANCE instanceHandle,
			PCWSTR className,
			PCWSTR windowName,
			DWORD windowStyle,
			DWORD extraWindowStyle = 0,
			int x = CW_USEDEFAULT,
			int y = CW_USEDEFAULT,
			int width = CW_USEDEFAULT,
			int height = CW_USEDEFAULT,
			HWND parentWindowHandle = nullptr,
			HMENU menuHandle = nullptr
		) {
			registerWindowClass(instanceHandle, className);
			createWindow(
				instanceHandle,
				className,
				windowName,
				windowStyle,
				extraWindowStyle,
				x,
				y,
				width,
				height,
				parentWindowHandle,
				menuHandle
			);
		}
		
		virtual LRESULT handleMessage(
			UINT messageCode,
			WPARAM wParam,
			LPARAM lParam
		) = 0;
	
	private:
		void registerWindowClass(HINSTANCE instanceHandle, const wchar_t* className) {
			
			WNDCLASS windowClass {};
			
			windowClass.lpfnWndProc = DerivedClass::WindowProc;
			windowClass.hInstance = instanceHandle;
			windowClass.lpszClassName = className;
			
			RegisterClass(&windowClass);
		}
		
		#pragma warning(suppress : 4068) //suppress unknown pragma
		#pragma clang diagnostic push
		#pragma warning(suppress : 4068) //suppress unknown pragma
		#pragma clang diagnostic ignored "-Wshadow"
		void createWindow(
			HINSTANCE instanceHandle,
			PCWSTR className,
			PCWSTR windowName,
			DWORD windowStyle,
			DWORD extraWindowStyle,
			int x,
			int y,
			int width,
			int height,
			HWND parentWindowHandle,
			HMENU menuHandle
		) {
			
			HWND windowHandle = CreateWindowEx(
				extraWindowStyle,       // Optional window styles.
				reinterpret_cast<LPCWSTR>(className),              // Window class
				windowName,             // Window text
				windowStyle,            // Window style
				x, y, width, height,    // Size and position
				parentWindowHandle,     // Parent window
				menuHandle,             // Menu
				instanceHandle,         // Instance handle
				this                    // Additional application data
			);
			
			if( !windowHandle ) {
				throw std::runtime_error { "Error creating window handle" };
			}
		}
		#pragma warning(suppress : 4068) //suppress unknown pragma
		#pragma clang diagnostic pop
	};
}