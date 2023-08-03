#include "Prototypes.h"

#include "StringUtil.h"

namespace process::game::systems{
	
	using AABB = wasp::math::AABB;
	
	namespace playerA{
		constexpr float boomerangHitbox{ 9.0f };
		constexpr int boomerangDamage{ 8 };
		constexpr float boomerangOutbound{ -50.0f };
		constexpr float boomerangSpin{ 21.728172f };
		
		constexpr float scytheHitbox{ 35.0f };
		constexpr int scytheDamage{ 2 };//damage per tick
		constexpr int scytheExplodeDamage{ 500 };
		constexpr float scytheSpin{ 19.4892f };
	}
	
	namespace playerB{
		constexpr float laserHitbox{ 10.0f };
		constexpr int laserDamage{ 7 };
		constexpr float laserOutbound{ -70.0f };
		
		constexpr float laserPartHitbox{ 15.0f };
		constexpr int laserPartDamage{ 1 };//damage per tick
		constexpr float haloHitbox{ 20.0f };
		constexpr int haloDamage{ 1 };//damage per tick
	}
	
	namespace pickup{
		constexpr float smallHitbox{ 8.0f };
		constexpr float largeHitbox{ 12.0f };
		constexpr float outbound{ -100.0f };
	}
	
	namespace enemy{
		constexpr int spawnHealth{ 32000 };//set health in each script
		constexpr float outbound{ -30.0f };
		constexpr float machineHitbox{ 10.0f };
		constexpr float machine2Hitbox{ 9.0f };
		constexpr float batHitbox{ 8.0f };
		constexpr float wingHitbox{ 11.0f };
		constexpr float cloudHitbox{ 9.5f };
		constexpr float crystalHitbox{ 6.0f };
		constexpr float automatonHitbox{ 5.0f };
		constexpr float flameHitbox{ 7.0 };
		constexpr AABB bossHitbox{ 7.0f, 13.0f };
		
		constexpr float trapSpin{ -2.342f };
		constexpr int bossAnimationTick{ 6 };
	}
	
	namespace enemyProjectile{
		constexpr float smallHitbox{ 2.5f };
		constexpr float mediumHitbox{ 3.5f };
		constexpr float largeHitbox{ 7.5f };
		constexpr float sharpHitbox{ 1.5f };
		constexpr float outbound{ -21.0f };
	}
	
	Prototypes::Prototypes(
		resources::ScriptStorage& scriptStorage,
		resources::SpriteStorage& spriteStorage
	){
		add("projectileExplode", EntityBuilder::makeVisiblePrototype(
			SpriteInstruction{
				spriteStorage.get(L"projectileExplode1")->sprite,
				config::effectDepth
			},
			game::AnimationList{
				components::Animation {
					{
						L"projectileExplode1",
						L"projectileExplode2",
						L"projectileExplode3"
					},
					false
				},
				4
			},
			ScriptList{ {
				scriptStorage.get(L"removeProjectileExplode"),
				"removeProjectileExplode"
			} }
		).heapClone());
		
		addPlayerPrototypes(scriptStorage, spriteStorage);
		addPickupPrototypes(scriptStorage, spriteStorage);
		addEnemyPrototypes(scriptStorage, spriteStorage);
	}
	
