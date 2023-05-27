#ifndef UNICODE
#define UNICODE
#endif

#include <thread>
#include <chrono>
#include <iostream>
#include <fstream>
#include "windowsInclude.h"

#include "MainConfig.h"

#include "Game\GameLoop.h"
#include "Game\Resources\ResourceMasterStorage.h"
#include "Game\WindowModes.h"
#include "Window\WindowUtil.h"
#include "Window\BaseWindow.h"
#include "Window\MainWindow.h"
/*
#include "Graphics\BitmapConstructor.h"
#include "Graphics\RenderScheduler.h"
 */
#include "Input\KeyInputTable.h"
#include "Sound\MidiHub.h"
#include "ComLibraryGuard.h"
//#include "Game/Game.h"
#include "Settings.h"

//debug
#include "ConsoleOutput.h"
#include "Logging.h"

//todo:temp
#include "Math/Point2.h"
#include "Graphics/SpriteDrawInstruction.h"

using namespace process;
using namespace process::game;

using wasp::window::getPrimaryMonitorInfo;
using wasp::window::getWindowBorderWidthPadding;
using wasp::window::getWindowBorderHeightPadding;

//forward declarations
void pumpMessages();

#pragma warning(suppress : 28251) //suppress inconsistent annotation warning

int WINAPI WinMain(HINSTANCE instanceHandle, HINSTANCE, PSTR, int windowShowMode) {
	try {
		wasp::debug::initConsoleOutput();
		
		//read settings
		wasp::game::Settings settings {
			wasp::game::settings::readOrCreateSettingsFromFile(
				config::mainConfigPath
			)
		};
		
		//init COM
		wasp::windowsadaptor::ComLibraryGuard comLibraryGuard {
			tagCOINIT::COINIT_APARTMENTTHREADED
		};
		
		//init Resources
		resources::ResourceMasterStorage resourceMasterStorage {};
		
		resource::ResourceLoader resourceLoader {
			std::array<wasp::resource::Loadable*, 5> {
				&resourceMasterStorage.directoryStorage,
				&resourceMasterStorage.manifestStorage,
				&resourceMasterStorage.spriteStorage,
				&resourceMasterStorage.midiSequenceStorage,
				&resourceMasterStorage.dialogueStorage
			}
		};
		resourceLoader.loadFile({ config::mainManifestPath });
		
		//init window
		window::MainWindow window {
			settings.fullscreen ?
			windowmodes::fullscreen : windowmodes::windowed,
			instanceHandle,
			config::className,
			config::windowName,
			config::graphicsWidth,
			config::graphicsHeight
		};
		
		//init d3d textures
		resourceMasterStorage.spriteStorage.setDevicePointerAndLoadD3DTextures(
			window.getGraphicsWrapper().getDevicePointer()
		);

		//init input
		wasp::input::KeyInputTable keyInputTable {};
		window.setKeyDownCallback(
			[&](WPARAM wParam, LPARAM lParam) {
				keyInputTable.handleKeyDown(wParam, lParam);
			}
		);
		window.setKeyUpCallback(
			[&](WPARAM wParam, LPARAM lParam) {
				keyInputTable.handleKeyUp(wParam, lParam);
			}
		);
		window.setOutOfFocusCallback(
			[&] { keyInputTable.allKeysOff(); }
		);
		
		/*

		//init rendering
		graphics::RendererScheduler renderer{
				&window,
				&resourceMasterStorage.spriteStorage,
				config::graphicsWidth,
				config::graphicsHeight
		};

		 */
		
		//init midi
		wasp::sound::midi::MidiHub midiHub { settings.muted };
		
		/*
		//init Game
		Game Game{
				&settings,
				&resourceMasterStorage,
				&window.getGraphicsWrapper(),
				&keyInputTable,
				&midiHub
		};

		Game.setUpdateFullscreenCallback(
				[&]() {
					if (settings.fullscreen) {
						window.changeWindowMode(windowmodes::fullscreen);
					}
					else {
						window.changeWindowMode(windowmodes::windowed);
					}
				}
		);

		Game.setWriteSettingsCallback(
				[&]() {
					settings::writeSettingsToFile(settings, config::mainConfigPath);
				}
		);

		//init gameloop
		graphics::RendererScheduler::RenderCallback renderCallback{
				[&](float deltaTime) {
					Game.render(deltaTime);
				}
		};
		 */
		float tick{ 0.0f };
		
		game::GameLoop gameLoop {
			config::updatesPerSecond,
			config::maxUpdatesWithoutFrame,
			//update function
			[&] {
				//Game.update();
				pumpMessages();
			},
			//draw function
			[&]() {
				//todo: for now don't worry about render scheduler, just draw same thread
				wasp::math::Point2 point{
					config::graphicsWidth/2,
					config::graphicsHeight/2
				};
				auto frameAndSpritePointer{
					resourceMasterStorage.spriteStorage.get(L"test")
				};
				graphics::SpriteDrawInstruction drawInstruction{
					frameAndSpritePointer->sprite,
					wasp::math::Vector2{ 0.0f, 0.0f},
					static_cast<float>(tick),
					1.0f,
					1.0f
				};
				window.getGraphicsWrapper().drawSprite(
					point,
					drawInstruction
				);
				/*
				renderer.render(
						deltaTime,
						renderCallback
				);
				 */
				window.getGraphicsWrapper().present();
				tick += 0.01f;
			}
		};
		
		auto stopGameLoopCallback { [&] { gameLoop.stop(); } };
		
		window.setDestroyCallback(stopGameLoopCallback);
		//Game.setExitCallback(stopGameLoopCallback);
		
		//make the Game visible and begin running
		window.show(windowShowMode);
		gameLoop.run();
		
		//after the Game has ended, write settings and exit
		wasp::game::settings::writeSettingsToFile(settings, config::mainConfigPath);
		return 0;
	}
	#ifdef _DEBUG
	catch( std::exception& exception ) {
		wasp::debug::log(exception.what());
		#pragma warning(suppress : 4297)  //if debug, we throw exceptions in main
		throw;
	}
	catch( std::string& str ) {
		wasp::debug::log(str);
		#pragma warning(suppress : 4297)  //if debug, we throw exceptions in main
		throw;
	}
	catch( ... ) {
		wasp::debug::log("Exception caught in main of unknown type\n");
		#pragma warning(suppress : 4297)  //if debug, we throw exceptions in main
		throw;
	}
	#else
	catch (...) {
		std::exit(1);
	}
	#endif
}

void pumpMessages() {
	MSG msg = {};
	while( PeekMessage(
		&msg,
		nullptr,
		0,
		0,
		PM_NOREMOVE
	) ) {
		int result {
			GetMessage(&msg, nullptr, 0, 0)
		};
		
		if( result == -1 ) {
			throw std::runtime_error { "Error message pump failed to get message" };
		}
		
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}