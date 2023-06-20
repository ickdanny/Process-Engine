#pragma once

#include "systemInclude.h"
#include "Game/Resources/SpriteStorage.h"
#include "Game/Resources/ScriptStorage.h"
#include "Game/Systems/EntityBuilder.h"

namespace process::game::systems {

	class InitSystem {
	private:
		//typedefs
		using EntityHandle = wasp::ecs::entity::EntityHandle;
		using BasicButtonComponentTuple = ComponentTuple<
			Position,
			VisibleMarker,
			ButtonData,
			MenuCommandSelect,
			SpriteInstruction
		>;

		//fields
		wasp::channel::ChannelSet* globalChannelSetPointer{};
		resources::SpriteStorage* spriteStoragePointer{};
		resources::ScriptStorage* scriptStoragePointer{};

	public:
		InitSystem(
			wasp::channel::ChannelSet* globalChannelSetPointer,
			resources::SpriteStorage* spriteStoragePointer,
			resources::ScriptStorage* scriptStoragePointer
		)
			: globalChannelSetPointer{ globalChannelSetPointer }
			, spriteStoragePointer { spriteStoragePointer }
			, scriptStoragePointer { scriptStoragePointer } {
		}

		void operator()(Scene& scene) const;

		//helper functions
	private:
		void initScene(Scene& scene) const;
		void initMainMenu(Scene& scene) const;
		void initDifficultyMenu(Scene& scene) const;
		void initShotMenu(Scene& scene) const;
		void initStageMenu(Scene& scene) const;
		void initMusicMenu(Scene& scene) const;
		void initOptionsMenu(Scene& scene) const;
		void initContinueMenu(Scene& scene) const;
		void initLoad(Scene& scene) const;

		void initGame(Scene& scene) const;
		void initDialogue(Scene& scene) const;
		void initPauseMenu(Scene& scene) const;
		void initCredits(Scene& scene) const;

		void addBackground(
			wasp::ecs::DataStorage& dataStorage,
			const std::wstring& name,
			int relativeDepth = 0,
			const wasp::math::Point2& position =
				{ config::graphicsWidth / 2.0f, config::graphicsHeight / 2.0f }
		) const;
		void addForeground(
			wasp::ecs::DataStorage& dataStorage,
			const std::wstring& name,
			int relativeDepth = 0,
			const wasp::math::Point2& position =
				{ config::graphicsWidth / 2.0f, config::graphicsHeight / 2.0f }
		) const;
		
		[[nodiscard]]
		BasicButtonComponentTuple makeButton(
			const wasp::math::Point2& initPos,
			const wasp::math::Vector2& offset,
			const wasp::math::Vector2& selOffset,
			int index,
			const std::wstring& name,
			MenuCommandSelect selectCommand,
			int depth = 0,
			bool selected = false
		) const;
		
		[[nodiscard]]
		BasicButtonComponentTuple makeButton(
			const wasp::math::Point2& unselPos,
			const wasp::math::Vector2& selOffset,
			const std::wstring& name,
			MenuCommandSelect selectCommand,
			int depth = 0,
			bool selected = false
		) const;

		//attaches buttons from top down
		void attachButtonsVertical(
			wasp::ecs::DataStorage& dataStorage,
			const std::vector<EntityHandle>& entityHandles
		) const;

		//attaches buttons from left to right
		void attachButtonsHorizontal(
			wasp::ecs::DataStorage& dataStorage,
			const std::vector<EntityHandle>& entityHandles
		) const;

		void setInitSelectedElement(Scene& scene, const EntityHandle& entityHandle)
			const;
	};
}