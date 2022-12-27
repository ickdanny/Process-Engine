#ifndef UNICODE
#define UNICODE
#endif

#include <thread>
#include <chrono>
#include <iostream>
#include <fstream>
#include "windowsInclude.h"


#include "GameConfig.h"
 /*
#include "Game\GameLoop.h"
#include "Game\Resources\ResourceMasterStorage.h"
#include "Game\WindowModes.h"
#include "Window\WindowUtil.h"
#include "Window\BaseWindow.h"
#include "Window\MainWindow.h"
#include "Graphics\BitmapConstructor.h"
#include "Graphics\RenderScheduler.h"
#include "Input\KeyInputTable.h"
#include "Sound\MidiHub.h"
#include "Adaptor\ComLibraryGuard.h"
#include "Game/Game.h"
 */
#include "Settings.h"


//debug
#include "ConsoleOutput.h"
#include "Logging.h"

using namespace process;
using namespace process::game;
/*
using window::getPrimaryMonitorInfo;
using window::getWindowBorderWidthPadding;
using window::getWindowBorderHeightPadding;
 */

//forward declarations
void pumpMessages();

#pragma warning(suppress : 28251) //suppress inconsistent annotation warning
int WINAPI WinMain(HINSTANCE instanceHandle, HINSTANCE, PSTR, int windowShowMode) {
    try {
        wasp::debug::initConsoleOutput();
        wasp::debug::log("test");


        //read settings
        wasp::game::Settings settings{
                wasp::game::settings::readOrCreateSettingsFromFile(
                        config::mainConfigPath
                )
        };

         /*
        //init COM
        windowsadaptor::ComLibraryGuard comLibraryGuard{ COINIT_APARTMENTTHREADED };

        //init Resources : WIC graphics
        resources::ResourceMasterStorage resourceMasterStorage{};

        resource::ResourceLoader resourceLoader{
                std::array<resource::Loadable*, 5>{
                        &resourceMasterStorage.directoryStorage,
                        &resourceMasterStorage.manifestStorage,
                        &resourceMasterStorage.bitmapStorage,
                        &resourceMasterStorage.midiSequenceStorage,
                        &resourceMasterStorage.dialogueStorage
                }
        };
        resourceLoader.loadFile({ config::mainManifestPath });

        //init window and Direct 2D
        window::MainWindow window{
                settings.fullscreen ? windowmodes::fullscreen : windowmodes::windowed,
                instanceHandle,
                config::className,
                config::windowName,
                config::graphicsWidth,
                config::graphicsHeight,
                config::fillColor,
                config::textColor,
                config::fontName,
                config::fontSize,
                config::fontWeight,
                config::fontStyle,
                config::fontStretch,
                config::textAlignment,
                config::paragraphAlignment
        };

        //init D2D Bitmaps
        resourceMasterStorage.bitmapStorage.setRenderTargetPointerAndLoadD2DBitmaps(
                window.getWindowPainter().getRenderTargetPointer()
        );

        //init input
        input::KeyInputTable keyInputTable{};
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
                [&] {keyInputTable.allKeysOff(); }
        );

        //init rendering
        graphics::RendererScheduler renderer{
                &window,
                &resourceMasterStorage.bitmapStorage,
                config::graphicsWidth,
                config::graphicsHeight
        };

        //init midi
        sound::midi::MidiHub midiHub{ settings.muted };

        //init game
        Game game{
                &settings,
                &resourceMasterStorage,
                &window.getWindowPainter(),
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
                    settings::writeSettingsToFile(settings, config::mainConfigPath);
                }
        );

        //init gameloop
        graphics::RendererScheduler::RenderCallback renderCallback{
                [&](float deltaTime) {
                    game.render(deltaTime);
                }
        };

        GameLoop gameLoop{
                config::updatesPerSecond,
                config::maxUpdatesWithoutFrame,
                //update function
                [&] {
                    game.update();
                    pumpMessages();
                },
                //draw function
                [&](float deltaTime) {
                    renderer.render(
                            deltaTime,
                            renderCallback
                    );
                }
        };

        auto stopGameLoopCallback{ [&] { gameLoop.stop(); } };

        window.setDestroyCallback(stopGameLoopCallback);
        game.setExitCallback(stopGameLoopCallback);

        //make the game visible and begin running
        window.show(windowShowMode);
        gameLoop.run();

        //after the game has ended, write settings and exit
        settings::writeSettingsToFile(settings, config::mainConfigPath);

         */
#ifdef _DEBUG
        while(true) {
            //spin so can see debug
        }
#endif
        return 0;
    }
#ifdef _DEBUG
        catch (std::exception& exception) {
        wasp::debug::log(exception.what());
        #pragma warning(suppress : 4297)  //if debug, we throw exceptions in main
        throw;
    }
    catch (std::string& str) {
        wasp::debug::log(str);
        #pragma warning(suppress : 4297)  //if debug, we throw exceptions in main
        throw;
    }
    catch (...) {
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
    MSG msg = { };
    while (PeekMessage(
            &msg,
            nullptr,
            0,
            0,
            PM_NOREMOVE
            ))
    {
        int result{ GetMessage(&msg, nullptr, 0, 0) };

        if (result == -1) {
            throw std::runtime_error{ "Error message pump failed to get message" };
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}