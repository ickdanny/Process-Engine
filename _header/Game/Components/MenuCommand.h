#pragma once

#include <utility>
#include <variant>
#include <string>

#include "Game/Scenes.h"
#include "Game/Systems/GameBuilderCommands.h"

namespace process::game::components {
    struct MenuCommand {
    private:
        //typedefs
        using GameBuilderCommands = systems::GameBuilderCommands;
        using DataVariant
            = std::variant<std::tuple<SceneNames, GameBuilderCommands>, std::wstring>;

    public:
        //fields
        enum class Commands {
            navUp,
            navDown,
            navLeft,
            navRight,

            navFarUp,
            navFarDown,
            navFarLeft,
            navFarRight,

            enter,
            enterAndStopMusic,

            backTo,
            backAndWriteSettings,
            backAndSetTrackToMenu,

            startTrack,

            toggleSound,
            toggleFullscreen,

            restartGame,

            gameOver,

            exit,

            none
        } command{};
        DataVariant data{};

        //static constant for no command
        static const MenuCommand none;

        //constructors

        //default constructor
        MenuCommand()
            : command{ Commands::none }
            , data{} {
        }
        
        //full arg constructor
        MenuCommand(Commands command, DataVariant  data)
            : command{ command }
            , data{std::move( data )}{
        }

        //constructs a MenuCommand with the given scene name as data and "none" as the
        //game builder command
        MenuCommand(Commands command, SceneNames sceneName)
            : command{ command }
            , data{ std::tuple{ sceneName, GameBuilderCommands::none } }{
        }

        //constructs a MenuCommand with a default-initialized data member
        MenuCommand(Commands command)
            : command{ command }
            , data{}{
        }
    };
}