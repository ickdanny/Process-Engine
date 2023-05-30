#pragma once

#include "systemInclude.h"
#include "Game/Components.h"
#include "MenuNavigationCommands.h"

namespace process::game::systems {

    class MenuNavigationSystem {
    private:
        //typedefs
        using MenuCommand = components::MenuCommand;
        using DataStorage = wasp::ecs::DataStorage;
        using EntityHandle = wasp::ecs::entity::EntityHandle;

        //fields
        wasp::channel::ChannelSet* globalChannelSetPointer{};

    public:
        MenuNavigationSystem(wasp::channel::ChannelSet* globalChannelSetPointer)
            : globalChannelSetPointer{ globalChannelSetPointer } {
        }

        void operator()(Scene& scene);

    private:
        //helper methods
        bool parseNavigationCommand(
            Scene& scene,
            MenuNavigationCommands menuNavigationCommand,
            const EntityHandle& currentSelectedElement
        );

        const MenuCommand& getKeyboardBackMenuCommand(Scene& scene);

        template <typename C>
        const MenuCommand& getMenuCommand(
            const DataStorage& dataStorage,
            const EntityHandle& currentSelectedElement
        ) {
            if (dataStorage.containsComponent<C>(currentSelectedElement)) {
                return dataStorage.getComponent<C>(currentSelectedElement);
            }
            return { MenuCommand::none };
        }

        //true if we can continue parsing navigation commands, false if critical
        bool parseMenuCommand(
            Scene& scene,
            const MenuCommand& menuCommand,
            const EntityHandle& currentSelectedElement
        );

        //handler functions for individual commands
        template <typename C>
        void handleNavCommand(
            Scene& scene, 
            const EntityHandle& currentSelectedElement
        ) {
            DataStorage& dataStorage{ scene.getDataStorage() };
            if (dataStorage.containsComponent<C>(currentSelectedElement)) {
                EntityHandle nextElement{
                    dataStorage.getComponent<C>(currentSelectedElement)
                };
                //go to either the first non-locked button or the last element
                while (
                    isLockedButton(dataStorage, nextElement)
                    && dataStorage.containsComponent<C>(nextElement)
                ) {
                    nextElement = dataStorage.getComponent<C>(nextElement);
                }
                //if it's not locked, set selected (if they are same, will do nothing)
                if (!isLockedButton(dataStorage, nextElement)) {
                    setSelectedElement(scene, currentSelectedElement, nextElement);
                }
            }
        }

        template <typename C>
        void handleNavFarCommand(
            Scene& scene,
            const EntityHandle& currentSelectedElement
        ) {
            DataStorage& dataStorage{ scene.getDataStorage() };
            if (dataStorage.containsComponent<C>(currentSelectedElement)) {
                EntityHandle lastValidElement = currentSelectedElement;
                EntityHandle testElement = lastValidElement;

                //find the last element that isn't a locked button
                do {
                    testElement = dataStorage.getComponent<C>(testElement);
                    if (!isLockedButton(dataStorage, testElement)) {
                        lastValidElement = testElement;
                    }
                } 
                while (dataStorage.containsComponent<C>(testElement));

                setSelectedElement(scene, currentSelectedElement, lastValidElement);
            }
        }

        void handleStopMusic();
        void handleEnterCommand(Scene& scene, const MenuCommand& menuCommand);

        void handleBackToCommand(const MenuCommand& menuCommand);
        void handleStartTrack(const std::wstring& trackName);
        void handleWriteSettings();

        void handleToggleSoundCommand();
        void handleToggleFullscreenCommand();
        void handleRestartGameCommand(Scene& scene);
        void handleGameOverCommand();
        void handleExitCommand();

        bool isLockedButton(DataStorage& dataStorage, const EntityHandle& entity);

        void setSelectedElement(
            Scene& scene, 
            const EntityHandle& previousSelectedElement,
            const EntityHandle& newSelectedElement
        );
    };
}