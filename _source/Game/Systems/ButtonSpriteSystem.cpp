#include "Game/Systems/ButtonSpriteSystem.h"

namespace process::game::systems {

	ButtonSpriteSystem::ButtonSpriteSystem(
		resources::SpriteStorage* spriteStoragePointer
	)
		: spriteStoragePointer{ spriteStoragePointer } {
	}

	void ButtonSpriteSystem::operator()(Scene& scene) {
		//this system only runs when there are new elementSelection messages
		auto& elementSelectionChannel{ scene.getChannel(SceneTopics::elementSelection) };
		if (elementSelectionChannel.hasMessages()) {
			for (auto& [entityHandle, select] : elementSelectionChannel.getMessages()) {
				DataStorage& dataStorage{ scene.getDataStorage() };
				if (dataStorage.containsAllComponents<
					ButtonData,
					SpriteInstruction,
					Position
				>(entityHandle)) 
				{
					if (select) {
						selectButton(dataStorage, entityHandle);
					}
					else {
						unselectButton(dataStorage, entityHandle);
					}
				}
			}
		}
	}

	void ButtonSpriteSystem::selectButton(
		DataStorage& dataStorage, 
		const EntityHandle& buttonHandle
	) {
		ButtonData& buttonData{ dataStorage.getComponent<ButtonData>(buttonHandle) };
		changeSpriteAndPosition(
			dataStorage,
			buttonHandle,
			buttonData.getSelImageName(),
			buttonData.getSelPos()
		);
	}

	void ButtonSpriteSystem::unselectButton(
		DataStorage& dataStorage,
		const EntityHandle& buttonHandle
	) {
		ButtonData& buttonData{ dataStorage.getComponent<ButtonData>(buttonHandle) };
		changeSpriteAndPosition(
			dataStorage,
			buttonHandle,
			buttonData.getUnselImageName(),
			buttonData.getUnselPos()
		);
	}

	void ButtonSpriteSystem::changeSpriteAndPosition(
		DataStorage& dataStorage,
		const EntityHandle& buttonHandle,
		const std::wstring& spriteName,
		const wasp::math::Point2& position
	) {
		dataStorage.getComponent<SpriteInstruction>(buttonHandle).setSprite(
			spriteStoragePointer->get(spriteName)->sprite
		);
		dataStorage.getComponent<Position>(buttonHandle) = Position{ position };
	}
}