#include "Game/Systems/MenuNavigationSystem.h"

namespace process::game::systems {

	void MenuNavigationSystem::operator()(Scene& scene) {

		//if this scene has a selected element (in other words, if this scene is a menu)
		if (scene.hasChannel(SceneTopics::currentSelectedElement)) {
			auto& currentSelectedElementChannel{
				scene.getChannel(SceneTopics::currentSelectedElement)
			};
			if (currentSelectedElementChannel.hasMessages()) {

				//this system is responsible for clearing the elementSelection channel
				auto& elementSelectionChannel{
					scene.getChannel(SceneTopics::elementSelection)
				};
				elementSelectionChannel.clear();

				//if this scene has any menu navigation commands to parse
				auto& menuNavigationCommandChannel{
					scene.getChannel(SceneTopics::menuNavigationCommands)
				};
				if (menuNavigationCommandChannel.hasMessages()) {

					//grab the selected element
					EntityHandle currentSelectedElement{
						currentSelectedElementChannel.getMessages()[0]
					};

					//and start parsing commands
					for (auto menuNavigationCommand
						: menuNavigationCommandChannel.getMessages())
					{
						if (
							!parseNavigationCommand(
								scene,
								menuNavigationCommand,
								currentSelectedElement
							)
							) {
							break;	//break out of loop if we hit a critical command
						}
					}
				}
			}
		}
	}

	bool MenuNavigationSystem::parseNavigationCommand(
		Scene& scene,
		MenuNavigationCommands menuNavigationCommand,
		const EntityHandle& currentSelectedElement
	) {
		DataStorage& dataStorage{ scene.getDataStorage() };

		MenuCommand menuCommand{};
		switch (menuNavigationCommand) {
			case MenuNavigationCommands::back:
				menuCommand = getKeyboardBackMenuCommand(scene);
				break;
			case MenuNavigationCommands::select:
				menuCommand = getMenuCommand<MenuCommandSelect>(
					dataStorage, 
					currentSelectedElement
				);
				break;
			case MenuNavigationCommands::up:
				menuCommand = getMenuCommand<MenuCommandUp>(
					dataStorage,
					currentSelectedElement
				);
				break;
			case MenuNavigationCommands::down:
				menuCommand = getMenuCommand<MenuCommandDown>(
					dataStorage,
					currentSelectedElement
				);
				break;
			case MenuNavigationCommands::left:
				menuCommand = getMenuCommand<MenuCommandLeft>(
					dataStorage,
					currentSelectedElement
				);
				break;
			case MenuNavigationCommands::right:
				menuCommand = getMenuCommand<MenuCommandRight>(
					dataStorage,
					currentSelectedElement
				);
				break;
			default:
				throw std::runtime_error{ "reached default case in MenuNavSystem" };
		}
		return parseMenuCommand(scene, menuCommand, currentSelectedElement);
	}

	const components::MenuCommand& MenuNavigationSystem::getKeyboardBackMenuCommand(
		Scene& scene
	) {
		auto& keyboardBackMenuCommandChannel{
			scene.getChannel(SceneTopics::keyboardBackMenuCommand)
		};
		if (keyboardBackMenuCommandChannel.hasMessages()) {
			return keyboardBackMenuCommandChannel.getMessages()[0];
		}
		return { MenuCommand::none };
	}

