#pragma once

#include "systemInclude.h"

#include "Game/Resources/BitmapStorage.h"
#include "Game/Resources/DialogueStorage.h"
#include "EntityBuilder.h"

namespace process::game::systems {

	class DialogueSystem {
	private:
		//typedefs
		using EntityHandle = ecs::entity::EntityHandle;
		using DialogueCommand = resources::DialogueCommand;
		using Dialogue = resources::Dialogue;

			//dialogue, dialoguePos, spriteHandles
			//spriteHandles are 0 - left image, 1 - right image, 2 - text
		using SceneData = std::tuple<
			Dialogue, 
			std::size_t, 
			std::array<EntityHandle, 3>
		>;

		//fields
		channel::ChannelSet* globalChannelSetPointer{};
		resources::BitmapStorage* bitmapStoragePointer{};
		resources::DialogueStorage* dialogueStoragePointer{};

	public:
		DialogueSystem(
			channel::ChannelSet* globalChannelSetPointer,
			resources::BitmapStorage* bitmapStoragePointer,
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