	void Prototypes::addPlayerPrototypes(
		resources::ScriptStorage& scriptStorage,
		resources::SpriteStorage& spriteStorage
	){
		//PLAYER A
		add("boomerang", EntityBuilder::makeVisibleCollidablePrototype(
			AABB{ playerA::boomerangHitbox },
			EnemyCollisions::Source{ components::CollisionCommands::death },
			Damage{ playerA::boomerangDamage },
			Outbound{ playerA::boomerangOutbound },
			SpriteInstruction{
				spriteStorage.get(L"boomerang")->sprite,
				config::playerBulletDepth
			},
			SpriteSpin{ playerA::boomerangSpin },
			game::DeathCommand{ game::DeathCommand::Commands::deathSpawn },
			DeathSpawn{ ScriptList{ {
				scriptStorage.get(L"projectileExplode"),
				std::string{ ScriptList::spawnString } + "projectileExplode"
			} } }
		).heapClone());
		add("scythe", EntityBuilder::makeVisibleCollidablePrototype(
			AABB{ playerA::scytheHitbox },
			EnemyCollisions::Source{},
			BulletCollisions::Source{},
			Damage{ playerA::scytheDamage },
			SpriteInstruction{
				spriteStorage.get(L"scythe")->sprite,
				config::playerBulletDepth - 10
			},
			SpriteSpin{ playerA::scytheSpin },
			game::DeathCommand{ game::DeathCommand::Commands::deathSpawn },
			DeathSpawn{ ScriptList{ {
				scriptStorage.get(L"scytheExplode"),
				std::string{ ScriptList::spawnString } + "scytheExplode"
			} } }
		).heapClone());
		add("scytheExplode", EntityBuilder::makeVisibleCollidablePrototype(
			AABB{ playerA::scytheHitbox },
			EnemyCollisions::Source{},
			BulletCollisions::Source{},
			Damage{ playerA::scytheExplodeDamage },
			SpriteInstruction{
				spriteStorage.get(L"scytheExplode1")->sprite,
				config::playerDepth + 1
			},
			game::AnimationList{
				components::Animation {
					{
						L"scytheExplode1",
						L"scytheExplode2",
						L"scytheExplode3",
						L"scytheExplode4"
					},
					false
				},
				3
			},
			ScriptList{ {
				scriptStorage.get(L"removeScytheExplode"),
				"removeScytheExplode"
			} }
		).heapClone());
		
		//PLAYER B
		add("smallLaser", EntityBuilder::makeVisibleCollidablePrototype(
			AABB{ playerB::laserHitbox },
			EnemyCollisions::Source{ components::CollisionCommands::death },
			Damage{ playerB::laserDamage },
			Outbound{ playerB::laserOutbound },
			SpriteInstruction{
				spriteStorage.get(L"smallLaser")->sprite,
				config::playerBulletDepth
			},
			RotateSpriteForwardMarker{},
			game::DeathCommand{ game::DeathCommand::Commands::deathSpawn },
			DeathSpawn{ ScriptList{ {
				scriptStorage.get(L"projectileExplode"),
				std::string{ ScriptList::spawnString } + "projectileExplode"
			} } }
		).heapClone());
		add("laserBombPart", EntityBuilder::makeVisiblePrototype(
			Hitbox{ playerB::laserPartHitbox },
			EnemyCollisions::Source{},
			BulletCollisions::Source{},
			Damage{ playerB::laserPartDamage },
			SpriteInstruction{
				spriteStorage.get(L"laserBombPart1")->sprite,
				config::playerBulletDepth + 10
			},
			game::AnimationList{
				components::Animation {
					{
						L"laserBombPart1",
						L"laserBombPart2",
						L"laserBombPart3",
						L"laserBombPart4"
					},
					false
				},
				5
			},
			game::DeathCommand{ game::DeathCommand::Commands::deathSpawn },
			DeathSpawn{ ScriptList{ {
				scriptStorage.get(L"laserBombPartExplode"),
				std::string{ ScriptList::spawnString } + "laserBombPartExplode"
			} } }
		).heapClone());
		add("laserBombPartExplode", EntityBuilder::makeVisiblePrototype(
			SpriteInstruction{
				spriteStorage.get(L"laserBombPartExplode1")->sprite,
				config::playerBulletDepth + 10
			},
			game::AnimationList{
				components::Animation {
					{
						L"laserBombPartExplode1",
						L"laserBombPartExplode2",
						L"laserBombPartExplode3"
					},
					false
				},
				4
			},
			ScriptList{ {
				scriptStorage.get(L"removeLaserBombPartExplode"),
				"removeLaserBombPartExplode"
			} }
		).heapClone());
		add("halo", EntityBuilder::makeVisiblePrototype(
			Hitbox{ playerB::haloHitbox },
			EnemyCollisions::Source{},
			BulletCollisions::Source{},
			Damage{ playerB::haloDamage },
			SpriteInstruction{
				spriteStorage.get(L"halo1")->sprite,
				config::playerBulletDepth + 11
			},
			SpriteSpin{ 240.0f / 205.0f },
			game::AnimationList{
				components::Animation {
					{
						L"halo1",
						L"halo2",
						L"halo3",
					},
					false
				},
				5
			},
			game::DeathCommand{ game::DeathCommand::Commands::deathSpawn },
			DeathSpawn{ ScriptList{ {
				scriptStorage.get(L"haloExplode"),
				std::string{ ScriptList::spawnString } + "haloExplode"
			} } }
		).heapClone());
		add("haloExplode", EntityBuilder::makeVisiblePrototype(
			SpriteInstruction{
				spriteStorage.get(L"haloExplode1")->sprite,
				config::playerBulletDepth + 11
			},
			game::AnimationList{
				components::Animation {
					{
						L"haloExplode1",
						L"haloExplode2",
						L"haloExplode3"
					},
					false
				},
				4
			},
			ScriptList{ {
				scriptStorage.get(L"removeHaloExplode"),
				"removeHaloExplode"
			} }
		).heapClone());
	}
	