	//true if we can continue parsing navigation commands, false if critical
	#pragma warning(suppress : 4715)  //suppress not all paths returning (throw instead)
	bool MenuNavigationSystem::parseMenuCommand(
		Scene& scene,
		const MenuCommand& menuCommand,
		const EntityHandle& currentSelectedElement
	) {
		switch (menuCommand.command) {
			//menu navigation
			case MenuCommand::Commands::navUp:
				handleNavCommand<NeighborElementUp>(scene, currentSelectedElement);
				return true;
			case MenuCommand::Commands::navDown:
				handleNavCommand<NeighborElementDown>(scene, currentSelectedElement);
				return true;
			case MenuCommand::Commands::navLeft:
				handleNavCommand<NeighborElementLeft>(scene, currentSelectedElement);
				return true;
			case MenuCommand::Commands::navRight:
				handleNavCommand<NeighborElementRight>(scene, currentSelectedElement);
				return true;
			case MenuCommand::Commands::navFarUp:
				handleNavFarCommand<NeighborElementUp>(scene, currentSelectedElement);
				return true;
			case MenuCommand::Commands::navFarDown:
				handleNavFarCommand<NeighborElementDown>(scene, currentSelectedElement);
				return true;
			case MenuCommand::Commands::navFarLeft:
				handleNavFarCommand<NeighborElementLeft>(scene, currentSelectedElement);
				return true;
			case MenuCommand::Commands::navFarRight:
				handleNavFarCommand<NeighborElementRight>(
					scene, 
					currentSelectedElement
				);
				return true;

			//scene navigation and variants thereof
			case MenuCommand::Commands::enterAndStopMusic:
				handleStopMusic();
			case MenuCommand::Commands::enter:
				handleEnterCommand(scene, menuCommand);
				return false;

			case MenuCommand::Commands::backAndSetTrackToMenu:
				handleStartTrack(L"01");
			case MenuCommand::Commands::backTo:
				handleBackToCommand(menuCommand);
				return false;
			case MenuCommand::Commands::backAndWriteSettings:
				handleWriteSettings();
				handleBackToCommand(menuCommand);
				return false;

			//miscellaneous functionality
			case MenuCommand::Commands::startTrack:
				handleStartTrack(std::get<std::wstring>(menuCommand.data));
				return true;

			case MenuCommand::Commands::toggleSound:
				handleToggleSoundCommand();
				return true;
			case MenuCommand::Commands::toggleFullscreen:
				handleToggleFullscreenCommand();
				return true;

			case MenuCommand::Commands::restartGame:
				handleRestartGameCommand(scene);
				return false;

			case MenuCommand::Commands::gameOver:
				handleGameOverCommand();
				return false;

			case MenuCommand::Commands::exit:
				handleExitCommand();
				return false;

			case MenuCommand::Commands::none:
				return true;

			default:
				throw std::runtime_error{ "default case in parseMenuCommand" };
		}	//end of switch
	}

	void MenuNavigationSystem::handleStopMusic() {
		globalChannelSetPointer->getChannel(GlobalTopics::stopMusicFlag).addMessage();
	}
	void MenuNavigationSystem::handleEnterCommand(
		Scene& scene,
		const MenuCommand& menuCommand
	) {
		const auto& [sceneName, gameBuilderCommand] 
			= std::get<std::tuple<SceneNames, GameBuilderCommands>>(menuCommand.data);
		auto& sceneEntryChannel{
			globalChannelSetPointer->getChannel(GlobalTopics::sceneEntry)
		};

		//push a scene entry message
		sceneEntryChannel.addMessage(sceneName);

		//push a load screen message if we are going to game
		if (sceneName == SceneNames::game) {
			sceneEntryChannel.addMessage(SceneNames::load);
		}

		//push a game builder message if necessary
		if (gameBuilderCommand != GameBuilderCommands::none) {
			scene.getChannel(SceneTopics::gameBuilderCommands)
				.addMessage(gameBuilderCommand);
		}
	}

