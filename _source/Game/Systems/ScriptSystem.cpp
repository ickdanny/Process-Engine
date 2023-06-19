#include "Game/Systems/ScriptSystem.h"

#include "Logging.h"

namespace process::game::systems {
	
	using namespace wasp::ecs;
	using namespace wasp::ecs::entity;
	
	ScriptSystem::ScriptSystem(wasp::channel::ChannelSet* globalChannelSetPointer)
		: globalChannelSetPointer { globalChannelSetPointer } {
		//add native functions
		addNativeFunction("print", print);
		//todo: should load scripts as callable functions
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
			if(!scriptContainer.scriptPointer){
				throw std::runtime_error{ "bad script pointer!" };
			}
			//if the script is not stalled, run the script
			if(!scriptContainer.state.stalled){
				scriptContainer.state = runScript(*scriptContainer.scriptPointer);
				
			}
			//if the script IS stalled, resume the script
			else{
				scriptContainer.state = resumeScript(
					*scriptContainer.scriptPointer,
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
	
	ScriptSystem::DataType ScriptSystem::print(const std::vector<DataType>& parameters){
		for(const DataType& data : parameters){
			switch(data.index()){
				case boolIndex:
					if(std::get<bool>(data)){
						wasp::debug::log("true");
					}
					else{
						wasp::debug::log("false");
					}
					break;
				case intIndex:
					wasp::debug::log(std::to_string(std::get<int>(data)));
					break;
				case floatIndex:
					wasp::debug::log(std::to_string(std::get<float>(data)));
					break;
				case stringIndex:
					wasp::debug::log(std::get<std::string>(data));
					break;
				case functionIndex:
					throw std::runtime_error("native func print cannot print a function");
				default:
					throw std::runtime_error("native func print bad type");
			}
		}
		return false;
	}
}