	void Prototypes::addPickupPrototypes(
		resources::ScriptStorage& scriptStorage,
		resources::SpriteStorage& spriteStorage
	){
		add("powerSmall", EntityBuilder::makeVisibleCollidablePrototype(
			AABB{ pickup::smallHitbox },
			PickupCollisions::Source{ components::CollisionCommands::pickup },
			PickupType{ wasp::game::components::PickupType::Types::powerSmall },
			Outbound{ pickup::outbound },
			SpriteInstruction{
				spriteStorage.get(L"pickupPowerSmall")->sprite,
				config::pickupDepth
			}
		).heapClone());
		add("powerLarge", EntityBuilder::makeVisibleCollidablePrototype(
			AABB{ pickup::largeHitbox },
			PickupCollisions::Source{ components::CollisionCommands::pickup },
			PickupType{ wasp::game::components::PickupType::Types::powerLarge },
			Outbound{ pickup::outbound },
			SpriteInstruction{
				spriteStorage.get(L"pickupPowerLarge")->sprite,
				config::pickupDepth + 1
			}
		).heapClone());
		add("life", EntityBuilder::makeVisibleCollidablePrototype(
			AABB{ pickup::largeHitbox },
			PickupCollisions::Source{ components::CollisionCommands::pickup },
			PickupType{ wasp::game::components::PickupType::Types::life },
			Outbound{ pickup::outbound },
			SpriteInstruction{
				spriteStorage.get(L"pickupLife")->sprite,
				config::pickupDepth + 2
			}
		).heapClone());
		add("bomb", EntityBuilder::makeVisibleCollidablePrototype(
			AABB{ pickup::largeHitbox },
			PickupCollisions::Source{ components::CollisionCommands::pickup },
			PickupType{ wasp::game::components::PickupType::Types::bomb },
			Outbound{ pickup::outbound },
			SpriteInstruction{
				spriteStorage.get(L"pickupBomb")->sprite,
				config::pickupDepth + 2
			}
		).heapClone());
	}
	
