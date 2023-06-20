#include "Game/Systems/ScriptSystem.h"

#include "Logging.h"

namespace process::game::systems {
	
	using namespace wasp::ecs;
	using namespace wasp::ecs::entity;
	using namespace std::placeholders;	//for binding with _1
	using ResourceSharedPointer = std::shared_ptr<resources::ScriptStorage::ResourceType>;
	
	ScriptSystem::ScriptSystem(
		wasp::channel::ChannelSet* globalChannelSetPointer,
		resources::ScriptStorage* scriptStoragePointer
	)
		: globalChannelSetPointer { globalChannelSetPointer }
		, scriptStoragePointer{ scriptStoragePointer } {
		//add native functions
		addNativeFunction("print", print);
		addNativeFunction("timer", std::bind(&ScriptSystem::timer, this, _1));
		addNativeFunction("stall", std::bind(&ScriptSystem::stall, this, _1));
		addNativeFunction("stallUntil", stallUntil);
		//load function scripts manually
		addFunctionScript("helloWorld", scriptStoragePointer->get(L"hello_world"));
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
			//load the current script container
			currentScriptContainerPointer = &scriptContainer;
			
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
					throw std::runtime_error{ "native func print cannot print a function" };
				default:
					throw std::runtime_error{ "native func print bad type" };
			}
		}
		return false;
	}
	
	ScriptSystem::DataType ScriptSystem::timer(const std::vector<DataType>& parameters) {
		int& containerTimer = currentScriptContainerPointer->timer;
		//if there is a positive timer, tick down the timer and continue stalling
		if(containerTimer > 0){
			--containerTimer;
			throw StallFlag{};
		}
		//if there is no timer, begin the timer and stall
		else if(containerTimer == ScriptContainer::noTimer){
			if(parameters.size() != 1){
				throw std::runtime_error{
					"native func timer wrong arg num: " + std::to_string(parameters.size())
				};
			}
			const DataType& paramData{ parameters.front() };
			if(paramData.index() != intIndex){
				throw std::runtime_error{ "native func timer needs int arg!" };
			}
			containerTimer = std::get<int>(paramData);
			throw StallFlag{};
		}
		//if the timer is over, set to no timer and exit
		else if(containerTimer == 0){
			containerTimer = ScriptContainer::noTimer;
		}
		//if there is a negative timer, that's an error
		else{
			throw std::runtime_error{
				"native func timer unexpected timer: " + std::to_string(containerTimer)
			};
		}
		return false;
	}
	
	ScriptSystem::DataType ScriptSystem::stall(const std::vector<DataType>& parameters){
		int& containerTimer = currentScriptContainerPointer->timer;
		//if there is no timer, stall
		if(containerTimer == ScriptContainer::noTimer){
			++containerTimer;
			throw StallFlag{};
		}
		//otherwise, exit
		else{
			containerTimer = ScriptContainer::noTimer;
		}
		return false;
	}
	
	ScriptSystem::DataType ScriptSystem::stallUntil(const std::vector<DataType>& parameters){
		if(parameters.empty()){
			throw std::runtime_error{ "native func stallUntil received no params!" };
		}
		const DataType& paramData{ parameters.front() };
		const NativeFunction& paramNativeFunction{ unwrapNativeFunctionFromData(paramData) };
		DataType paramReturn;
		if(parameters.size() > 1) {
			//pass all the rest of the parameters except the native function itself
			std::vector<DataType> passParams(parameters.begin() + 1, parameters.end());
			paramReturn = paramNativeFunction(passParams);
		}
		else{
			paramReturn = paramNativeFunction({});
		}
		if(!std::holds_alternative<bool>(paramReturn)){
			throw std::runtime_error{ "native func stallUntil received not bool!" };
		}
		//if we got true, exit
		if(std::get<bool>(paramReturn)){
			return false;
		}
		//otherwise, keep stalling
		else{
			throw StallFlag{};
		}
	}
}