#include "Game/Systems/ScriptSystem.h"

#include "Logging.h"

namespace process::game::systems {
	
	using namespace wasp::ecs;
	using namespace wasp::ecs::entity;
	
	namespace {
		
		constexpr float angleEquivalenceEpsilon { .05f };
		constexpr float pointEquivalenceEpsilon { .5f };
		
		constexpr float gotoDeceleratingExponentBase { 2.0f };
		constexpr float gotoDeceleratingHorizontalShift { 0.1f };
		constexpr float gotoDeceleratingHorizontalStretch { 7.0f };
		
		float calculateGotoDeceleratingSpeed(
			float currentDistance,
			float initDistance,
			float maxSpeed
		) {
			float distanceRatio { currentDistance / initDistance };
			float exponent {            //a linear function based on distanceRatio; mx+b
				(gotoDeceleratingHorizontalStretch * distanceRatio) +
					gotoDeceleratingHorizontalShift
			};
			float speedMulti {
				1.0f - (1.0f / std::powf(gotoDeceleratingExponentBase, exponent))
			};
			return std::fminf(maxSpeed * speedMulti, currentDistance);
		}
	}
	
	ScriptSystem::ScriptSystem(wasp::channel::ChannelSet* globalChannelSetPointer)
		: globalChannelSetPointer { globalChannelSetPointer } {
	}
	
	void ScriptSystem::operator()(Scene& scene) {
		//load the current scene
		currentScenePointer = &scene;
		
		//get the group iterator for ScriptProgramList
		static const Topic<wasp::ecs::component::Group*> groupPointerStorageTopic {};
		auto groupPointer {	getGroupPointer<ScriptList>(scene, groupPointerStorageTopic) };
		auto groupIterator { groupPointer->groupIterator<ScriptList>() };
		
		//populate our component order queue with every component order this tick
		while( groupIterator.isValid() ) {
			auto [scriptList] = *groupIterator;
			//load the current entity
			currentEntityID = groupIterator.getEntityID();
			runScriptList(scriptList);
			++groupIterator;
		}
		
		//applyAndClear all our queued component orders to the ecs world
		componentOrderQueue.applyAndClear(scene.getDataStorage());
		//unload current scene
		currentScenePointer = nullptr;
	}
	
	void ScriptSystem::runScriptList(ScriptList& scriptList) {
		//for each script attached to the current entity
		for( auto itr { scriptList.begin() }; itr != scriptList.end(); ) {
			auto& scriptContainer { *itr };
			//if the script is not stalled, run the script
			if(!scriptContainer.state.stalled){
				scriptContainer.state = runScript(scriptContainer.script);
				
			}
			//if the script is stalled, resume the script
			else{
				scriptContainer.state = resumeScript(
					scriptContainer.script,
					scriptContainer.state
				);
			}
			//if the script is not tagged as runForever, check to see if its finished
			if(!scriptContainer.runForever && !scriptContainer.state.stalled){
				//if the script is finished, remove it
				itr = scriptList.erase(itr);
			}
			//if the script IS tagged as runForever, we don't care about its state right now
			else{
				++itr;
			}
		}
	}
}