	void Prototypes::addEnemyPrototypes(
		resources::ScriptStorage& scriptStorage,
		resources::SpriteStorage& spriteStorage
	){
		//PROJECTILES
		#define addLarge(color) \
			addEnemyProjectile( \
				scriptStorage, \
				spriteStorage, \
				"large", \
				"large", \
				enemyProjectile::largeHitbox, \
				color, \
				0 \
            )
		addLarge("Black");
		addLarge("DBlue");
		addLarge("Blue");
		addLarge("LBlue");
		addLarge("Clear");
		addLarge("DGray");
		addLarge("LGray");
		addLarge("DGreen");
		addLarge("LGreen");
		addLarge("Orange");
		addLarge("DPurple");
		addLarge("LPurple");
		addLarge("DRed");
		addLarge("LRed");
		addLarge("Yellow");
		#undef addLarge
		
		#define addMedium(color) \
			addEnemyProjectile( \
				scriptStorage, \
				spriteStorage, \
				"medium", \
				"medium", \
				enemyProjectile::mediumHitbox, \
				color, \
				10 \
            )
		addMedium("Black");
		addMedium("DBlue");
		addMedium("Blue");
		addMedium("LBlue");
		addMedium("Clear");
		addMedium("DGray");
		addMedium("LGray");
		addMedium("DGreen");
		addMedium("LGreen");
		addMedium("Orange");
		addMedium("DPurple");
		addMedium("LPurple");
		addMedium("DRed");
		addMedium("LRed");
		addMedium("Yellow");
		#undef addMedium
		
		#define addSmall(color) \
			addEnemyProjectile( \
				scriptStorage, \
				spriteStorage, \
				"small", \
				"small", \
				enemyProjectile::smallHitbox, \
				color, \
                20 \
            )
		addSmall("Black");
		addSmall("DBlue");
		addSmall("Blue");
		addSmall("LBlue");
		addSmall("Clear");
		addSmall("DGray");
		addSmall("LGray");
		addSmall("DGreen");
		addSmall("LGreen");
		addSmall("Orange");
		addSmall("DPurple");
		addSmall("LPurple");
		addSmall("DRed");
		addSmall("LRed");
		addSmall("Yellow");
		#undef addSmall
		
		#define addSharp(color) \
			addEnemyForwardProjectile( \
				scriptStorage, \
				spriteStorage, \
				"sharp", \
				"sharp", \
				enemyProjectile::sharpHitbox, \
				color, \
                30 \
            )
		addSharp("Black");
		addSharp("DBlue");
		addSharp("Blue");
		addSharp("LBlue");
		addSharp("Clear");
		addSharp("DGray");
		addSharp("LGray");
		addSharp("DGreen");
		addSharp("LGreen");
		addSharp("Orange");
		addSharp("DPurple");
		addSharp("LPurple");
		addSharp("DRed");
		addSharp("LRed");
		addSharp("Yellow");
		#undef addSharp
		
		#define addSharpUnderSmall(color) \
			addEnemyForwardProjectile( \
				scriptStorage, \
				spriteStorage, \
				"sharpUnderSmall", \
				"sharp", \
				enemyProjectile::sharpHitbox, \
				color, \
                19 \
            )
		addSharpUnderSmall("Black");
		addSharpUnderSmall("DBlue");
		addSharpUnderSmall("Blue");
		addSharpUnderSmall("LBlue");
		addSharpUnderSmall("Clear");
		addSharpUnderSmall("DGray");
		addSharpUnderSmall("LGray");
		addSharpUnderSmall("DGreen");
		addSharpUnderSmall("LGreen");
		addSharpUnderSmall("Orange");
		addSharpUnderSmall("DPurple");
		addSharpUnderSmall("LPurple");
		addSharpUnderSmall("DRed");
		addSharpUnderSmall("LRed");
		addSharpUnderSmall("Yellow");
		#undef addSharpUnderSmall
		
		//MOBS
		add("enemyExplode", EntityBuilder::makeVisiblePrototype(
			SpriteInstruction{
				spriteStorage.get(L"enemyExplode1")->sprite,
				config::effectDepth
			},
			game::AnimationList{
				components::Animation {
					{
						L"enemyExplode1",
						L"enemyExplode2",
						L"enemyExplode3"
					},
					false
				},
				4
			},
			ScriptList{ {
				scriptStorage.get(L"removeEnemyExplode"),
				"removeEnemyExplode"
			} }
		).heapClone());
		add("bossExplode", EntityBuilder::makeVisiblePrototype(
			SpriteInstruction{
				spriteStorage.get(L"bossExplode1")->sprite,
				config::effectDepth + 1
			},
			game::AnimationList{
				components::Animation {
					{
						L"bossExplode1",
						L"bossExplode2",
						L"bossExplode3"
					},
					false
				},
				4
			},
			ScriptList{ {
				scriptStorage.get(L"removeBossExplode"),
				"removeBossExplode"
			} }
		).heapClone());
		add("machineOrange", EntityBuilder::makeVisibleCollidablePrototype(
			AABB{ enemy::machineHitbox },
			PlayerCollisions::Source{},
			EnemyCollisions::Target{ components::CollisionCommands::damage },
			SpriteInstruction{
				spriteStorage.get(L"machineOrange")->sprite,
				config::enemyDepth
			},
			RotateSpriteForwardMarker{},
			Health{ enemy::spawnHealth },
			Outbound{ enemy::outbound },
			DeathCommand{ DeathCommand::Commands::deathSpawn },
			DeathSpawn{ ScriptList{ {
				scriptStorage.get(L"enemyExplode"),
				std::string{ ScriptList::spawnString } + "enemyExplode"
			} } }
		).heapClone());
		add("machine2Orange", EntityBuilder::makeVisibleCollidablePrototype(
			AABB{ enemy::machine2Hitbox },
			PlayerCollisions::Source{},
			EnemyCollisions::Target{ components::CollisionCommands::damage },
			SpriteInstruction{
				spriteStorage.get(L"machine2Orange1")->sprite,
				config::enemyDepth + 1
			},
			game::AnimationList{
				components::Animation {
					{
						L"machine2Orange1",
						L"machine2Orange2",
						L"machine2Orange3"
					},
					true
				},
				3
			},
			Health{ enemy::spawnHealth },
			Outbound{ enemy::outbound },
			DeathCommand{ DeathCommand::Commands::deathSpawn },
			DeathSpawn{ ScriptList{ {
				scriptStorage.get(L"enemyExplode"),
				std::string{ ScriptList::spawnString } + "enemyExplode"
			} } }
		).heapClone());
		add("machineRed", EntityBuilder::makeVisibleCollidablePrototype(
			AABB{ enemy::machineHitbox },
			PlayerCollisions::Source{},
			EnemyCollisions::Target{ components::CollisionCommands::damage },
			SpriteInstruction{
				spriteStorage.get(L"machineRed")->sprite,
				config::enemyDepth
			},
			RotateSpriteForwardMarker{},
			Health{ enemy::spawnHealth },
			Outbound{ enemy::outbound },
			DeathCommand{ DeathCommand::Commands::deathSpawn },
			DeathSpawn{ ScriptList{ {
				scriptStorage.get(L"enemyExplode"),
				std::string{ ScriptList::spawnString } + "enemyExplode"
			} } }
		).heapClone());
		add("machine2Red", EntityBuilder::makeVisibleCollidablePrototype(
			AABB{ enemy::machine2Hitbox },
			PlayerCollisions::Source{},
			EnemyCollisions::Target{ components::CollisionCommands::damage },
			SpriteInstruction{
				spriteStorage.get(L"machine2Red1")->sprite,
				config::enemyDepth + 1
			},
			game::AnimationList{
				components::Animation {
					{
						L"machine2Red1",
						L"machine2Red2",
						L"machine2Red3"
					},
					true
				},
				3
			},
			Health{ enemy::spawnHealth },
			Outbound{ enemy::outbound },
			DeathCommand{ DeathCommand::Commands::deathSpawn },
			DeathSpawn{ ScriptList{ {
				scriptStorage.get(L"enemyExplode"),
				std::string{ ScriptList::spawnString } + "enemyExplode"
			} } }
		).heapClone());
		add("bat", EntityBuilder::makeVisibleCollidablePrototype(
			AABB{ enemy::batHitbox },
			PlayerCollisions::Source{},
			EnemyCollisions::Target{ components::CollisionCommands::damage },
			SpriteInstruction{
				spriteStorage.get(L"bat1")->sprite,
				config::enemyDepth
			},
			RotateSpriteForwardMarker{},
			game::AnimationList{
				components::Animation {
					{
						L"bat1",
						L"bat2"
					},
					true
				},
				8
			},
			Health{ enemy::spawnHealth },
			Outbound{ enemy::outbound },
			DeathCommand{ DeathCommand::Commands::deathSpawn },
			DeathSpawn{ ScriptList{ {
				scriptStorage.get(L"enemyExplode"),
				std::string{ ScriptList::spawnString } + "enemyExplode"
			} } }
		).heapClone());
		add("wing", EntityBuilder::makeVisibleCollidablePrototype(
			AABB{ enemy::wingHitbox },
			PlayerCollisions::Source{},
			EnemyCollisions::Target{ components::CollisionCommands::damage },
			SpriteInstruction{
				spriteStorage.get(L"wing1")->sprite,
				config::enemyDepth
			},
			game::AnimationList{
				components::Animation {
					{
						L"wing1",
						L"wing2",
						L"wing3",
						L"wing4",
						L"wing5",
						L"wing6"
					},
					true
				},
				5
			},
			Health{ enemy::spawnHealth },
			Outbound{ enemy::outbound },
			DeathCommand{ DeathCommand::Commands::deathSpawn },
			DeathSpawn{ ScriptList{ {
				scriptStorage.get(L"bossExplode"),	//enemy is big so use boss explode
				std::string{ ScriptList::spawnString } + "bossExplode"
			} } }
		).heapClone());
		add("cloud", EntityBuilder::makeVisibleCollidablePrototype(
			AABB{ enemy::cloudHitbox },
			PlayerCollisions::Source{},
			EnemyCollisions::Target{ components::CollisionCommands::damage },
			SpriteInstruction{
				spriteStorage.get(L"cloud1")->sprite,
				config::enemyDepth
			},
			game::AnimationList{
				components::Animation {
					{
						L"cloud1",
						L"cloud2",
						L"cloud3",
						L"cloud4"
					},
					true
				},
				6
			},
			Health{ enemy::spawnHealth },
			Outbound{ enemy::outbound },
			DeathCommand{ DeathCommand::Commands::deathSpawn },
			DeathSpawn{ ScriptList{ {
				scriptStorage.get(L"enemyExplode"),
				std::string{ ScriptList::spawnString } + "enemyExplode"
			} } }
		).heapClone());
		add("crystal", EntityBuilder::makeVisibleCollidablePrototype(
			AABB{ enemy::crystalHitbox },
			PlayerCollisions::Source{},
			EnemyCollisions::Target{ components::CollisionCommands::damage },
			SpriteInstruction{
				spriteStorage.get(L"crystal1")->sprite,
				config::enemyDepth
			},
			game::AnimationList{
				components::Animation {
					{
						L"crystal1",
						L"crystal2",
						L"crystal3"
					},
					true
				},
				5
			},
			Health{ enemy::spawnHealth },
			Outbound{ enemy::outbound },
			DeathCommand{ DeathCommand::Commands::deathSpawn },
			DeathSpawn{ ScriptList{ {
				scriptStorage.get(L"enemyExplode"),
				std::string{ ScriptList::spawnString } + "enemyExplode"
			} } }
		).heapClone());
		add("crystalEmerge", EntityBuilder::makeVisiblePrototype(
			SpriteInstruction{
				spriteStorage.get(L"crystalEmerge1")->sprite,
				config::enemyDepth
			},
			game::AnimationList{
				components::Animation {
					{
						L"crystalEmerge1",
						L"crystalEmerge2",
						L"crystalEmerge3",
						L"crystalEmerge4",
						L"crystalEmerge5",
						L"crystalEmerge6",
						L"crystalEmerge7"
					},
					false
				},
				5
			},
			DeathCommand{ DeathCommand::Commands::deathSpawn },
			DeathSpawn{ ScriptList{} }
		).heapClone());
		add("trap", EntityBuilder::makeVisiblePrototype(
			SpriteInstruction{
				spriteStorage.get(L"trap1")->sprite,
				config::enemyBulletDepth - 1
			},
			game::AnimationList{
				components::Animation {
					{
						L"trap1",
						L"trap2",
						L"trap3",
						L"trap4",
						L"trap5",
						L"trap6",
						L"trap7",
						L"trap8",
						L"trap9",
						L"trap10",
						L"trap11",
						L"trap12"
					},
					false
				},
				3
			},
			SpriteSpin{ enemy::trapSpin },
			DeathCommand{ DeathCommand::Commands::deathSpawn },
			DeathSpawn{ ScriptList{} }
		).heapClone());
		add("automatonBlue", EntityBuilder::makeVisibleCollidablePrototype(
			AABB{ enemy::automatonHitbox },
			PlayerCollisions::Source{},
			EnemyCollisions::Target{ components::CollisionCommands::damage },
			SpriteInstruction{
				spriteStorage.get(L"automatonBlue1")->sprite,
				config::enemyDepth
			},
			game::AnimationList{
				components::Animation {
					{
						L"automatonBlue1",
						L"automatonBlue2",
						L"automatonBlue3",
						L"automatonBlue4"
					},
					true
				},
				enemy::bossAnimationTick
			},
			Health{ enemy::spawnHealth },
			Outbound{ enemy::outbound },
			DeathCommand{ DeathCommand::Commands::deathSpawn },
			DeathSpawn{ ScriptList{ {
				scriptStorage.get(L"enemyExplode"),
				std::string{ ScriptList::spawnString } + "enemyExplode"
			} } }
		).heapClone());
		add("automatonRed", EntityBuilder::makeVisibleCollidablePrototype(
			AABB{ enemy::automatonHitbox },
			PlayerCollisions::Source{},
			EnemyCollisions::Target{ components::CollisionCommands::damage },
			SpriteInstruction{
				spriteStorage.get(L"automatonRed1")->sprite,
				config::enemyDepth
			},
			game::AnimationList{
				components::Animation {
					{
						L"automatonRed1",
						L"automatonRed2",
						L"automatonRed3",
						L"automatonRed4"
					},
					true
				},
				enemy::bossAnimationTick
			},
			Health{ enemy::spawnHealth },
			Outbound{ enemy::outbound },
			DeathCommand{ DeathCommand::Commands::deathSpawn },
			DeathSpawn{ ScriptList{ {
				scriptStorage.get(L"enemyExplode"),
				std::string{ ScriptList::spawnString } + "enemyExplode"
			} } }
		).heapClone());
		add("shadowMaid", EntityBuilder::makeVisibleCollidablePrototype(
			AABB{ enemy::automatonHitbox },
			PlayerCollisions::Source{},
			EnemyCollisions::Target{ components::CollisionCommands::damage },
			SpriteInstruction{
				spriteStorage.get(L"shadowMaid1")->sprite,
				config::enemyDepth
			},
			game::AnimationList{
				components::Animation {
					{
						L"shadowMaid1",
						L"shadowMaid2",
						L"shadowMaid3",
						L"shadowMaid4"
					},
					true
				},
				enemy::bossAnimationTick
			},
			Health{ enemy::spawnHealth },
			Outbound{ enemy::outbound },
			DeathCommand{ DeathCommand::Commands::deathSpawn },
			DeathSpawn{ ScriptList{ {
				scriptStorage.get(L"enemyExplode"),
				std::string{ ScriptList::spawnString } + "enemyExplode"
			} } }
		).heapClone());
		add("shadowBoss", EntityBuilder::makeVisibleCollidablePrototype(
			AABB{ enemy::bossHitbox },
			PlayerCollisions::Source{},
			EnemyCollisions::Target{ components::CollisionCommands::damage },
			SpriteInstruction{
				spriteStorage.get(L"shadowBoss1")->sprite,
				config::enemyDepth
			},
			game::AnimationList{
				components::Animation {
					{
						L"shadowBoss1",
						L"shadowBoss2",
						L"shadowBoss3",
						L"shadowBoss4"
					},
					true
				},
				enemy::bossAnimationTick
			},
			Health{ enemy::spawnHealth },
			Outbound{ enemy::outbound },
			DeathCommand{ DeathCommand::Commands::deathSpawn },
			DeathSpawn{ ScriptList{ {
				scriptStorage.get(L"bossExplode"),	//enemy is big so use boss explode
				std::string{ ScriptList::spawnString } + "bossExplode"
			} } }
		).heapClone());
		add("shadowPlayer", EntityBuilder::makeVisiblePrototype(
			SpriteInstruction{
				spriteStorage.get(L"shadowPlayerIdle")->sprite,
				config::enemyDepth,
				wasp::math::Vector2{ 0.0f, 4.0f }	//sprite offset
			},
			AnimationList{
				{
					components::Animation{ {
						L"shadowPlayerLeft"
					} },
					components::Animation{ {
						L"shadowPlayerLeftTurn"
					} },
					components::Animation{ {
						L"shadowPlayerIdle"
					} },
					components::Animation{ {
						L"shadowPlayerRightTurn"
					} },
					components::Animation{ {
						L"shadowPlayerRight"
					} }
				},
				2,	//idle index
				4	//ticks
			},
			DeathCommand{ DeathCommand::Commands::deathSpawn },
			DeathSpawn{ ScriptList{ {
				scriptStorage.get(L"bossExplode"),
				std::string{ ScriptList::spawnString } + "bossExplode"
			} } }
		).heapClone());
		add("flame", EntityBuilder::makeVisibleCollidablePrototype(
			AABB{ enemy::flameHitbox },
			PlayerCollisions::Source{},
			EnemyCollisions::Target{ components::CollisionCommands::damage },
			SpriteInstruction{
				spriteStorage.get(L"flame1")->sprite,
				config::enemyDepth,
				{ 0.0, -4.0 }
			},
			game::AnimationList{
				components::Animation {
					{
						L"flame1",
						L"flame2",
						L"flame3",
						L"flame4"
					},
					true
				},
				5
			},
			Health{ enemy::spawnHealth },
			Outbound{ enemy::outbound },
			DeathCommand{ DeathCommand::Commands::deathSpawn },
			DeathSpawn{ ScriptList{ {
				scriptStorage.get(L"enemyExplode"),
				std::string{ ScriptList::spawnString } + "enemyExplode"
			} } }
		).heapClone());
		
		//bosses
		add("boss1", EntityBuilder::makeVisiblePrototype(
			Hitbox{ enemy::bossHitbox },
			PlayerCollisions::Source{},
			EnemyCollisions::Target{ components::CollisionCommands::damage },
			SpriteInstruction{
				spriteStorage.get(L"b1Idle1")->sprite,
				config::enemyDepth + 100
			},
			game::AnimationList{
				components::Animation {
					{
						L"b1Idle1",
						L"b1Idle2",
						L"b1Idle3",
						L"b1Idle4"
					},
					true
				},
				enemy::bossAnimationTick
			},
			Health{ enemy::spawnHealth },
			DeathCommand{ DeathCommand::Commands::bossDeath },
			DeathSpawn{ ScriptList{ {
				scriptStorage.get(L"clearBullets"),
				"clearBullets"
			} } }
		).heapClone());
		
		add("boss2", EntityBuilder::makeVisiblePrototype(
			Hitbox{ enemy::bossHitbox },
			PlayerCollisions::Source{},
			EnemyCollisions::Target{ components::CollisionCommands::damage },
			SpriteInstruction{
				spriteStorage.get(L"b2Idle1")->sprite,
				config::enemyDepth + 100
			},
			game::AnimationList{
				components::Animation {
					{
						L"b2Idle1",
						L"b2Idle2",
						L"b2Idle3",
						L"b2Idle4"
					},
					true
				},
				enemy::bossAnimationTick
			},
			Health{ enemy::spawnHealth },
			DeathCommand{ DeathCommand::Commands::bossDeath },
			DeathSpawn{ ScriptList{ {
				scriptStorage.get(L"clearBullets"),
				"clearBullets"
			} } }
		).heapClone());
		
		add("boss3", EntityBuilder::makeVisiblePrototype(
			Hitbox{ enemy::bossHitbox },
			PlayerCollisions::Source{},
			EnemyCollisions::Target{ components::CollisionCommands::damage },
			SpriteInstruction{
				spriteStorage.get(L"b3Idle1")->sprite,
				config::enemyDepth + 100
			},
			game::AnimationList{
				components::Animation {
					{
						L"b3Idle1",
						L"b3Idle2",
						L"b3Idle3",
						L"b3Idle4"
					},
					true
				},
				enemy::bossAnimationTick
			},
			Health{ enemy::spawnHealth },
			DeathCommand{ DeathCommand::Commands::bossDeath },
			DeathSpawn{ ScriptList{ {
				scriptStorage.get(L"clearBullets"),
				"clearBullets"
			} } }
		).heapClone());
		
		add("boss4", EntityBuilder::makeVisiblePrototype(
			Hitbox{ enemy::bossHitbox },
			PlayerCollisions::Source{},
			EnemyCollisions::Target{ components::CollisionCommands::damage },
			SpriteInstruction{
				spriteStorage.get(L"b4Idle1")->sprite,
				config::enemyDepth + 100
			},
			game::AnimationList{
				components::Animation {
					{
						L"b4Idle1",
						L"b4Idle2",
						L"b4Idle3",
						L"b4Idle4"
					},
					true
				},
				enemy::bossAnimationTick
			},
			Health{ enemy::spawnHealth },
			DeathCommand{ DeathCommand::Commands::bossDeath },
			DeathSpawn{ ScriptList{ {
				scriptStorage.get(L"clearBullets"),
				"clearBullets"
			} } }
		).heapClone());
		
		add("boss5", EntityBuilder::makeVisiblePrototype(
			Hitbox{ enemy::bossHitbox },
			PlayerCollisions::Source{},
			EnemyCollisions::Target{ components::CollisionCommands::damage },
			SpriteInstruction{
				spriteStorage.get(L"b5Idle1")->sprite,
				config::enemyDepth + 100
			},
			game::AnimationList{
				components::Animation {
					{
						L"b5Idle1",
						L"b5Idle2",
						L"b5Idle3",
						L"b5Idle4"
					},
					true
				},
				enemy::bossAnimationTick
			},
			Health{ enemy::spawnHealth },
			DeathCommand{ DeathCommand::Commands::bossDeath },
			DeathSpawn{ ScriptList{ {
				scriptStorage.get(L"clearBullets"),
				"clearBullets"
			} } }
		).heapClone());
	}
	
