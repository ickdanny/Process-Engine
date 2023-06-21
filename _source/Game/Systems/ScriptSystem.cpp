#include "Game/Systems/ScriptSystem.h"

#include <locale>
#include <codecvt>

#include "Logging.h"

namespace process::game::systems {
	
	namespace{
		constexpr char spawnString[] { "spawn" };
		
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
		
		bool containsSpawnString(const std::string& string){
			return string.find(spawnString) != std::string::npos;
		}
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
		addNativeFunction("removeVisible",
			std::bind(&ScriptSystem::removeComponent<VisibleMarker>, this, "removeVisible", _1)
		);
		addNativeFunction("setSpriteInstruction",
			std::bind(&ScriptSystem::setSpriteInstruction, this, _1)
		);
		addNativeFunction("setDepth", std::bind(&ScriptSystem::setDepth, this, _1));
		addNativeFunction("isSpawning", std::bind(&ScriptSystem::isSpawning, this, _1));
		addNativeFunction("isBossDead", std::bind(&ScriptSystem::isBossDead, this, _1));
		addNativeFunction("isDialogueOver", std::bind(&ScriptSystem::isDialogueOver, this, _1));
		addNativeFunction("isWin", std::bind(&ScriptSystem::isWin, this, _1));
		addNativeFunction("isXAbove",
			std::bind(&ScriptSystem::checkCoordinate<true, true>, this, _1)
		);
		addNativeFunction("isXBelow",
			std::bind(&ScriptSystem::checkCoordinate<true, false>, this, _1)
		);
		addNativeFunction("isYAbove",
			std::bind(&ScriptSystem::checkCoordinate<false, true>, this, _1)
		);
		addNativeFunction("isYBelow",
			std::bind(&ScriptSystem::checkCoordinate<false, false>, this, _1)
		);
		addNativeFunction("setCollidable", std::bind(&ScriptSystem::setCollidable, this, _1));
		addNativeFunction("removeCollidable",
			std::bind(
				&ScriptSystem::removeComponent<CollidableMarker>,
				this,
				"removeCollidable",
				_1
			)
		);
		addNativeFunction("setHealth", std::bind(&ScriptSystem::setHealth, this, _1));
		addNativeFunction("removeHealth",
			std::bind(&ScriptSystem::removeComponent<Health>, this, "removeHealth", _1)
		);
		addNativeFunction("setDamage", std::bind(&ScriptSystem::setDamage, this, _1));
		addNativeFunction("removeDamage",
			std::bind(&ScriptSystem::removeComponent<Damage>, this, "removeDamage", _1)
		);
		addNativeFunction("addSpawn", std::bind(&ScriptSystem::addSpawn, this, _1));
		addNativeFunction("clearSpawns", std::bind(&ScriptSystem::flagClearSpawns, this, _1));
		addNativeFunction("addScript", std::bind(&ScriptSystem::addScript, this, _1));
		addNativeFunction("setVelocity", std::bind(&ScriptSystem::setVelocity, this, _1));
		
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
			scriptsToAddToCurrentEntity.clear();
			
			runScriptList(scriptList);
			
			//clear spawns
			if(clearSpawnsFlag){
				clearSpawns(scriptList);
				clearSpawnsFlag = false;
			}
			
			//add any new scripts
			if(!scriptsToAddToCurrentEntity.empty()) {
				scriptList.insert(
					scriptList.end(),
					std::make_move_iterator(scriptsToAddToCurrentEntity.begin()),
					std::make_move_iterator(scriptsToAddToCurrentEntity.end())
				);
			}
			
			
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
	
	void ScriptSystem::throwIfNativeFunctionWrongArity(
		std::size_t expectedArity,
		const std::vector<DataType>& parameters,
		const std::string& funcName
	){
		if(parameters.size() != expectedArity){
			throw std::runtime_error{
				"native func " + funcName + " wrong arity, expected "
				+ std::to_string(expectedArity) + " but got "
				+ std::to_string(parameters.size())
			};
		}
	}
	
