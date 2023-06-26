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
			&(resourceMasterStoragePointer->scriptStorage)
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
		, scriptSystem{
			globalChannelSetPointer,
			&(resourceMasterStoragePointer->scriptStorage),
			&(resourceMasterStoragePointer->spriteStorage)
		}
		, collisionHandlerSystem{ &(resourceMasterStoragePointer->scriptStorage) }
		, playerShotSystem{ &(resourceMasterStoragePointer->scriptStorage) }
		, playerBombSystem{ &(resourceMasterStoragePointer->scriptStorage) }
		, continueSystem{ globalChannelSetPointer }
		, deathHandlerSystem{ resourceMasterStoragePointer->scriptStorage }
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
		scriptSystem(scene);
		playerMovementSystem(scene);
		velocitySystem(scene);
		collisionDetectorSystem(scene);
		collisionHandlerSystem(scene);
		clearSystem(scene);
		playerShotSystem(scene);
		playerStateSystem(scene);
		playerBombSystem(scene);
		playerDeathDetectorSystem(scene);
		continueSystem(scene);
		playerRespawnSystem(scene);
		playerReactivateSystem(scene);
		deathHandlerSystem(scene);
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