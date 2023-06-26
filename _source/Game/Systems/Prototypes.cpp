#include "Prototypes.h"

namespace process::game::systems{
	namespace{
		constexpr float smallHitbox{ 3.0f };
		constexpr float mediumHitbox{ 6.15f };
		constexpr float largeHitbox{ 10.0f };
		constexpr float sharpHitbox{ 3.75f };
		constexpr float outbound{ -21.0f };
	}
	
	using AABB = wasp::math::AABB;
	
	Prototypes::Prototypes(resources::SpriteStorage& spriteStorage){
		add("test", EntityBuilder::makeVisibleCollidablePrototype(
			AABB{ 0.0f },
			SpriteInstruction{
				spriteStorage.get(L"life")->sprite,
				config::playerDepth
			}
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