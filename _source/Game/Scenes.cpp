#include "Game/Scenes.h"

namespace process::game {

	using SceneStorage = wasp::scene::SceneStorage<SystemChainIDs, SceneNames>;

	SceneList makeSceneList() {
		SceneStorage sceneStorage{};

		std::vector<std::pair<SystemChainIDs, bool>> noTransparency{
			{ SystemChainIDs::update, false},
			{ SystemChainIDs::render, false}
		};

		std::vector<std::pair<SystemChainIDs, bool>> renderTransparency{
			{ SystemChainIDs::update, false},
			{ SystemChainIDs::render, true}
		};

		std::vector<std::pair<SystemChainIDs, bool>> fullTransparency{
			{ SystemChainIDs::update, true},
			{ SystemChainIDs::render, true}
		};

		sceneStorage.makeScene(
			SceneNames::main,		//name
			20,						//initEntityCapacity
			20,						//initComponentCapacity
			noTransparency,			//transparency
			false					//refresh
		);

		sceneStorage.makeScene(
			SceneNames::difficulty,	//name
			20,						//initEntityCapacity
			20,						//initComponentCapacity
			noTransparency,			//transparency
			false					//refresh
		);

		sceneStorage.makeScene(
			SceneNames::shot,		//name
			50,						//initEntityCapacity
			20,						//initComponentCapacity
			noTransparency,			//transparency
			true					//refresh - true so we can figure out game or stage
		);

		sceneStorage.makeScene(
			SceneNames::stage,		//name
			20,						//initEntityCapacity
			20,						//initComponentCapacity
			noTransparency,			//transparency
			false					//refresh
		);

		sceneStorage.makeScene(
			SceneNames::music,		//name
			20,						//initEntityCapacity
			20,						//initComponentCapacity
			renderTransparency,		//transparency
			true					//refresh
		);

		sceneStorage.makeScene(
			SceneNames::options,	//name
			20,						//initEntityCapacity
			20,						//initComponentCapacity
			renderTransparency,		//transparency
			true					//refresh
		);

		sceneStorage.makeScene(
			SceneNames::load,		//name
			3,						//initEntityCapacity
			20,						//initComponentCapacity
			renderTransparency,		//transparency
			false					//refresh
		);

		sceneStorage.makeScene(
			SceneNames::game,		//name
			1000,					//initEntityCapacity
			50,						//initComponentCapacity
			noTransparency,			//transparency
			true					//refresh
		);

		sceneStorage.makeScene(
			SceneNames::dialogue,	//name
			5,						//initEntityCapacity
			50,						//initComponentCapacity
			fullTransparency,		//transparency
			true					//refresh
		);

		sceneStorage.makeScene(
			SceneNames::pause,		//name
			20,						//initEntityCapacity
			20,						//initComponentCapacity
			renderTransparency,		//transparency
			true					//refresh
		);

		sceneStorage.makeScene(
			SceneNames::continues,	//name
			20,						//initEntityCapacity
			20,						//initComponentCapacity
			renderTransparency,		//transparency
			true					//refresh
		);

		sceneStorage.makeScene(
			SceneNames::credits,	//name
			20,						//initEntityCapacity
			20,						//initComponentCapacity
			noTransparency,			//transparency
			true					//refresh
		);

		return { std::move(sceneStorage) };
	}
}