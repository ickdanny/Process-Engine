#pragma once

#include "systemInclude.h"

#include "Game/Resources/SpriteStorage.h"
#include "Game/Resources/DialogueStorage.h"
#include "EntityBuilder.h"

namespace process::game::systems {

	class DialogueSystem {
	private:
		//typedefs
		using EntityHandle = wasp::ecs::entity::EntityHandle;
		using DialogueCommand = wasp::game::resources::DialogueCommand;
		using Dialogue = wasp::game::resources::Dialogue;

			//dialogue, dialoguePos, spriteHandles
			//spriteHandles are 0 - left image, 1 - right image, 2 - text
		using SceneData = std::tuple<
			Dialogue, 
			std::size_t, 
			std::array<EntityHandle, 3>
		>;

		//fields
		wasp::channel::ChannelSet* globalChannelSetPointer{};
		resources::SpriteStorage* spriteStoragePointer{};
		resources::DialogueStorage* dialogueStoragePointer{};

	public:
		DialogueSystem(
			wasp::channel::ChannelSet* globalChannelSetPointer,
			resources::SpriteStorage* spriteStoragePointer,
			resources::DialogueStorage* dialogueStoragePointer
		);
		void operator()(Scene& scene);

	private:
		SceneData& getSceneData(Scene& scene);
		void advanceDialogue(Scene& scene, SceneData& sceneData);

		//true if continue, false if block
		bool executeCommand(
			Scene& scene,
			std::array<EntityHandle, 3>& spriteHandles,
			DialogueCommand& dialogueCommand
		);

		void setImage(Scene& scene, EntityHandle& entityHandle, const std::wstring& id);
		void setText(
			Scene& scene, 
			EntityHandle& entityHandle, 
			const std::wstring& text
		);
		void setTrack(const std::wstring& id);
		void exit();
	};
}