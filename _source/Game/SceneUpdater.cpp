#include "Game/SceneUpdater.h"

#include "Logging.h"

namespace process::game {
	SceneUpdater::SceneUpdater(
		resources::ResourceMasterStorage* resourceMasterStoragePointer,
		wasp::input::IKeyInputTable* keyInputTablePointer,
		wasp::channel::ChannelSet* globalChannelSetPointer
	)
		//: programs{ &(resourceMasterStoragePointer->bitmapStorage) }

		: initSystem{
			globalChannelSetPointer, 
			&(resourceMasterStoragePointer->spriteStorage),
			&programs
		}
		, inputParserSystem{ keyInputTablePointer }
		, menuNavigationSystem{ globalChannelSetPointer }
		, buttonSpriteSystem{ &(resourceMasterStoragePointer->spriteStorage) }
		, gameBuilderSystem { globalChannelSetPointer }
		, loadSystem{ globalChannelSetPointer }
		, dialogueSystem{ 
			globalChannelSetPointer,
			&(resourceMasterStoragePointer->spriteStorage),
			&(resourceMasterStoragePointer->dialogueStorage)
		}
		, scriptSystem{ globalChannelSetPointer }
		, playerShotSystem{ &programs }
		, collisionHandlerSystem{ &programs }
		, playerBombSystem{ &programs }
		, continueSystem{ globalChannelSetPointer }
		, spawnSystem{ globalChannelSetPointer }
		, overlaySystem{ &(resourceMasterStoragePointer->spriteStorage) }
		, pauseSystem{ globalChannelSetPointer }
		, animationSystem{ &(resourceMasterStoragePointer->spriteStorage) }
		, gameOverSystem{ globalChannelSetPointer }
		, creditsSystem{ globalChannelSetPointer } {
	}

	void SceneUpdater::operator()(Scene& scene) {
		initSystem(scene);
		miscellaneousSystem(scene);
		inputParserSystem(scene);
		menuNavigationSystem(scene);
		buttonSpriteSystem(scene);
		gameBuilderSystem(scene);
		loadSystem(scene);
		dialogueSystem(scene);
		velocitySystem(scene);
		scriptSystem(scene);
		collisionDetectorSystem(scene);
		playerMovementSystem(scene);
		playerShotSystem(scene);
		collisionHandlerSystem(scene);
		playerStateSystem(scene);
		playerBombSystem(scene);
		playerDeathDetectorSystem(scene);
		continueSystem(scene);
		playerRespawnSystem(scene);
		playerReactivateSystem(scene);
		deathHandlerSystem(scene);
		spawnSystem(scene);
		overlaySystem(scene);
		pauseSystem(scene);
		animationSystem(scene);
		rotateSpriteForwardSystem(scene);
		spriteSpinSystem(scene);
		subImageScrollSystem(scene);
        inboundSystem(scene);
        outboundSystem(scene);
		gameOverSystem(scene);
		creditsSystem(scene);
	}
}