	void ScriptSystem::throwIfNativeFunctionArityOutOfRange(
		std::size_t arityMinInclusive,
		std::size_t arityMaxInclusive,
		const std::vector<DataType>& parameters,
		const std::string& funcName
	){
		std::size_t parametersSize{ parameters.size() };
		if(parametersSize < arityMinInclusive || parametersSize > arityMaxInclusive){
			throw std::runtime_error{
				"native func " + funcName + " wrong arity, expected "
				+ std::to_string(arityMinInclusive)	+ "-" + std::to_string(arityMaxInclusive)
				+ " but got " + std::to_string(parametersSize)
			};
		}
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
			//if the script is stalled after being run, go to the next script
			if(scriptContainer.state.stalled){
				++itr;
			}
			//if the script is finished after being run, remove it
			else{
				itr = scriptList.erase(itr);
			}
		}
	}
	
	void ScriptSystem::clearSpawns(ScriptList& scriptList){
		//for each script attached to the current entity
		for( auto itr { scriptList.begin() }; itr != scriptList.end(); ) {
			auto& scriptContainer { *itr };
			//if the script is a spawn, remove it
			if(containsSpawnString(scriptContainer.name)){
				itr = scriptList.erase(itr);
			}
			else{
				++itr;
			}
		}
	}
	
	/**
	 * any... toPrint
	 */
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
	
	/**
	 * int ticks
	 */
	ScriptSystem::DataType ScriptSystem::timer(const std::vector<DataType>& parameters) {
		int& containerTimer = currentScriptContainerPointer->timer;
		//if there is a positive timer, tick down the timer and continue stalling
		if(containerTimer > 0){
			--containerTimer;
			throw StallFlag{};
		}
		//if there is no timer, begin the timer and stall
		else if(containerTimer == ScriptContainer::noTimer){
			throwIfNativeFunctionWrongArity(1, parameters, "timer");
			containerTimer = std::get<int>(parameters[0]);
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
			throwIfNativeFunctionWrongArity(0, parameters, "stall");
			++containerTimer;
			throw StallFlag{};
		}
		//otherwise, exit
		else{
			containerTimer = ScriptContainer::noTimer;
		}
		return false;
	}
	
	/**
	 * func condition, params... passToCondition
	 */
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
		throwIfNativeFunctionArityOutOfRange(0, 1, parameters, "error");
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
	
	ScriptSystem::DataType ScriptSystem::setVisible(
		const std::vector<DataType>& parameters
	){
		throwIfNativeFunctionWrongArity(0, parameters, "setVisible");
		EntityHandle entityHandle{ makeCurrentEntityHandle() };
		componentOrderQueue.queueSetComponent<VisibleMarker>(entityHandle, {});
		return false;
	}
	
