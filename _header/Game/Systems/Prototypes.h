#pragma once

#include "Game/Systems/EntityBuilder.h"
#include "SpriteStorage.h"
#include "ScriptStorage.h"

namespace process::game::systems{

	class Prototypes{
	private:
		//typedefs
		using Angle = wasp::math::Angle;
		using AABB = wasp::math::AABB;
		using Point2 = wasp::math::Point2;
		using Vector2 = wasp::math::Vector2;
		using PolarVector = wasp::math::PolarVector;
		using ComponentTupleSharedPointer = std::shared_ptr<ComponentTupleBase>;
		
		//fields
		std::unordered_map<std::string, ComponentTupleSharedPointer> prototypeMap{};
	public:
		Prototypes(
			resources::ScriptStorage& scriptStorage,
			resources::SpriteStorage& spriteStorage
		);
		
		ComponentTupleSharedPointer get(const std::string& prototypeID) const;
		
	private:
		void addPlayerPrototypes(
			resources::ScriptStorage& scriptStorage,
			resources::SpriteStorage& spriteStorage
		);
		
		void addPickupPrototypes(
			resources::ScriptStorage& scriptStorage,
			resources::SpriteStorage& spriteStorage
		);
		
		void addEnemyPrototypes(
			resources::ScriptStorage& scriptStorage,
			resources::SpriteStorage& spriteStorage
		);
		
		void addEnemyProjectile(
			resources::ScriptStorage& scriptStorage,
			resources::SpriteStorage& spriteStorage,
			const std::string& idPrefix,
			const std::string& type,
			float hitbox,
			const std::string& color,
			int relativeDepth
		);
		
		void addEnemyForwardProjectile(
			resources::ScriptStorage& scriptStorage,
			resources::SpriteStorage& spriteStorage,
			const std::string& idPrefix,
			const std::string& type,
			float hitbox,
			const std::string& color,
			int relativeDepth
		);
		
		void add(
			const std::string& prototypeID,
			const ComponentTupleSharedPointer& prototypePointer
		);
	};
}