	Prototypes::ComponentTupleSharedPointer Prototypes::get(
		const std::string& prototypeID
	) const{
		const auto& found{ prototypeMap.find(prototypeID) };
		if(found != prototypeMap.end()){
			return found->second;
		}
		else{
			throw std::runtime_error{ "failed to find prototype " + prototypeID };
		}
	}
	
	void Prototypes::addEnemyProjectile(
		resources::ScriptStorage& scriptStorage,
		resources::SpriteStorage& spriteStorage,
		const std::string& idPrefix,
		const std::string& type,
		float hitbox,
		const std::string& color,
		int relativeDepth
	){
		add(idPrefix + color, EntityBuilder::makeVisibleCollidablePrototype(
			AABB{ hitbox },
			PlayerCollisions::Source{ components::CollisionCommands::death },
			BulletCollisions::Target{ components::CollisionCommands::death },
			ClearMarker{},
			Damage{ 1 },
			SpriteInstruction{
				spriteStorage.get(stringUtil::convertToWideString(type + color))->sprite,
				config::enemyBulletDepth + relativeDepth
			},
			Outbound{ enemyProjectile::outbound },
			DeathCommand{ DeathCommand::Commands::deathSpawn },
			DeathSpawn{ ScriptList{ {
				scriptStorage.get(L"projectileExplode"),
				std::string{ ScriptList::spawnString } + "projectileExplode"
			} } }
		).heapClone());
	}
	
