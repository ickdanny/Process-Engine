#pragma once

#include "systemInclude.h"

#include "Game/Resources/SpriteStorage.h"
#include "EntityBuilder.h"

namespace process::game::systems {

	class OverlaySystem {
	private:
		//typedefs
		using EntityHandle = wasp::ecs::entity::EntityHandle;
		using SceneData = std::tuple<
			std::array<EntityHandle, config::maxLives>,		//life icons
			std::array<EntityHandle, config::maxBombs>,		//bomb icons
			EntityHandle,									//power meter
			int,											//current life index
			int												//current bomb index
		>;
		using IconComponentTuple = ComponentTuple<
			Position,
			SpriteInstruction
		>;
		using Point2 = wasp::math::Point2;
		using Vector2 = wasp::math::Vector2;

		//fields
		resources::SpriteStorage* spriteStoragePointer{};

	public:
		OverlaySystem(
			resources::SpriteStorage* spriteStoragePointer
		)
			: spriteStoragePointer{ spriteStoragePointer } {
		}

		void operator()(Scene& scene);

	private:
		//helper functions
		void updateOverlay(Scene& scene, const PlayerData& playerData);
		SceneData& getSceneData(Scene& scene, const PlayerData& playerData);
		IconComponentTuple makeIcon(
			const Point2& initPos,
			const Vector2& offset,
			int index,
			const std::wstring& imageName
		) const;

		void updateLives(
			Scene& scene,
			SceneData& sceneData,
			const PlayerData& playerData
		);

		void updateBombs(
			Scene& scene,
			SceneData& sceneData,
			const PlayerData& playerData
		);

		void updatePower(
			Scene& scene,
			SceneData& sceneData,
			const PlayerData& playerData
		);
	};
}