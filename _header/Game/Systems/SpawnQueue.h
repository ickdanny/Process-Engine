#pragma once

#include "EntityBuilder.h"

namespace process::game::systems{
	
	class SpawnQueue{
	private:
		//typedefs
		using SpawnPointer = std::shared_ptr<ComponentTupleBase>;
		using SpawnList = std::vector<SpawnPointer>;
		
		//fields
		SpawnList spawnList{};
		
	public:
		void queueSpawn(const SpawnPointer& spawnPointer){
			spawnList.emplace_back(spawnPointer);
		}
		
		void applyAndClear(wasp::ecs::DataStorage& dataStorage) {
			for (const SpawnPointer& spawnPointer : spawnList) {
				spawnPointer->addTo(dataStorage);
			}
			spawnList.clear();
		}
	};
}