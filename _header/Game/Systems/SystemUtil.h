#pragma once

#include "Game/Scenes.h"
#include "GameConfig.h"

namespace process::game::systems {
	
	//Retrieves the group pointer from the channel for the given group pointer topic
	template <typename... Ts>
	wasp::ecs::component::Group* getGroupPointer(
		Scene& scene, 
		wasp::channel::Topic<wasp::ecs::component::Group*> groupPointerStorageTopic
	) {
		wasp::ecs::DataStorage& dataStorage{ scene.getDataStorage() };

		auto& channel{ scene.getChannel(groupPointerStorageTopic) };
		wasp::ecs::component::Group* groupPointer;	//not initialized!
		if (channel.isEmpty()) {
			groupPointer = dataStorage.getGroupPointer<Ts...>();
			channel.addMessage(groupPointer);
		}
		else {
			groupPointer = channel.getMessages()[0];
		}
		return groupPointer;
	}

	//Returns true if the given position is outside of the bounds specified,
	//false otherwise
	bool isOutOfBounds(wasp::math::Point2 pos, float bound);
}