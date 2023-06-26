#pragma once

#include "Game/Systems/EntityBuilder.h"
#include "SpriteStorage.h"

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
		Prototypes(resources::SpriteStorage& spriteStorage);
		
		ComponentTupleSharedPointer get(const std::string& prototypeID) const;
		
	private:
		void add(
			const std::string& prototypeID,
			const ComponentTupleSharedPointer& prototypePointer
		);
	};
}