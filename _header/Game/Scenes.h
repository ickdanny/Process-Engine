#pragma once

#include "Scene/SceneList.h"
#include "SystemChainIDs.h"

namespace process::game {

	enum class SceneNames {
		main,
		difficulty,
		shot,
		stage,
		music,
		options,
		load,
		game,
		dialogue,
		pause,
		continues,
		credits
	};

	using Scene = wasp::scene::Scene<SystemChainIDs, SceneNames>;
	using SceneList = wasp::scene::SceneList<SystemChainIDs, SceneNames>;

	SceneList makeSceneList();
}