	void MenuNavigationSystem::handleBackToCommand(const MenuCommand& menuCommand) {
		SceneNames backTo{
			std::get<0>(
				std::get<std::tuple<SceneNames, GameBuilderCommands>>(
					menuCommand.data
				)
			)
		};
		globalChannelSetPointer->getChannel(GlobalTopics::sceneExitTo)
			.addMessage(backTo);
	}
	void MenuNavigationSystem::handleStartTrack(const std::wstring& trackName) {
		globalChannelSetPointer->getChannel(GlobalTopics::startMusic)
			.addMessage(trackName);
	}
	void MenuNavigationSystem::handleWriteSettings() {
		globalChannelSetPointer->getChannel(GlobalTopics::writeSettingsFlag)
			.addMessage();
	}
	void MenuNavigationSystem::handleRestartGameCommand(Scene& scene) {
		handleStopMusic();
		
		//send us back to the correct menu
		auto& gameStateChannel{
			globalChannelSetPointer->getChannel(GlobalTopics::gameState)
		};
		GameState& gameState{ gameStateChannel.getMessages()[0] };

		SceneNames backTo{};
		switch (gameState.gameMode) {
			case GameMode::campaign:
				backTo = SceneNames::main;
				break;
			case GameMode::practice:
				backTo = SceneNames::stage;
				break;
			default:
				throw std::runtime_error{
					"default case reached in GameOverSystem.gameOver()!"
				};
		}

		globalChannelSetPointer->getChannel(GlobalTopics::sceneExitTo).addMessage(
			backTo
		);

		//have GameBuilderSystem reset the game state
		scene.getChannel(SceneTopics::gameBuilderCommands)
			.addMessage(GameBuilderCommands::reset);

		//then, immediately pop up a new game and loading screen
		auto& sceneEntryChannel{
			globalChannelSetPointer->getChannel(GlobalTopics::sceneEntry)
		};
		sceneEntryChannel.addMessage(SceneNames::game);
		sceneEntryChannel.addMessage(SceneNames::load);
	}
	void MenuNavigationSystem::handleToggleSoundCommand() {
		globalChannelSetPointer->getChannel(GlobalTopics::toggleSoundFlag).addMessage();
	}
	void MenuNavigationSystem::handleToggleFullscreenCommand() {
		globalChannelSetPointer->getChannel(GlobalTopics::toggleFullscreenFlag)
			.addMessage();
	}

	void MenuNavigationSystem::handleGameOverCommand() {
		handleStopMusic();
		handleStartTrack(L"01");

		//send us back to the correct menu
		auto& gameStateChannel{
			globalChannelSetPointer->getChannel(GlobalTopics::gameState)
		};
		GameState& gameState{ gameStateChannel.getMessages()[0] };

		SceneNames backTo{};
		switch (gameState.gameMode) {
			case GameMode::campaign:
				backTo = SceneNames::main;
				break;
			case GameMode::practice:
				backTo = SceneNames::stage;
				break;
			default:
				throw std::runtime_error{
					"default case reached in GameOverSystem.gameOver()!"
				};
		}

		globalChannelSetPointer->getChannel(GlobalTopics::sceneExitTo).addMessage(
			backTo
		);
	}
	void MenuNavigationSystem::handleExitCommand() {
		globalChannelSetPointer->getChannel(GlobalTopics::exitFlag).addMessage();
	}

	bool MenuNavigationSystem::isLockedButton(
		DataStorage& dataStorage,
		const EntityHandle& entity
	) {
		if (dataStorage.containsComponent<ButtonData>(entity)) {
			const ButtonData& buttonData{
				dataStorage.getComponent<ButtonData>(entity)
			};
			return buttonData.locked;
		}
		return false;
	}

	void MenuNavigationSystem::setSelectedElement(
		Scene& scene,
		const EntityHandle& previousSelectedElement,
		const EntityHandle& newSelectedElement
	) {
		//bail out if the two elements are the same somehow
		if (newSelectedElement == previousSelectedElement) {
			return;
		}

		DataStorage& dataStorage{ scene.getDataStorage() };

		//update the stored element
		auto& currentSelectedElementChannel{
			scene.getChannel(SceneTopics::currentSelectedElement)
		};
		currentSelectedElementChannel.clear();
		currentSelectedElementChannel.addMessage(newSelectedElement);

		//send out selection messages
		auto& elementSelectionChannel{ 
			scene.getChannel(SceneTopics::elementSelection) 
		};

		elementSelectionChannel.addMessage({ previousSelectedElement, false });
		elementSelectionChannel.addMessage({ newSelectedElement, true });
	}
}