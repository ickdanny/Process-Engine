#include "Prototypes.h"

namespace process::game::systems{
	namespace enemyProjectiles{
		constexpr float smallHitbox{ 3.0f };
		constexpr float mediumHitbox{ 6.15f };
		constexpr float largeHitbox{ 10.0f };
		constexpr float sharpHitbox{ 3.75f };
		constexpr float outbound{ -21.0f };
	}
	
	namespace playerA{
		constexpr float boomerangHitbox{ 9.0f };
		constexpr int boomerangDamage{ 10 };
		constexpr float boomerangOutbound{ -50.0f };
		constexpr float boomerangSpin{ 21.728172f };
		
		constexpr float scytheHitbox{ 50.0f };
		constexpr int scytheDamage{ 1 };//damage per tick
		constexpr int scytheExplodeDamage{ 60 };
		constexpr float scytheSpin{ 19.4892f };
	}
	
	namespace playerB{
		constexpr float laserHitbox{ 10.0f };
		constexpr int laserDamage{ 7 };
		constexpr float laserOutbound{ -20.0f };
		
		constexpr float laserPartHitbox{ 40.0f };
		constexpr int laserPartDamage{ 1 };//damage per tick
		constexpr float haloHitbox{ 50.0f };
		constexpr int haloDamage{ 1 };//damage per tick
	}
	
	using AABB = wasp::math::AABB;
	
	Prototypes::Prototypes(
		resources::SpriteStorage& spriteStorage,
		resources::ScriptStorage& scriptStorage
	){
		add("test", EntityBuilder::makeVisibleCollidablePrototype(
			AABB{ 0.0f },
			SpriteInstruction{
				spriteStorage.get(L"life")->sprite,
				config::playerDepth
			}
		).heapClone());
		add("projectileExplode", EntityBuilder::makeVisiblePrototype(
			SpriteInstruction{
				spriteStorage.get(L"explode_1")->sprite,
				config::playerBulletDepth - 1
			},
			game::AnimationList{
				components::Animation {
					{
						L"explode_1",
						L"explode_2",
						L"explode_3"
					},
					false
				},
				6
			},
			ScriptList{ {
				scriptStorage.get(L"removeExplode"),
				"removeExplode"
			} }
		).heapClone());
		//playerA
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
		add("scytheExplode", EntityBuilder::makeVisiblePrototype(
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
		//playerB
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
			AABB{ playerB::laserPartHitbox },
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
			AABB{ playerB::haloHitbox },
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