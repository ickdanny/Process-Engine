#include "Game/Systems/DialogueSystem.h"

namespace process::game::systems {

    namespace {
        constexpr float portraitY{ 170.0f };
        constexpr float leftX{ 45.0f };
        constexpr float rightX{ 270.0f };

        constexpr wasp::math::Point2 textPos{ 85.0f, 170.0f };

        constexpr int textRightBound{ 250 };
    }

	DialogueSystem::DialogueSystem(
        wasp::channel::ChannelSet* globalChannelSetPointer,
		resources::SpriteStorage* spriteStoragePointer,
		resources::DialogueStorage* dialogueStoragePointer
	) 
        : globalChannelSetPointer{ globalChannelSetPointer }
		, spriteStoragePointer{ spriteStoragePointer }
		, dialogueStoragePointer{ dialogueStoragePointer }
	{
	}

	void DialogueSystem::operator()(Scene& scene) {
        if (scene.getName() == SceneNames::dialogue) {
            SceneData& sceneData{ getSceneData(scene) };

            if (scene.getChannel(SceneTopics::readDialogueFlag).hasMessages()) {
                advanceDialogue(scene, sceneData);
            }
        }
	}

	DialogueSystem::SceneData& DialogueSystem::getSceneData(Scene& scene) {
        static const Topic<SceneData> sceneDataTopic{};
        auto& sceneDataChannel{ scene.getChannel(sceneDataTopic) };

        if (!sceneDataChannel.hasMessages()) {

            //grab the dialogue
            auto& startDialogueChannel{
                globalChannelSetPointer->getChannel(GlobalTopics::startDialogue)
            };
            if (!startDialogueChannel.hasMessages()) {
                throw std::runtime_error{ "start dialogue channel no messages!" };

            }
            std::wstring& dialogueID{ startDialogueChannel.getMessages()[0] };
            Dialogue& dialogue{ *dialogueStoragePointer->get(dialogueID) };
            startDialogueChannel.clear();

            //create entities
            auto& dataStorage{ scene.getDataStorage() };
            std::array<EntityHandle, 3> spriteHandles{};

            //left image
            spriteHandles[0] = dataStorage.addEntity(
                EntityBuilder::makeVisible(
                    { leftX, portraitY }
                ).package()
            );
            //right image
            spriteHandles[1] = dataStorage.addEntity(
                EntityBuilder::makeVisible(
                    { rightX, portraitY }
                ).package()
            );
            //text
            spriteHandles[2] = dataStorage.addEntity(
                EntityBuilder::makeVisible(textPos).package()
            );

            SceneData sceneData{
                dialogue,
                0,
                std::move(spriteHandles)
            };
            
            //advance dialogue once upon init
            advanceDialogue(scene, sceneData);

            sceneDataChannel.addMessage(sceneData);
        }
        return sceneDataChannel.getMessages()[0];
	}

    void DialogueSystem::advanceDialogue(Scene& scene, SceneData& sceneData) {
        auto& [dialogue, dialoguePos, spriteHandles] = sceneData;
        while (dialoguePos < dialogue.size()) {
            if (executeCommand(scene, spriteHandles, dialogue[dialoguePos])) {
                ++dialoguePos;
            }
            else {
                ++dialoguePos;
                break;
            }
        }
    }

    //true if continue, false if block
    bool DialogueSystem::executeCommand(
        Scene& scene,
        std::array<EntityHandle, 3>& spriteHandles,
        DialogueCommand& dialogueCommand
    ) {
        switch (dialogueCommand.command) {
            case DialogueCommand::Commands::setLeftImage:
                setImage(scene, spriteHandles[0], dialogueCommand.data);
                return true;
            case DialogueCommand::Commands::setRightImage:
                setImage(scene, spriteHandles[1], dialogueCommand.data);
                return true;
            case DialogueCommand::Commands::setText:
                setText(scene, spriteHandles[2], dialogueCommand.data);
                return true;
            case DialogueCommand::Commands::setTrack:
                setTrack(dialogueCommand.data);
                return true;
            case DialogueCommand::Commands::stop:
                return false;
            case DialogueCommand::Commands::end:
                exit();
                return false;
            default:
                throw std::runtime_error{ "reached default case!" };
        }
    }

    void DialogueSystem::setImage(
        Scene& scene,
        EntityHandle& entityHandle,
        const std::wstring& id
    ) {
        auto& dataStorage{ scene.getDataStorage() };
        dataStorage.setComponent<SpriteInstruction>({
            entityHandle,
            SpriteInstruction{ 
                spriteStoragePointer->get(id)->sprite,
				config::foregroundDepth
            }
        });
    }

    void DialogueSystem::setText(
        Scene& scene,
        EntityHandle& entityHandle,
        const std::wstring& text
    ) {
        auto& dataStorage{ scene.getDataStorage() };
        dataStorage.setComponent<TextInstruction>({
            entityHandle,
            { text, textRightBound }
        });
    }

    void DialogueSystem::setTrack(const std::wstring& id) {
        globalChannelSetPointer->getChannel(GlobalTopics::startMusic)
            .addMessage(id);
    }

    void DialogueSystem::exit() {
        globalChannelSetPointer->getChannel(GlobalTopics::sceneExitTo)
            .addMessage(SceneNames::game);
        globalChannelSetPointer->getChannel(GlobalTopics::endDialogueFlag).addMessage();
    }
}