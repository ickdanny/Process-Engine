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
		
		//add native vars
		addNativeVariable("angleEquivalenceEpsilon", 0.05f);
		addNativeVariable("pointEquivalenceEpsilon", 0.5f);
		
		//add native functions
		
		//native operator handlers
		addNativeFunction(darkness::reservedFunctionNames::unaryMinus, nativeUnaryMinus);
		addNativeFunction(darkness::reservedFunctionNames::binaryPlus, nativeBinaryPlus);
		addNativeFunction(darkness::reservedFunctionNames::binaryMinus, nativeBinaryMinus);
		
		//utility functions
		addNativeFunction("error", throwError);
		addNativeFunction("print", print);
		addNativeFunction("timer", std::bind(&ScriptSystem::timer, this, _1));
		addNativeFunction("stall", std::bind(&ScriptSystem::stall, this, _1));
		addNativeFunction("stallUntil", stallUntil);
		
		//condition queries
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
		
		//entity graphics
		addNativeFunction("setVisible", std::bind(&ScriptSystem::setVisible, this, _1));
		addNativeFunction("removeVisible",
			std::bind(&ScriptSystem::removeComponent<VisibleMarker>, this, "removeVisible", _1)
		);
		addNativeFunction("setSpriteInstruction",
			std::bind(&ScriptSystem::setSpriteInstruction, this, _1)
		);
		addNativeFunction("setDepth", std::bind(&ScriptSystem::setDepth, this, _1));
		
		//entity mutators
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
		addNativeFunction("setInbound", std::bind(&ScriptSystem::setInbound, this, _1));
		addNativeFunction("removeInbound",
			std::bind(&ScriptSystem::removeComponent<Inbound>, this, "removeInbound", _1)
		);
		addNativeFunction("setOutbound", std::bind(&ScriptSystem::setOutbound, this, _1));
		addNativeFunction("removeOutbound",
			std::bind(&ScriptSystem::removeComponent<Outbound>, this, "removeOutbound", _1)
		);
		addNativeFunction("setPosition", std::bind(&ScriptSystem::setPosition, this, _1));
		addNativeFunction("setVelocity", std::bind(&ScriptSystem::setVelocity, this, _1));
		addNativeFunction("setSpeed", std::bind(&ScriptSystem::setSpeed, this, _1));
		addNativeFunction("setAngle", std::bind(&ScriptSystem::setAngle, this, _1));
		addNativeFunction("die", std::bind(&ScriptSystem::die, this, _1));
		addNativeFunction("removeEntity", std::bind(&ScriptSystem::removeEntity, this, _1));
		
		//multi-scripting
		addNativeFunction("addSpawn", std::bind(&ScriptSystem::addSpawn, this, _1));
		addNativeFunction("clearSpawns", std::bind(&ScriptSystem::flagClearSpawns, this, _1));
		addNativeFunction("addScript", std::bind(&ScriptSystem::addScript, this, _1));
		
		//math
		addNativeFunction("makePoint", makePoint);
		addNativeFunction("makeVector", makeVector);
		addNativeFunction("makePolar", makePolar);
		addNativeFunction("getX", getX);
		addNativeFunction("getY", getY);
		addNativeFunction("getR", getR);
		addNativeFunction("getTheta", getTheta);
		addNativeFunction("pow", exponent);
		addNativeFunction("min", min);
		addNativeFunction("max", max);
		addNativeFunction("smallerDifference", smallerDifference);
		addNativeFunction("largerDifference", largerDifference);
		addNativeFunction("abs", absoluteValue);
		addNativeFunction("pointDist", pointDistance);
		addNativeFunction("pointAngle", pointAngle);
		addNativeFunction("random", std::bind(&ScriptSystem::random, this, _1));
		
		//entity queries
		addNativeFunction("angleToPlayer", std::bind(&ScriptSystem::angleToPlayer, this, _1));
		addNativeFunction("entityPos", std::bind(&ScriptSystem::entityPosition, this, _1));
		addNativeFunction("entitySpeed", std::bind(&ScriptSystem::entitySpeed, this, _1));
		addNativeFunction("entityAngle", std::bind(&ScriptSystem::entityAngle, this, _1));
		
		//scene signaling
		addNativeFunction("showDialogue", std::bind(&ScriptSystem::showDialogue, this, _1));
		addNativeFunction("win", std::bind(&ScriptSystem::win, this, _1));
		addNativeFunction("endStage", std::bind(&ScriptSystem::endStage, this, _1));
		
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
		static const Topic<Group*> groupPointerStorageTopic {};
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
	
	float ScriptSystem::getAsFloat(const DataType& data){
		switch(data.index()){
			case floatIndex:
				return std::get<float>(data);
			case intIndex:
				return static_cast<float>(std::get<int>(data));
			default:
				throw std::runtime_error{ "expected int or float" };
		}
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
	
	ScriptSystem::DataType ScriptSystem::nativeUnaryMinus(
		const std::vector<DataType>& parameters
	) {
		throwIfNativeFunctionWrongArity(1, parameters, "native unary minus");
		const DataType& data{ parameters[0] };
		if(std::holds_alternative<Vector2>(data)){
			return -std::get<Vector2>(data);
		}
		else if(std::holds_alternative<PolarVector>(data)){
			return -std::get<PolarVector>(data);
		}
		else{
			throw std::runtime_error{ "native unary minus bad type!" };
		}
	}
	
	ScriptSystem::DataType ScriptSystem::nativeBinaryPlus(
		const std::vector<DataType>& parameters
	) {
		throwIfNativeFunctionWrongArity(2, parameters, "native binary plus");
		const DataType& leftData{ parameters[0] };
		const DataType& rightData{ parameters[1] };
		if(std::holds_alternative<Point2>(leftData)){
			const Point2& left{ std::get<Point2>(leftData) };
			if(std::holds_alternative<Vector2>(rightData)){
				const Vector2& right{ std::get<Vector2>(rightData) };
				return left + right;
			}
			else if(std::holds_alternative<PolarVector>(rightData)){
				const Velocity& right{ std::get<PolarVector>(rightData) };
				return left + right;
			}
			else{
				throw std::runtime_error{ "native binary plus bad right type for point!" };
			}
		}
		else if(std::holds_alternative<Vector2>(leftData)){
			const Vector2& left{ std::get<Vector2>(leftData) };
			if(std::holds_alternative<Vector2>(rightData)){
				const Vector2& right{ std::get<Vector2>(rightData) };
				return left + right;
			}
			else if(std::holds_alternative<PolarVector>(rightData)){
				const Velocity& right{ std::get<PolarVector>(rightData) };
				return left + right;
			}
			else{
				throw std::runtime_error{ "native binary plus bad right type for vector!" };
			}
		}
		else if(std::holds_alternative<PolarVector>(leftData)){
			const PolarVector& left{ std::get<PolarVector>(leftData) };
			if(std::holds_alternative<Vector2>(rightData)){
				const Vector2& right{ std::get<Vector2>(rightData) };
				return left + right;
			}
			else if(std::holds_alternative<PolarVector>(rightData)){
				const Velocity& right{ std::get<PolarVector>(rightData) };
				return left + right;
			}
			else{
				throw std::runtime_error{ "native binary plus bad right type for polar!" };
			}
		}
		else{
			throw std::runtime_error{ "native binary plus bad left type!" };
		}
	}
	
	ScriptSystem::DataType ScriptSystem::nativeBinaryMinus(
		const std::vector<DataType>& parameters
	) {
		throwIfNativeFunctionWrongArity(2, parameters, "native binary minus");
		const DataType& leftData{ parameters[0] };
		const DataType& rightData{ parameters[1] };
		if(std::holds_alternative<Point2>(leftData)){
			const Point2& left{ std::get<Point2>(leftData) };
			if(std::holds_alternative<Vector2>(rightData)){
				const Vector2& right{ std::get<Vector2>(rightData) };
				return left - right;
			}
			else if(std::holds_alternative<PolarVector>(rightData)){
				const Velocity& right{ std::get<PolarVector>(rightData) };
				return left - right;
			}
			else{
				throw std::runtime_error{ "native binary minus bad right type for point!" };
			}
		}
		else if(std::holds_alternative<Vector2>(leftData)){
			const Vector2& left{ std::get<Vector2>(leftData) };
			if(std::holds_alternative<Vector2>(rightData)){
				const Vector2& right{ std::get<Vector2>(rightData) };
				return left - right;
			}
			else if(std::holds_alternative<PolarVector>(rightData)){
				const Velocity& right{ std::get<PolarVector>(rightData) };
				return left - right;
			}
			else{
				throw std::runtime_error{ "native binary minus bad right type for vector!" };
			}
		}
		else if(std::holds_alternative<PolarVector>(leftData)){
			const PolarVector& left{ std::get<PolarVector>(leftData) };
			if(std::holds_alternative<Vector2>(rightData)){
				const Vector2& right{ std::get<Vector2>(rightData) };
				return left - right;
			}
			else if(std::holds_alternative<PolarVector>(rightData)){
				const Velocity& right{ std::get<PolarVector>(rightData) };
				return left - right;
			}
			else{
				throw std::runtime_error{ "native binary minus bad right type for polar!" };
			}
		}
		else{
			throw std::runtime_error{ "native binary minus bad left type!" };
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
				spriteInstruction.setScale(getAsFloat(parameters[4]));
				//fall through
			case 4:
				spriteInstruction.setRotation(getAsFloat(parameters[3]));
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
	 * either Point2 position OR float x, float y
	 */
	ScriptSystem::DataType ScriptSystem::setPosition(const std::vector<DataType>& parameters) {
		throwIfNativeFunctionArityOutOfRange(1, 2, parameters, "setPosition");
		Point2 position;
		if(parameters.size() == 1){
			position = std::get<Point2>(parameters[0]);
		}
		else{
			position = Point2{
				getAsFloat(parameters[0]),
				getAsFloat(parameters[1])
			};
		}
		EntityHandle entityHandle{ makeCurrentEntityHandle() };
		auto& dataStorage{ currentScenePointer->getDataStorage() };
		if(dataStorage.containsComponent<Position>(entityHandle)){
			Position& entityPosition{ dataStorage.getComponent<Position>(entityHandle) };
			entityPosition = position;
		}
		else {
			componentOrderQueue.queueSetComponent<Position>(
				entityHandle,
				Position{ position }
			);
		}
		return false;
	}
	
	/**
	 * either Velocity velocity OR float magnitude, float angle
	 */
	ScriptSystem::DataType ScriptSystem::setVelocity(const std::vector<DataType>& parameters) {
		throwIfNativeFunctionArityOutOfRange(1, 2, parameters, "setVelocity");
		Velocity velocity;
		if(parameters.size() == 1){
			velocity = std::get<PolarVector>(parameters[0]);
		}
		else{
			velocity = Velocity{
				getAsFloat(parameters[0]),
				getAsFloat(parameters[1])
			};
		}
		EntityHandle entityHandle{ makeCurrentEntityHandle() };
		componentOrderQueue.queueSetComponent<Velocity>(entityHandle, velocity);
		return false;
	}
	
	/**
	 * float x, float y
	 */
	ScriptSystem::DataType ScriptSystem::makePoint(const std::vector<DataType>& parameters) {
		throwIfNativeFunctionWrongArity(2, parameters, "makePoint");
		return Point2{
			getAsFloat(parameters[0]),
			getAsFloat(parameters[1])
		};
	}
	
	/**
	 * float x, float y
	 */
	ScriptSystem::DataType ScriptSystem::makeVector(const std::vector<DataType>& parameters) {
		throwIfNativeFunctionWrongArity(2, parameters, "makeVector");
		return Vector2{
			getAsFloat(parameters[0]),
			getAsFloat(parameters[1])
		};
	}
	
	/**
	 * float magnitude, float angle
	 */
	ScriptSystem::DataType ScriptSystem::makePolar(const std::vector<DataType>& parameters){
		throwIfNativeFunctionWrongArity(2, parameters, "makePolar");
		return PolarVector{
			getAsFloat(parameters[0]),
		    getAsFloat(parameters[1])
		};
	}
	
	/**
	 * Point2 point OR Vector2 vector
	 */
	ScriptSystem::DataType ScriptSystem::getX(const std::vector<DataType>& parameters){
		throwIfNativeFunctionWrongArity(1, parameters, "getX");
		const DataType& data{ parameters[0] };
		if(std::holds_alternative<Point2>(data)){
			return std::get<Point2>(data).x;
		}
		else if(std::holds_alternative<Vector2>(data)){
			return std::get<Vector2>(data).x;
		}
		else{
			throw std::runtime_error{ "native func getX bad type!" };
		}
	}
	
	/**
	 * Point2 point OR Vector2 vector
	 */
	ScriptSystem::DataType ScriptSystem::getY(const std::vector<DataType>& parameters){
		throwIfNativeFunctionWrongArity(1, parameters, "getY");
		const DataType& data{ parameters[0] };
		if(std::holds_alternative<Point2>(data)){
			return std::get<Point2>(data).y;
		}
		else if(std::holds_alternative<Vector2>(data)){
			return std::get<Vector2>(data).y;
		}
		else{
			throw std::runtime_error{ "native func getY bad type!" };
		}
	}
	
	/**
	 * PolarVector polar
	 */
	ScriptSystem::DataType ScriptSystem::getR(const std::vector<DataType>& parameters){
		throwIfNativeFunctionWrongArity(1, parameters, "getR");
		const DataType& data{ parameters[0] };
		if(std::holds_alternative<PolarVector>(data)){
			return std::get<PolarVector>(data).getMagnitude();
		}
		else{
			throw std::runtime_error{ "native func getR bad type!" };
		}
	}
	
	/**
	 * PolarVector polar
	 */
	ScriptSystem::DataType ScriptSystem::getTheta(const std::vector<DataType>& parameters){
		throwIfNativeFunctionWrongArity(1, parameters, "getTheta");
		const DataType& data{ parameters[0] };
		if(std::holds_alternative<PolarVector>(data)){
			return static_cast<float>(std::get<PolarVector>(data).getAngle());
		}
		else{
			throw std::runtime_error{ "native func getTheta bad type!" };
		}
	}
	
	/**
	 * float base, float exponent
	 */
	ScriptSystem::DataType ScriptSystem::exponent(const std::vector<DataType>& parameters){
		throwIfNativeFunctionWrongArity(2, parameters, "pow");
		return std::powf(getAsFloat(parameters[0]),	getAsFloat(parameters[1]));
	}
	
	/**
	 * float a, float b OR int a, int b
	 */
	ScriptSystem::DataType ScriptSystem::min(const std::vector<DataType>& parameters){
		throwIfNativeFunctionWrongArity(2, parameters, "min");
		const DataType& dataA{ parameters[0] };
		const DataType& dataB{ parameters[1] };
		switch(dataA.index()){
			case floatIndex: {
				float a{ std::get<float>(dataA) };
				switch(dataB.index()){
					case floatIndex: {
						float b { std::get<float>(dataB) };
						return std::fminf(a, b);
					}
					case intIndex:
						throw std::runtime_error{ "native func min type mismatch!" };
					default:
						throw std::runtime_error{ "native func min bad type second arg!" };
				}
			}
			case intIndex: {
				int a{ std::get<int>(dataA) };
				switch(dataB.index()){
					case intIndex:{
						int b{ std::get<int>(dataB) };
						return std::min(a, b);
					}
					case floatIndex:
						throw std::runtime_error{ "native func min type mismatch!" };
					default:
						throw std::runtime_error{ "native func min bad type second arg!" };
				}
			}
			default:
				throw std::runtime_error{ "native func min bad type first arg!" };
		}
	}
	
	/**
	 * float a, float b OR int a, int b
	 */
	ScriptSystem::DataType ScriptSystem::max(const std::vector<DataType>& parameters){
		throwIfNativeFunctionWrongArity(2, parameters, "max");
		const DataType& dataA{ parameters[0] };
		const DataType& dataB{ parameters[1] };
		switch(dataA.index()){
			case floatIndex: {
				float a{ std::get<float>(dataA) };
				switch(dataB.index()){
					case floatIndex: {
						float b { std::get<float>(dataB) };
						return std::fmaxf(a, b);
					}
					case intIndex:
						throw std::runtime_error{ "native func max type mismatch!" };
					default:
						throw std::runtime_error{ "native func max bad type second arg!" };
				}
			}
			case intIndex: {
				int a{ std::get<int>(dataA) };
				switch(dataB.index()){
					case intIndex:{
						int b{ std::get<int>(dataB) };
						return std::max(a, b);
					}
					case floatIndex:
						throw std::runtime_error{ "native func max type mismatch!" };
					default:
						throw std::runtime_error{ "native func max bad type second arg!" };
				}
			}
			default:
				throw std::runtime_error{ "native func max bad type first arg!" };
		}
	}
	
	ScriptSystem::DataType ScriptSystem::angleToPlayer(
		const std::vector<DataType>& parameters
	) {
		Point2 pos{
			currentScenePointer->getDataStorage().getComponent<Position>(currentEntityID)
		};
		
		//get the iterator for players
		static const Topic<Group*> playerGroupPointerStorageTopic{};
		
		auto playerGroupPointer{
			getGroupPointer<PlayerData, Position>(
				*currentScenePointer,
				playerGroupPointerStorageTopic
			)
		};
		auto playerGroupIterator{
			playerGroupPointer->groupIterator<Position>()
		};
		
		if (playerGroupIterator.isValid()) {
			//just grab the first player
			const auto [playerPos] = *playerGroupIterator;
			return static_cast<float>(wasp::math::getAngleFromAToB(pos, playerPos));
		}
		else {
			return 0.0f;
		}
	}
	
	ScriptSystem::DataType ScriptSystem::entityPosition(
		const std::vector<DataType>& parameters)
	{
		return Point2{
			currentScenePointer->getDataStorage().getComponent<Position>(currentEntityID)
		};
	}
	
	/**
	 * float min, float max OR int min, int max
	 */
	ScriptSystem::DataType ScriptSystem::random(const std::vector<DataType>& parameters) {
		throwIfNativeFunctionWrongArity(2, parameters, "random");
		const auto& randomChannel{ currentScenePointer->getChannel(SceneTopics::random) };
		if(randomChannel.isEmpty()){
			throw std::runtime_error{ "native func random no prng in scene!" };
		}
		auto& prng{ currentScenePointer->getChannel(SceneTopics::random).getMessages()[0] };
		const DataType& minData{ parameters[0] };
		const DataType& maxData{ parameters[1] };
		switch(minData.index()){
			case floatIndex: {
				float min{ std::get<float>(minData) };
				switch(maxData.index()){
					case floatIndex: {
						float max { std::get<float>(maxData) };
						std::uniform_real_distribution<float> distribution { min, max };
						return distribution(prng);
					}
					case intIndex:
						throw std::runtime_error{ "native func random type mismatch!" };
					default:
						throw std::runtime_error{ "native func random bad type second arg!" };
				}
				
			}
			case intIndex: {
				int min{ std::get<int>(minData) };
				switch(maxData.index()){
					case intIndex:{
						int max{ std::get<int>(maxData) };
						//inclusive
						std::uniform_int_distribution<int> distribution { min, max };
						return distribution(prng);
					}
					case floatIndex:
						throw std::runtime_error{ "native func random type mismatch!" };
					default:
						throw std::runtime_error{ "native func random bad type second arg!" };
				}
			}
			default:
				throw std::runtime_error{ "native func random bad type first arg!" };
		}
	}
	
	/**
	 * float inbound
	 */
	ScriptSystem::DataType ScriptSystem::setInbound(const std::vector<DataType>& parameters) {
		throwIfNativeFunctionWrongArity(1, parameters, "setInbound");
		float inbound{ getAsFloat(parameters[0]) };
		EntityHandle entityHandle{ makeCurrentEntityHandle() };
		componentOrderQueue.queueSetComponent<Inbound>(entityHandle, { inbound });
		return false;
	}
	
	/**
	 * float outbound
	 */
	ScriptSystem::DataType ScriptSystem::setOutbound(const std::vector<DataType>& parameters) {
		throwIfNativeFunctionWrongArity(1, parameters, "setOutbound");
		float outbound{ getAsFloat(parameters[0]) };
		EntityHandle entityHandle{ makeCurrentEntityHandle() };
		componentOrderQueue.queueSetComponent<Outbound>(entityHandle, { outbound });
		return false;
	}
	
	ScriptSystem::DataType ScriptSystem::entitySpeed(const std::vector<DataType>& parameters) {
		throwIfNativeFunctionWrongArity(0, parameters, "entitySpeed");
		Velocity& velocity{
			currentScenePointer->getDataStorage().getComponent<Velocity>(currentEntityID)
		};
		return velocity.getMagnitude();
	}
	
	ScriptSystem::DataType ScriptSystem::entityAngle(const std::vector<DataType>& parameters) {
		throwIfNativeFunctionWrongArity(0, parameters, "entityAngle");
		Velocity& velocity{
			currentScenePointer->getDataStorage().getComponent<Velocity>(currentEntityID)
		};
		return static_cast<float>(velocity.getAngle());
	}
	
	/**
	 * float speed
	 */
	ScriptSystem::DataType ScriptSystem::setSpeed(const std::vector<DataType>& parameters) {
		throwIfNativeFunctionWrongArity(1, parameters, "setSpeed");
		float speed{ getAsFloat(parameters[0]) };
		Velocity& velocity{
			currentScenePointer->getDataStorage().getComponent<Velocity>(currentEntityID)
		};
		velocity.setMagnitude(speed);
		return false;
	}
	
	/**
	 * float angle
	 */
	ScriptSystem::DataType ScriptSystem::setAngle(const std::vector<DataType>& parameters) {
		throwIfNativeFunctionWrongArity(1, parameters, "setAngle");
		float angle{ getAsFloat(parameters[0]) };
		Velocity& velocity{
			currentScenePointer->getDataStorage().getComponent<Velocity>(currentEntityID)
		};
		velocity.setAngle(angle);
		return false;
	}
	
	/**
	 * float left, float right
	 */
	ScriptSystem::DataType ScriptSystem::smallerDifference(
		const std::vector<DataType>& parameters
	) {
		throwIfNativeFunctionWrongArity(2, parameters, "smallerDifference");
		Angle left{ getAsFloat(parameters[0]) };
		Angle right{ getAsFloat(parameters[1]) };
		return left.smallerDifference(right);
	}
	
	/**
	 * float left, float right
	 */
	ScriptSystem::DataType ScriptSystem::largerDifference(
		const std::vector<DataType>& parameters
	) {
		throwIfNativeFunctionWrongArity(2, parameters, "largerDifference");
		Angle left{ getAsFloat(parameters[0]) };
		Angle right{ getAsFloat(parameters[1]) };
		return left.largerDifference(right);
	}
	
	/**
	 * float value OR int value
	 */
	ScriptSystem::DataType ScriptSystem::absoluteValue(
		const std::vector<DataType>& parameters
	) {
		throwIfNativeFunctionWrongArity(1, parameters, "abs");
		const DataType& data{ parameters[0] };
		switch(data.index()){
			case intIndex:
				return std::abs(std::get<int>(data));
			case floatIndex:
				return std::abs(std::get<float>(data));
			default:
				throw std::runtime_error{ "native func abs bad type!" };
		}
	}
	
	/**
	 * Point2 pointA, Point2 pointB
	 */
	ScriptSystem::DataType ScriptSystem::pointDistance(
		const std::vector<DataType>& parameters
	){
		throwIfNativeFunctionWrongArity(2, parameters, "pointDist");
		const Point2& pointA{ std::get<Point2>(parameters[0]) };
		const Point2& pointB{ std::get<Point2>(parameters[1]) };
		return wasp::math::distanceFromAToB(pointA, pointB);
	}
	
	/**
	 * Point2 pointA, Point2 pointB
	 */
	ScriptSystem::DataType ScriptSystem::pointAngle(const std::vector<DataType>& parameters){
		throwIfNativeFunctionWrongArity(2, parameters, "pointAngle");
		const Point2& pointA{ std::get<Point2>(parameters[0]) };
		const Point2& pointB{ std::get<Point2>(parameters[1]) };
		return static_cast<float>(wasp::math::getAngleFromAToB(pointA, pointB));
	}
	
	ScriptSystem::DataType ScriptSystem::die(const std::vector<DataType>& parameters) {
		throwIfNativeFunctionWrongArity(0, parameters, "die");
		EntityHandle entityHandle{ makeCurrentEntityHandle() };
		currentScenePointer->getChannel(SceneTopics::deaths).addMessage(entityHandle);
		return false;
	}
	
	ScriptSystem::DataType ScriptSystem::removeEntity(const std::vector<DataType>& parameters) {
		throwIfNativeFunctionWrongArity(0, parameters, "removeEntity");
		componentOrderQueue.queueRemoveEntity(makeCurrentEntityHandle());
		return false;
	}
	
	/**
	 * string dialogueID
	 */
	ScriptSystem::DataType ScriptSystem::showDialogue(const std::vector<DataType>& parameters) {
		throwIfNativeFunctionWrongArity(1, parameters, "showDialogue");
		const std::string& dialogueID{ std::get<std::string>(parameters[0]) };
		//add message to sceneEntry and startDialogue topics
		globalChannelSetPointer->getChannel(GlobalTopics::sceneEntry).addMessage(
			SceneNames::dialogue
		);
		globalChannelSetPointer->getChannel(GlobalTopics::startDialogue).addMessage(
			convertToWideString(dialogueID)
		);
		return false;
	}
	
	ScriptSystem::DataType ScriptSystem::win(const std::vector<DataType>& parameters){
		currentScenePointer->getChannel(SceneTopics::winFlag).addMessage();
		return false;
	}
	
	ScriptSystem::DataType ScriptSystem::endStage(const std::vector<DataType>& parameters) {
		throwIfNativeFunctionWrongArity(0, parameters, "endStage");
		globalChannelSetPointer->getChannel(GlobalTopics::stopMusicFlag).addMessage();
		auto& gameStateChannel{ globalChannelSetPointer->getChannel(GlobalTopics::gameState) };
		GameState& gameState{ gameStateChannel.getMessages()[0] };
		
		//send us back to the correct menu
		SceneNames backTo{};
		switch (gameState.gameMode) {
			case GameMode::campaign:
				backTo = SceneNames::main;
				break;
			case GameMode::practice:
				backTo = SceneNames::stage;
				break;
			default:
				throw std::runtime_error{ "native func endStage default case reached!" };
		}
		
		globalChannelSetPointer->getChannel(GlobalTopics::sceneExitTo).addMessage(backTo);
		if(gameState.gameMode == GameMode::campaign) {
			auto& sceneEntryChannel{
				globalChannelSetPointer->getChannel(GlobalTopics::sceneEntry)
			};
			switch (gameState.stage) {
				case 1:
				case 2:
				case 3:
				case 4:
				{
					++gameState.stage;
					//immediately pop up a new game and loading screen
					sceneEntryChannel.addMessage(SceneNames::game);
					sceneEntryChannel.addMessage(SceneNames::load);
					
					//send player data to global
					static const Topic<Group*> playerGroupPointerStorageTopic{};
					
					auto playerGroupPointer{
						getGroupPointer<PlayerData>(
							*currentScenePointer,
							playerGroupPointerStorageTopic
						)
					};
					
					auto playerGroupIterator{
						playerGroupPointer->groupIterator<PlayerData>()
					};
					
					PlayerData playerData{
						ShotType::shotA,
						-1,
						-1,
						-1,
						-1
					};
					while (playerGroupIterator.isValid()) {
						playerData = std::get<0>(*playerGroupIterator);
						break;
					}
					auto& playerDataChannel{
						globalChannelSetPointer->getChannel(
							GlobalTopics::playerData
						)
					};
					playerDataChannel.clear();
					playerDataChannel.addMessage(playerData);
					break;
				}
				case 5:
					//go to credits screen
					sceneEntryChannel.addMessage(SceneNames::credits);
					globalChannelSetPointer->getChannel(GlobalTopics::startMusic).addMessage(
						L"12"
					);
					break;
			}
		}
		else {
			//if we are in practice, then we go back to menu, thus menu track
			globalChannelSetPointer->getChannel(GlobalTopics::startMusic).addMessage(
				L"01"
			);
		}
		return false;
	}
}