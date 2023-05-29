#include "Game/Systems/InputParserSystem.h"

namespace process::game::systems {

	void InputParserSystem::operator()(Scene& scene) {
		switch (scene.getName()){
			case SceneNames::main:
			case SceneNames::difficulty:
			case SceneNames::shot:
			case SceneNames::stage:
			case SceneNames::music:
			case SceneNames::options:
			case SceneNames::pause:
			case SceneNames::continues:
				parseMenuInput(scene);
				break;
			case SceneNames::game:
				parseGameInput(scene);
				break;
			case SceneNames::dialogue:
				parseDialogueInput(scene);
				break;
			case SceneNames::load:
			case SceneNames::credits:
				return;
			default:
				throw std::runtime_error{ "default case reached in InputParserSystem" };
		}
	}

	void InputParserSystem::parseMenuInput(Scene& scene) {
		auto& menuNavigationCommandChannel{ 
			scene.getChannel(SceneTopics::menuNavigationCommands) 
		};
		menuNavigationCommandChannel.clear();

		if (isJustPressed(KeyValues::k_escape) 
			|| isJustPressed(KeyValues::k_x)
			|| isJustPressed(KeyValues::k_period)) {
			menuNavigationCommandChannel.addMessage(MenuNavigationCommands::back);
		}
		if (isJustPressed(KeyValues::k_z) || isJustPressed(KeyValues::k_slash)) {
			menuNavigationCommandChannel.addMessage(MenuNavigationCommands::select);
		}
		if (isJustPressed(KeyValues::k_up) || isJustPressed(KeyValues::k_w)) {
			menuNavigationCommandChannel.addMessage(MenuNavigationCommands::up);
		}
		if (isJustPressed(KeyValues::k_down) || isJustPressed(KeyValues::k_s)) {
			menuNavigationCommandChannel.addMessage(MenuNavigationCommands::down);
		}
		if (isJustPressed(KeyValues::k_left) || isJustPressed(KeyValues::k_a)) {
			menuNavigationCommandChannel.addMessage(MenuNavigationCommands::left);
		}
		if (isJustPressed(KeyValues::k_right) || isJustPressed(KeyValues::k_d)) {
			menuNavigationCommandChannel.addMessage(MenuNavigationCommands::right);
		}

		keyInputTablePointer->lockAll();	//menues have no input transparency
	}

	void InputParserSystem::parseGameInput(Scene& scene) {
		auto& gameCommandChannel{
			scene.getChannel(SceneTopics::gameCommands)
		};
		gameCommandChannel.clear();

		if (isJustPressed(KeyValues::k_escape)) {
			gameCommandChannel.addMessage(GameCommands::pause);
		}
		if (isBeingPressed(KeyValues::k_shift)) {
			gameCommandChannel.addMessage(GameCommands::focus);
		}
		if (isBeingPressed(KeyValues::k_z) || isBeingPressed(KeyValues::k_slash)) {
			gameCommandChannel.addMessage(GameCommands::shoot);
		}
		if (isJustPressed(KeyValues::k_x) || isJustPressed(KeyValues::k_period)) {
			gameCommandChannel.addMessage(GameCommands::bomb);
		}
		if (isBeingPressed(KeyValues::k_up) || isBeingPressed(KeyValues::k_w)) {
			gameCommandChannel.addMessage(GameCommands::up);
		}
		if (isBeingPressed(KeyValues::k_down) || isBeingPressed(KeyValues::k_s)) {
			gameCommandChannel.addMessage(GameCommands::down);
		}
		if (isBeingPressed(KeyValues::k_left) || isBeingPressed(KeyValues::k_a)) {
			gameCommandChannel.addMessage(GameCommands::left);
		}
		if (isBeingPressed(KeyValues::k_right) || isBeingPressed(KeyValues::k_d)) {
			gameCommandChannel.addMessage(GameCommands::right);
		}

		keyInputTablePointer->lockAll();	//game has no input transparency
	}

	void InputParserSystem::parseDialogueInput(Scene& scene) {
		auto& readDialogueFlagChannel{
			scene.getChannel(SceneTopics::readDialogueFlag)
		};
		readDialogueFlagChannel.clear();

		if (isJustPressed(KeyValues::k_z) 
			|| isJustPressed(KeyValues::k_slash)
			|| isBeingPressed(KeyValues::k_control)) {
			readDialogueFlagChannel.addMessage();
		}

		keyInputTablePointer->lock(KeyValues::k_z);
		keyInputTablePointer->lock(KeyValues::k_slash);
		keyInputTablePointer->lock(KeyValues::k_x);
		keyInputTablePointer->lock(KeyValues::k_period);
		keyInputTablePointer->lock(KeyValues::k_control);
		keyInputTablePointer->lock(KeyValues::k_escape);	//simplify things; no pause
	}

	bool InputParserSystem::isJustPressed(KeyValues key) {
		if (keyInputTablePointer->isNotLocked(key)) {
			return keyInputTablePointer->get(key) == KeyState::Press;
		}
		return false;
	}

	bool InputParserSystem::isBeingPressed(KeyValues key) {
		if (keyInputTablePointer->isNotLocked(key)) {
			KeyState keyState{ keyInputTablePointer->get(key) };
			return keyState == KeyState::Press || keyState == KeyState::Down;
		}
		return false;
	}
}