#include "Game/Systems/ScriptSystem.h"

#include <locale>
#include <codecvt>

#include "Logging.h"

namespace process::game::systems {
	
	namespace{
		#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
		std::string convertFromWideString(const std::wstring& wideString){
			//setup converter
			using convert_type = std::codecvt_utf8<wchar_t>;
			std::wstring_convert<convert_type, wchar_t> converter{};

			return converter.to_bytes( wideString );
		}
		
		std::wstring convertToWideString(const std::string& string){
			using convert_type = std::codecvt_utf8<wchar_t>;
			std::wstring_convert<convert_type, wchar_t> converter{};
			
			return converter.from_bytes(string);
		}
		#undef _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
	}
	
	using namespace wasp::ecs;
	using namespace wasp::ecs::entity;
	using namespace std::placeholders;	//for binding with _1
	using ResourceSharedPointer = std::shared_ptr<resources::ScriptStorage::ResourceType>;
	
	ScriptSystem::ScriptSystem(
		wasp::channel::ChannelSet* globalChannelSetPointer,
		resources::ScriptStorage* scriptStoragePointer,
		resources::SpriteStorage* spriteStoragePointer
	)
		: globalChannelSetPointer { globalChannelSetPointer }
		, scriptStoragePointer{ scriptStoragePointer }
		, spriteStoragePointer{ spriteStoragePointer } {
		//add native functions
		addNativeFunction("print", print);
		addNativeFunction("timer", std::bind(&ScriptSystem::timer, this, _1));
		addNativeFunction("stall", std::bind(&ScriptSystem::stall, this, _1));
		addNativeFunction("stallUntil", stallUntil);
		addNativeFunction("error", throwError);
		addNativeFunction("removeVisible", std::bind(&ScriptSystem::removeVisible, this, _1));
		addNativeFunction("setSpriteInstruction",
						  std::bind(&ScriptSystem::setSpriteInstruction, this, _1));
		addNativeFunction("setDepth", std::bind(&ScriptSystem::setDepth, this, _1));
		
		//load function scripts, which are files that start with keyword func
		darkness::Lexer lexer{};
		std::vector<std::string> paramNames{};
		scriptStoragePointer->forEach([&](const ResourceSharedPointer& resourceSharedPointer){
			const std::wstring& wideID{ resourceSharedPointer->getID() };
			const std::string shortID{ convertFromWideString(wideID) };
			//test to see if the filename starts with keyword func
			if(shortID.find("func") == 0){
				paramNames.clear();
				//split string into tokens with lexer
				const auto& tokens{ lexer.lex(shortID) };
				auto itr{ tokens.begin() };
				++itr;
				//grab the func name, which should always be the second token
				const std::string& funcName{ std::get<std::string>(itr->value) };
				++itr;
				//all identifiers following the func name are param names
				for(auto end{ tokens.end() }; itr != end; ++itr){
					if(itr->type == darkness::TokenType::identifier){
						paramNames.push_back(std::get<std::string>(itr->value));
					}
				}
				//add the script with the param names from the file name
				addFunctionScript(
					funcName,
					resourceSharedPointer->getDataPointerCopy(),
					paramNames
				);
			}
		});
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
	
	EntityHandle ScriptSystem::makeCurrentEntityHandle(){
		return currentScenePointer->getDataStorage().makeHandle(currentEntityID);
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
	
	ScriptSystem::DataType ScriptSystem::throwError(const std::vector<DataType>& parameters){
		if(parameters.empty()){
			throw std::runtime_error{ "Darkness runtime error" };
		}
		const DataType& paramData{ parameters.front() };
		switch(paramData.index()){
			case intIndex:
				throw std::runtime_error{
					"Darkness runtime error: " + std::to_string(std::get<int>(paramData))
				};
			case floatIndex:
				throw std::runtime_error{
					"Darkness runtime error: " + std::to_string(std::get<float>(paramData))
				};
			case boolIndex:
				throw std::runtime_error{
					"Darkness runtime error: " + std::to_string(std::get<bool>(paramData))
				};
			case stringIndex:
				throw std::runtime_error{
					"Darkness runtime error: " + std::get<std::string>(paramData)
				};
			default:
				throw std::runtime_error{ "Darkness runtime error of unknown type" };
		}
	}
	
	ScriptSystem::DataType ScriptSystem::removeVisible(
		const std::vector<DataType>& parameters
	) {
		EntityHandle entityHandle{ makeCurrentEntityHandle() };
		componentOrderQueue.queueRemoveComponent<VisibleMarker>(
			entityHandle
		);
		return false;
	}
	
	/**
	 * String spriteID, int depth, Vector2 offset, float rotation, float scale
	 */
	ScriptSystem::DataType ScriptSystem::setSpriteInstruction(
		const std::vector<DataType>& parameters
	) {
		if(parameters.size() < 2){
			throw std::runtime_error{ "native func setSpriteInstruction too few params" };
		}
		const std::string& spriteID{ std::get<std::string>(parameters[0]) };
		const auto& sprite{ spriteStoragePointer->get(convertToWideString(spriteID))->sprite };
		const int& depth{ std::get<int>(parameters[1]) };
		SpriteInstruction spriteInstruction{sprite, depth};
		switch(parameters.size()){
			case 5:
				spriteInstruction.setScale(std::get<float>(parameters[4]));
				//fall through
			case 4:
				spriteInstruction.setRotation(std::get<float>(parameters[3]));
				//fall through
			case 3:
				spriteInstruction.setOffset(std::get<wasp::math::Vector2>(parameters[2]));
				break;
			default:
				throw std::runtime_error{ "native func setSpriteInstruction too many params" };
		}
		EntityHandle entityHandle{ makeCurrentEntityHandle() };
		componentOrderQueue.queueSetComponent<SpriteInstruction>(
			entityHandle,
			spriteInstruction
		);
		return false;
	}
	
	ScriptSystem::DataType ScriptSystem::setDepth(const std::vector<DataType>& parameters){
		if(parameters.empty()){
			throw std::runtime_error{ "native func setDepth needs a param!" };
		}
		const int& depth{ std::get<int>(parameters[0]) };
		EntityHandle entityHandle{ makeCurrentEntityHandle() };
		if(!containsComponent<SpriteInstruction>(entityHandle)){
			throw std::runtime_error{ "native func setDepth no sprite instruction!" };
		}
		auto& spriteInstruction{ getComponent<SpriteInstruction>(entityHandle) };
		spriteInstruction.setDepth(depth);
		return false;
	}
}