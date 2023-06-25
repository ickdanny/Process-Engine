#include "Game/Topics.h"

namespace process::game {
	
	//global topics
	const Topic<SceneNames> GlobalTopics::sceneEntry{};
	const Topic<SceneNames> GlobalTopics::sceneExitTo{};
	const Topic<std::wstring> GlobalTopics::startMusic{};
	const Topic<> GlobalTopics::stopMusicFlag{};
	const Topic<> GlobalTopics::writeSettingsFlag{};
	const Topic<> GlobalTopics::toggleSoundFlag{};
	const Topic<> GlobalTopics::toggleFullscreenFlag{};
	const Topic<> GlobalTopics::exitFlag{};
	const Topic<std::wstring> GlobalTopics::startDialogue{};
	const Topic<> GlobalTopics::endDialogueFlag{};
	const Topic<systems::GameState> GlobalTopics::gameState{};
	const Topic<components::PlayerData> GlobalTopics::playerData{};

	//scene topics
	const Topic<systems::MenuNavigationCommands> 
		SceneTopics::menuNavigationCommands{};
	const Topic<components::MenuCommand> SceneTopics::keyboardBackMenuCommand{};
	const Topic<wasp::ecs::entity::EntityHandle> SceneTopics::currentSelectedElement{};
	const Topic<std::tuple<wasp::ecs::entity::EntityHandle, bool>>
		SceneTopics::elementSelection{};
	const Topic<systems::GameBuilderCommands> SceneTopics::gameBuilderCommands{};
	const Topic<> SceneTopics::readDialogueFlag{};
	const Topic<config::PrngType> SceneTopics::random{};
	const Topic<systems::GameCommands> SceneTopics::gameCommands{};
	const Topic<std::tuple<wasp::ecs::entity::EntityHandle, systems::PlayerStates>>
		SceneTopics::playerStateEntry{};
	const Topic<wasp::ecs::entity::EntityHandle> SceneTopics::playerHits{};
	const Topic<wasp::ecs::entity::EntityHandle> SceneTopics::deaths{};
	const Topic<wasp::ecs::entity::EntityHandle> SceneTopics::bossDeaths{};
	const Topic<> SceneTopics::clearFlag{};
	const Topic<> SceneTopics::pauseFlag{};
	const Topic<> SceneTopics::winFlag{};
}