#include "ClearSystem.h"

namespace process::game::systems{
	
	void ClearSystem::operator()(Scene& scene){
		auto& clearChannel{ scene.getChannel(SceneTopics::clearFlag) };
		if(clearChannel.hasMessages()){
			clearChannel.clear();
			handleClear(scene);
		}
	}
	
	void ClearSystem::handleClear(Scene& scene){
		//get the group iterator for ClearMarker
		static const Topic<Group*> groupPointerStorageTopic {};
		auto groupPointer {	getGroupPointer<ClearMarker>(scene, groupPointerStorageTopic) };
		auto groupIterator { groupPointer->groupIterator<ClearMarker>() };
		
		auto& deathsChannel{ scene.getChannel(SceneTopics::deaths) };
		
		while(groupIterator.isValid()){
			const EntityID entityID{ groupIterator.getEntityID() };
			const EntityHandle& entityHandle{ scene.getDataStorage().makeHandle(entityID) };
			deathsChannel.addMessage(entityHandle);
			++groupIterator;
		}
	}
}