	void Prototypes::addEnemyForwardProjectile(
		resources::ScriptStorage& scriptStorage,
		resources::SpriteStorage& spriteStorage,
		const std::string& idPrefix,
		const std::string& type,
		float hitbox,
		const std::string& color,
		int relativeDepth
	){
		add(idPrefix + color, EntityBuilder::makeVisibleCollidablePrototype(
			AABB{ hitbox },
			PlayerCollisions::Source{ components::CollisionCommands::death },
			BulletCollisions::Target{ components::CollisionCommands::death },
			ClearMarker{},
			Damage{ 1 },
			SpriteInstruction{
				spriteStorage.get(stringUtil::convertToWideString(type + color))->sprite,
				config::enemyBulletDepth + relativeDepth
			},
			RotateSpriteForwardMarker{},
			Outbound{ enemyProjectile::outbound },
			DeathCommand{ DeathCommand::Commands::deathSpawn },
			DeathSpawn{ ScriptList{ {
				scriptStorage.get(L"projectileExplode"),
				std::string{ ScriptList::spawnString } + "projectileExplode"
			} } }
		).heapClone());
	}
	
	void Prototypes::add(
		const std::string& prototypeID,
		const ComponentTupleSharedPointer& prototypePointer
	){
		if(prototypeMap.find(prototypeID) != prototypeMap.end()){
			throw std::runtime_error{ "try to add pre-existing prototypeID: " + prototypeID };
		}
		prototypeMap[prototypeID] = prototypePointer;
	}
}