#pragma once

#include "SceneStorage.h"

namespace wasp::scene {

	//A SceneList represents the active scenes in a game.
	//SystemCallEnumClass is the enum class used to identify different system chains
	//SceneNameEnumClass is the enum class used to identify different scenes
	template <typename SystemChainIDEnumClass, typename SceneNameEnumClass>
	class SceneList {

	private:
		//typedefs
		using Scene = Scene<SystemChainIDEnumClass, SceneNameEnumClass>;
		using ScenePointer = std::shared_ptr<Scene>;
		using SceneStorage = SceneStorage<SystemChainIDEnumClass, SceneNameEnumClass>;
	public:
		using Iterator = typename std::vector<ScenePointer>::iterator;
		using ReverseIterator = typename std::vector<ScenePointer>::reverse_iterator;

	private:
		//fields
		std::vector<ScenePointer> sceneList{};
		SceneStorage sceneStorage{};

	public:
		//takes ownership of an r-value sceneStorage
		SceneList(SceneStorage&& sceneStorage)
			: sceneStorage{ sceneStorage } {
		}

		//iteration
		Iterator begin() {
			return sceneList.begin();
		}
		Iterator end() {
			return sceneList.end();
		}
		ReverseIterator rbegin() {
			return sceneList.rbegin();
		}
		ReverseIterator rend() {
			return sceneList.rend();
		}

		//stacking scenes
		void pushScene(SceneNameEnumClass sceneName) {
			ScenePointer& scenePointer{ sceneStorage.getScenePointer(sceneName) };
			if (scenePointer->needsRefresh()) {
				scenePointer->refreshScene();
			}
			sceneList.push_back(scenePointer);
		}
		void popScene() {
			sceneList.pop_back();
		}
		void popBackTo(SceneNameEnumClass sceneName) {
			while (sceneList.back()->getName() != sceneName) {
				popScene();
			}
		}
	};
}