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
				spriteStorage.get(L"small_laser")->sprite,
				config::playerBulletDepth
			},
			RotateSpriteForwardMarker{},
			game::DeathCommand{ game::DeathCommand::Commands::deathSpawn },
			DeathSpawn{ ScriptList{ {
				scriptStorage.get(L"projectileExplode"),
				std::string{ ScriptList::spawnString } + "projectileExplode"
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