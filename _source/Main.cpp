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
#include "Input\KeyInputTable.h"
#include "Sound\MidiHub.h"
#include "ComLibraryGuard.h"
#include "Game/Game.h"
#include "Settings.h"

//debug
#include "ConsoleOutput.h"
#include "Logging.h"

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
			std::array<wasp::resource::Loadable*, 6> {
				&resourceMasterStorage.directoryStorage,
				&resourceMasterStorage.manifestStorage,
				&resourceMasterStorage.spriteStorage,
				&resourceMasterStorage.midiSequenceStorage,
				&resourceMasterStorage.dialogueStorage,
				&resourceMasterStorage.scriptStorage
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
		
		//init midi
		wasp::sound::midi::MidiHub midiHub { settings.muted };
		
		//init game
		Game game{
				&settings,
				&resourceMasterStorage,
				&window.getGraphicsWrapper(),
				&keyInputTable,
				&midiHub
		};

		game.setUpdateFullscreenCallback(
			[&]() {
				if (settings.fullscreen) {
					window.changeWindowMode(windowmodes::fullscreen);
				}
				else {
					window.changeWindowMode(windowmodes::windowed);
				}
			}
		);

		game.setWriteSettingsCallback(
			[&]() {
				wasp::game::settings::writeSettingsToFile(
					settings,
					config::mainConfigPath
				);
			}
		);
		
		//note: unlike previous engines, no interpolation thus no render scheduler
		game::GameLoop gameLoop {
			config::updatesPerSecond,
			config::maxUpdatesWithoutFrame,
			//update function
			[&] {
				game.update();
				pumpMessages();
			},
			//draw function
			[&]() {
				game.render();
				window.getGraphicsWrapper().present();
			}
		};
		
		auto stopGameLoopCallback { [&] { gameLoop.stop(); } };
		
		window.setDestroyCallback(stopGameLoopCallback);
		game.setExitCallback(stopGameLoopCallback);
		
		//make the game visible and begin running
		window.show(windowShowMode);
		gameLoop.run();
		
		//after the game has ended, write settings and exit
		wasp::game::settings::writeSettingsToFile(settings, config::mainConfigPath);
		return 0;
	}
	#ifdef _DEBUG
	catch( std::exception& exception ) {
		wasp::debug::log(exception.what());
		Sleep(100);
		#pragma warning(suppress : 4297)  //if debug, we throw
		throw;
	}
	catch( std::string& str ) {
		wasp::debug::log(str);
		Sleep(100);
		#pragma warning(suppress : 4297)  //if debug, we throw
		throw;
	}
	catch( ... ) {
		wasp::debug::log("Exception caught in main of unknown type\n");
		Sleep(100);
		#pragma warning(suppress : 4297)  //if debug, we throw
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