	/**
	 * string spriteID, int depth, Vector2 offset, float rotation, float scale
	 */
	ScriptSystem::DataType ScriptSystem::setSpriteInstruction(
		const std::vector<DataType>& parameters
	) {
		throwIfNativeFunctionArityOutOfRange(2, 5, parameters, "setSpriteInstruction");
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
		}
		EntityHandle entityHandle{ makeCurrentEntityHandle() };
		componentOrderQueue.queueSetComponent<SpriteInstruction>(
			entityHandle,
			spriteInstruction
		);
		return false;
	}
	
	/**
	 * int depth
	 */
	ScriptSystem::DataType ScriptSystem::setDepth(const std::vector<DataType>& parameters){
		throwIfNativeFunctionWrongArity(1, parameters, "setDepth");
		int depth{ std::get<int>(parameters[0]) };
		EntityHandle entityHandle{ makeCurrentEntityHandle() };
		if(!containsComponent<SpriteInstruction>(entityHandle)){
			throw std::runtime_error{ "native func setDepth no sprite instruction!" };
		}
		auto& spriteInstruction{ getComponent<SpriteInstruction>(entityHandle) };
		spriteInstruction.setDepth(depth);
		return false;
	}
	
	ScriptSystem::DataType ScriptSystem::isSpawning(const std::vector<DataType>& parameters) {
		throwIfNativeFunctionWrongArity(0, parameters, "isSpawning");
		EntityHandle entityHandle{ makeCurrentEntityHandle() };
		//since this is the script system, the entity obviously has a script list
		const auto& scriptList{ getComponent<ScriptList>(entityHandle) };
		for(const auto& scriptContainer : scriptList){
			if(containsSpawnString(scriptContainer.name)){
				return true;
			}
		}
		return false;
	}
	
	ScriptSystem::DataType ScriptSystem::isBossDead(const std::vector<DataType>& parameters) {
		throwIfNativeFunctionWrongArity(0, parameters, "isBossDead");
		auto& bossDeathsChannel{
			currentScenePointer->getChannel(SceneTopics::bossDeaths)
		};
		if (bossDeathsChannel.hasMessages()) {
			bossDeathsChannel.clear();
			return true;
		}
		return false;
	}
	
	ScriptSystem::DataType ScriptSystem::isDialogueOver(
		const std::vector<DataType>& parameters
	) {
		throwIfNativeFunctionWrongArity(0, parameters, "isDialogueOver");
		auto& endDialogueFlagChannel{
			globalChannelSetPointer->getChannel(GlobalTopics::endDialogueFlag)
		};
		if (endDialogueFlagChannel.hasMessages()) {
			endDialogueFlagChannel.clear();
			return true;
		}
		return false;
	}
	
	ScriptSystem::DataType ScriptSystem::isWin(const std::vector<DataType>& parameters) {
		throwIfNativeFunctionWrongArity(0, parameters, "isWin");
		auto& winFlagChannel{
			currentScenePointer->getChannel(SceneTopics::winFlag)
		};
		if (winFlagChannel.hasMessages()) {
			winFlagChannel.clear();
			return true;
		}
		return false;
	}
	
	ScriptSystem::DataType ScriptSystem::setCollidable(
		const std::vector<DataType>& parameters
	) {
		throwIfNativeFunctionWrongArity(0, parameters, "setCollidable");
		EntityHandle entityHandle{ makeCurrentEntityHandle() };
		componentOrderQueue.queueSetComponent<CollidableMarker>(entityHandle, {});
		return false;
	}
	
	/**
	 * int health
	 */
	ScriptSystem::DataType ScriptSystem::setHealth(const std::vector<DataType>& parameters) {
		throwIfNativeFunctionWrongArity(1, parameters, "setHealth");
		int health{ std::get<int>(parameters[0]) };
		EntityHandle entityHandle{ makeCurrentEntityHandle() };
		componentOrderQueue.queueSetComponent<Health>(entityHandle, { health });
		return false;
	}
	
	/**
	 * int damage
	 */
	ScriptSystem::DataType ScriptSystem::setDamage(const std::vector<DataType>& parameters) {
		throwIfNativeFunctionWrongArity(1, parameters, "setDamage");
		int damage{ std::get<int>(parameters[0]) };
		EntityHandle entityHandle{ makeCurrentEntityHandle() };
		componentOrderQueue.queueSetComponent<Damage>(entityHandle, { damage });
		return false;
	}
	
	/**
	 * string spawnName
	 */
	ScriptSystem::DataType ScriptSystem::addSpawn(const std::vector<DataType>& parameters) {
		throwIfNativeFunctionWrongArity(1, parameters, "addSpawn");
		const std::string& spawnID{ std::get<std::string>(parameters[0]) };
		const auto& scriptPointer{ scriptStoragePointer->get(convertToWideString(spawnID)) };
		scriptsToAddToCurrentEntity.push_back({
				scriptPointer,
				std::string { spawnString } + "_" + spawnID
		});
		return false;
	}
	
	ScriptSystem::DataType ScriptSystem::flagClearSpawns(
		const std::vector<DataType>& parameters
	) {
		throwIfNativeFunctionWrongArity(0, parameters, "flagClearSpawns");
		clearSpawnsFlag = true;
		return false;
	}
	
	/**
	 * string scriptName
	 */
	ScriptSystem::DataType ScriptSystem::addScript(const std::vector<DataType>& parameters) {
		throwIfNativeFunctionWrongArity(1, parameters, "addSpawn");
		const std::string& scriptID{ std::get<std::string>(parameters[0]) };
		const auto& scriptPointer{ scriptStoragePointer->get(convertToWideString(scriptID)) };
		scriptsToAddToCurrentEntity.push_back({
			scriptPointer,
			scriptID
		});
		return false;
	}
	
	/**
	 * either Velocity velocity OR float magnitude, float angle
	 */
	ScriptSystem::DataType ScriptSystem::setVelocity(const std::vector<DataType>& parameters) {
		throwIfNativeFunctionArityOutOfRange(1, 2, parameters, "setVelocity");
		Velocity velocity;
		if(parameters.size() == 1){
			velocity = std::get<Velocity>(parameters[0]);
		}
		else{
			velocity = Velocity{
				std::get<float>(parameters[0]),
				std::get<float>(parameters[1])
			};
		}
		//todo: set velocity
	}
}