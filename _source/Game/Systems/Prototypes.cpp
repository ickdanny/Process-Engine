#include "Prototypes.h"

#include "StringUtil.h"

namespace process::game::systems{
	
	
	namespace playerA{
		constexpr float boomerangHitbox{ 9.0f };
		constexpr int boomerangDamage{ 10 };
		constexpr float boomerangOutbound{ -50.0f };
		constexpr float boomerangSpin{ 21.728172f };
		
		constexpr float scytheHitbox{ 35.0f };
		constexpr int scytheDamage{ 1 };//damage per tick
		constexpr int scytheExplodeDamage{ 60 };
		constexpr float scytheSpin{ 19.4892f };
	}
	
	namespace playerB{
		constexpr float laserHitbox{ 10.0f };
		constexpr int laserDamage{ 7 };
		constexpr float laserOutbound{ -20.0f };
		
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
		constexpr float machineHitbox{ 13.0f };
		constexpr float machine2Hitbox{ 12.0f };
	}
	
	namespace enemyProjectile{
		constexpr float smallHitbox{ 2.5f };
		constexpr float mediumHitbox{ 4.0f };
		constexpr float largeHitbox{ 10.0f };
		constexpr float sharpHitbox{ 3.75f };
		constexpr float outbound{ -21.0f };
	}
	
	using AABB = wasp::math::AABB;
	
	Prototypes::Prototypes(
		resources::ScriptStorage& scriptStorage,
		resources::SpriteStorage& spriteStorage
	){
		add("projectileExplode", EntityBuilder::makeVisiblePrototype(
			SpriteInstruction{
				spriteStorage.get(L"projectileExplode1")->sprite,
				config::playerBulletDepth - 1
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
		#define addMedium(color) \
			addEnemyProjectile( \
				scriptStorage, \
				spriteStorage, \
				"medium", \
				enemyProjectile::mediumHitbox, \
				color \
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
				enemyProjectile::smallHitbox, \
				color \
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
		
		//MOBS
		add("enemyExplode", EntityBuilder::makeVisiblePrototype(
			SpriteInstruction{
				spriteStorage.get(L"enemyExplode1")->sprite,
				config::enemyDepth - 100
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
				config::enemyDepth - 100
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
		const std::string& type,
		float hitbox,
		const std::string& color
	){
		add(type + color, EntityBuilder::makeVisibleCollidablePrototype(
			AABB{ hitbox },
			PlayerCollisions::Source{ components::CollisionCommands::death },
			BulletCollisions::Target{ components::CollisionCommands::death },
			ClearMarker{},
			Damage{ 1 },
			SpriteInstruction{
				spriteStorage.get(stringUtil::convertToWideString(type + color))->sprite,
				config::enemyBulletDepth
			},
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