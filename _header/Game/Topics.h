#pragma once

#include "MainConfig.h"
#include "Channel/Topic.h"
#include "Game/Components/MenuCommand.h"
#include "Game/Systems/MenuNavigationCommands.h"
#include "ECS/Entity/EntityHandle.h"
#include "Game/Systems/GameState.h"
#include "Game/Systems/GameCommands.h"
#include "Game/Systems/PlayerStates.h"
#include "Game/Components/PlayerData.h"
#include "Point2.h"

namespace process::game {

	//forward declarations
	enum class SceneNames;

	//typedefs
	template <typename T = wasp::utility::Void>
	using Topic = wasp::channel::Topic<T>;

	//global topics (1 channel per game)
	struct GlobalTopics {
		//handled by Game::updateSceneList
		static const Topic<SceneNames> sceneEntry;
		static const Topic<SceneNames> sceneExitTo;

		//handled by Game::updateMusic
		static const Topic<std::wstring> startMusic;
		static const Topic<> stopMusicFlag;

		//handled by Game::updateSettings
		static const Topic<> writeSettingsFlag;
		static const Topic<> toggleSoundFlag;
		static const Topic<> toggleFullscreenFlag;

		//handled by Game::updateSceneList
		static const Topic<> exitFlag;

		//set by ScriptSystem, cleared by DialogueSystem
		static const Topic<std::wstring> startDialogue;
		//set by DialogueSystem, cleared by ScriptSystem
		static const Topic<> endDialogueFlag;

		//set by GameBuilderSystem; persistent
		static const Topic<systems::GameState> gameState;

		//set and cleared by ContinueSystem and ScriptSystem (for moving stages)
		//cleared by InitSystem if moving stages
		static const Topic<components::PlayerData> playerData;
	};

	//scene topics (1 channel per scene)
	struct SceneTopics {
	private:
		using EntityHandle = wasp::ecs::entity::EntityHandle;

	public:
		//set and cleared by InputParserSystem
		static const Topic<systems::MenuNavigationCommands> menuNavigationCommands;

		//set by InitSystem; persistent
		static const Topic<components::MenuCommand> keyboardBackMenuCommand;

		//set and cleared by MenuNavigationSystem; init by InitSystem
		static const Topic<EntityHandle> currentSelectedElement;

		//set and cleared by MenuNavigationSystem
		//true = select, false = unselect
		static const Topic<std::tuple<EntityHandle, bool>>
			elementSelection;

		//set by MenuNavigationSystem, cleared by GameBuilderSystem
		static const Topic<systems::GameBuilderCommands> gameBuilderCommands;

		//set and cleared by InputParserSystem
		static const Topic<> readDialogueFlag;

		//set by InitSystem (if needed); persistent
		static const Topic<config::PrngType> random;

		//set and cleared by InputParserSystem
		static const Topic<systems::GameCommands> gameCommands;

		//set and cleared by PlayerStateSystem
		static const Topic<std::tuple<EntityHandle, systems::PlayerStates>> 
			playerStateEntry;

		//set and cleared by CollisionHandlerSystem
		static const Topic<EntityHandle> playerHits;

		//set by CollisionHandlerSystem and PlayerDeathDetectorSystem,
		//cleared by MiscellaneousSystem
		static const Topic<EntityHandle> deaths;

		//set by DeathHandlerSystem, cleared by ScriptSystem (boss's program)
		static const Topic<EntityHandle> bossDeaths;
		
		//set by any system that clears the screen, cleared by ClearSystem
		static const Topic<> clearFlag;

		//set by ContinueSystem and PauseSystem, cleared by MiscellaneousSystem
		static const Topic<> pauseFlag;

		//set and cleared by ScriptSystem (stage program)
		static const Topic<> winFlag;
		
		//topics for scripts
		static const Topic<std::tuple<wasp::math::Point2, std::string>> points;
	};
}