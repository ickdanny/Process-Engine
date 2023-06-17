#pragma once

#include "Ast.h"

#include <stdexcept>
#include <functional>
#include <any>
#include <utility>

namespace darkness{
	
	namespace reservedFunctionNames{
		/**
		 * The reserved function name for the user-defined unary bang operator. Users must
		 * guarantee that any native unary bang operator they provide to the interpreter
		 * will never stall.
		 */
		static const std::string unaryBang{ "unaryBang" };
		/**
		 * The reserved function name for the user-defined unary plus operator. Users must
		 * guarantee that any native unary plus operator they provide to the interpreter
		 * will never stall.
		 */
		static const std::string unaryPlus{ "unaryPlus" };
		/**
		 * The reserved function name for the user-defined unary minus operator. Users must
		 * guarantee that any native unary minus operator they provide to the interpreter
		 * will never stall.
		 */
		static const std::string unaryMinus{ "unaryMinus" };
		
		/**
		 * The reserved function name for the user-defined binary plus operator. Users must
		 * guarantee that any native binary plus operator they provide to the interpreter
		 * will never stall.
		 */
		static const std::string binaryPlus{ "binaryPlus" };
		/**
		 * The reserved function name for the user-defined binary minus operator. Users must
		 * guarantee that any native binary minus operator they provide to the interpreter
		 * will never stall.
		 */
		static const std::string binaryMinus{ "binaryMinus" };
		/**
		 * The reserved function name for the user-defined binary star operator. Users must
		 * guarantee that any native binary star operator they provide to the interpreter
		 * will never stall.
		 */
		static const std::string binaryStar{ "binaryStar" };
		/**
		 * The reserved function name for the user-defined binary forward slash operator.
		 * Users must guarantee that any native binary forward slash operator they provide to
		 * the interpreter will never stall.
		 */
		static const std::string binaryForwardSlash{ "binaryForwardSlash" };
		/**
		 * The reserved function name for the user-defined binary dual equal operator. Users
		 * must guarantee that any native binary dual equal operator they provide to the
		 * interpreter will never stall.
		 */
		static const std::string binaryDualEqual{ "binaryDualEqual" };
		/**
		 * The reserved function name for the user-defined binary greater operator. Users must
		 * guarantee that any native binary greater operator they provide to the interpreter
		 * will never stall.
		 */
		static const std::string binaryGreater{ "binaryGreater" };
	}
	
	template <typename... CustomTypes>
	class Interpreter{
	protected:
		//typedefs
		using NativeFunctionWrapper = std::any;
		struct UserFunctionWrapper{
			std::vector<std::string> paramNames{};
			std::shared_ptr<AstNode> body{};
		};
		using FunctionWrapper = std::variant<NativeFunctionWrapper, UserFunctionWrapper>;
		using DataType = std::variant<
			bool,
			int,
			float,
			std::string,
			FunctionWrapper,
			CustomTypes...
		>;
		using NativeFunction = std::function<DataType(const std::vector<DataType>&)>;
		
		struct StallFlag{};
		struct StallNodeInfo{
			AstType type{};
			int index{};
			
			//special indices
			static constexpr int ifCondition{ -10 };
			static constexpr int ifTrue{ -11 };
			static constexpr int ifFalse{ -12 };
			static constexpr int whileCondition{ -20 };
			static constexpr int whileBody{ -21 };
			static constexpr int binLeft{ -31 };
			static constexpr int binRight{ -32 };
			static constexpr int callFuncExpr{ -40 };
			static constexpr int callNative{ -41 };
			static constexpr int callUser{ -42 };
		};
		struct StallingNativeFunctionCall{
			NativeFunction stallingNativeFunction{};
			std::vector<DataType> args{};
			
			DataType run(){
				return stallingNativeFunction(args);
			}
		};
		
		class Environment;	//definition at bottom of file
		
		//constants
		static constexpr auto numTypes{ std::variant_size_v<DataType> };
		static constexpr auto numCustomTypes{ sizeof...(CustomTypes) };
		static constexpr auto numBuiltInTypes{ numTypes - numCustomTypes };
		static constexpr auto boolIndex{ 0u };
		static constexpr auto intIndex{ 1u };
		static constexpr auto floatIndex{ 2u };
		static constexpr auto stringIndex{ 3u };
		static constexpr auto functionIndex{ 4u };
		
		//fields
		std::shared_ptr<Environment> nativeEnvironmentPointer{};
		std::shared_ptr<Environment> innermostEnvironmentPointer{};
		std::vector<std::variant<StallNodeInfo, DataType>> stallStack{};
		StallingNativeFunctionCall stallingNativeFunctionCall{};
		DataType stallReturn{ false };
		//todo: could possibly refactor the stall fields (and innermost environment ptr) out
		
	public:
		/**
		 * Constructs an interpreter with an empty native and base environment, with the base
		 * environment pointing to the native environment.
		 */
		Interpreter()
			: nativeEnvironmentPointer{ std::make_shared<Environment>() }
			, innermostEnvironmentPointer{
				std::make_shared<Environment>(nativeEnvironmentPointer)
			}{
		}
	
	protected:
		/**
		 * Binds a native function to the native environment. If the native function is to be
		 * an operator handler as defined by the reserved function names, users must guarantee
		 * that those operator handlers will never stall.
		 */
		void addNativeFunction(const std::string& name, const NativeFunction& function){
			if(nativeEnvironmentPointer->contains(name)){
				throwError(
					"trying to define native function " + name + " but "
					+ name + " is an already defined variable in the native environment"
				);
			}
			nativeEnvironmentPointer->define(
				name,
				DataType{ FunctionWrapper{ NativeFunctionWrapper{ function } } }
			);
		}
		
		/**
		 * Binds a variable to the native environment. The variable can be any datatype
		 * accepted by this interpreter.
		 */
		void addNativeVariable(const std::string& name, const DataType& data){
			if(nativeEnvironmentPointer->contains(name)){
				throwError(
					"trying to define native variable " + name + " but "
					+ name + " is an already defined variable in the native environment"
				);
			}
			nativeEnvironmentPointer->define(name, data);
		}
		
	public:
		//todo: maybe let scripts return stuff? Can define functions in different scripts
		/**
		 * Runs a darkness script. If the given AstNode is of any other type, throws an error.
		 * A script may stall on any of its statements. Returns true if the script completed,
		 * false if the script stalled.
		 */
		bool runScript(const AstNode& script){
			throwIfNotType(script, AstType::script, "trying to run not script!");
			int currentIndex{ 0 };
			const auto& statements{ std::get<AstScriptData>(script.dataVariant).statements };
			try{
				for(; currentIndex < statements.size(); ++currentIndex){
					runStatement(statements[currentIndex]);
				}
			}
			catch(const StallFlag&){
				//stalled on a native function! vomit onto the stack and exit
				pushStallNodeInfo({ AstType::script, currentIndex });
				return false;
			}
			return true;
		}
		
		/**
		 * Resumes a stalled darkness script. If the given AstNode is of any other type, throws
		 * an error. The script may stall on the same statement, or it may stall on a new
		 * statement. Returns true if the script completed, false if the script stalled again.
		 */
		 bool resumeScript(const AstNode& script){
			throwIfNotType(script, AstType::script, "trying to resume not script!");
			//make sure interpreter was stalled
			if(stallingNativeFunctionCall.stallingNativeFunction == nullptr){
				throwError("trying to resume a script but was not stalled!");
			}
			//try rerunning the stalled native function
			try{
				stallReturn = stallingNativeFunctionCall.run();
			}
			catch(const StallFlag&){
				//stalled again on the same native function! exit prematurely
				return false;
			}
			//successfully ran - reset stalling native function call to nothing
			stallingNativeFunctionCall = {};
			//get the stall info
			const StallNodeInfo& stallNodeInfo{ popLastStallNodeInfo() };
			throwIfNotType(stallNodeInfo, AstType::script, "bad stall type: not script!");
			//resume running the script
			int currentIndex{ stallNodeInfo.index };
			const auto& statements{ std::get<AstScriptData>(script.dataVariant).statements };
			try{
				//resume the stalled statement
				resumeStatement(statements[currentIndex]);
				++currentIndex;
				//run the rest of the statements like normal
				for(; currentIndex < statements.size(); ++currentIndex){
					runStatement(statements[currentIndex]);
				}
			}
			catch(const StallFlag&){
				//stalled on a different native function! vomit onto the stack and exit
				pushStallNodeInfo({ AstType::script, currentIndex });
				return false;
			}
			return true;
		 }
		 
	private:
		/**
		 * Runs a statement, which are nodes which do not evaluate to a value. Some statements
		 * may stall, while others are guaranteed not to stall. This method checks the type
		 * of AstNode it is given and delegates to the appropriate run method.
		 */
		void runStatement(const AstNode& statement){
			switch(statement.type){
				case AstType::stmtVarDeclare:
					runVarDeclare(statement);
					break;
				case AstType::stmtFuncDeclare:
					runFuncDeclare(statement);
					break;
				case AstType::stmtIf:
					runIf(statement);
					break;
				case AstType::stmtWhile:
					runWhile(statement);
					break;
				case AstType::stmtReturn:
					runReturn(statement);
					break;
				case AstType::stmtBlock:
					runBlock(statement);
					break;
				case AstType::stmtExpression:
					runExpression(
						*std::get<AstStmtExpressionData>(statement.dataVariant).expression
					);
					break;
				default:
					throwError("trying to run not a statement!");
			}
		}
		
		/**
		 * Resumes a statement. Statements do not evaluate to a value. This method checks the
		 * type of AstNode it is given and delegates to the appropriate resume method. Throws
		 * an error if it is discovered that the node should have never stalled in the first
		 * place.
		 */
		void resumeStatement(const AstNode& statement) {
			switch(statement.type){
				case AstType::stmtVarDeclare:
					resumeVarDeclare(statement);
					break;
				case AstType::stmtFuncDeclare:
					throwError("somehow stalled on a func declare!");
					break;
				case AstType::stmtIf:
					resumeIf(statement);
					break;
				case AstType::stmtWhile:
					resumeWhile(statement);
					break;
				case AstType::stmtReturn:
					resumeReturn(statement);
					break;
				case AstType::stmtBlock:
					resumeBlock(statement);
					break;
				case AstType::stmtExpression:
					resumeExpression(
						*std::get<AstStmtExpressionData>(statement.dataVariant).expression
					);
					break;
				default:
					throwError("trying to resume not a statement!");
			}
		}
		
		/**
		 * Runs a var declare node. Since var declares can only stall on their initializer
		 * expression, this type of node does not vomit onto the stack.
		 */
		void runVarDeclare(const AstNode& varDeclare){
			throwIfNotType(varDeclare, AstType::stmtVarDeclare, "not var declare!");
			const auto& data{
				std::get<AstStmtVarDeclareData>(varDeclare.dataVariant)
			};
			const auto& initializer{ data.initializer };
			DataType initData;	//uninitialized!
			if(initializer){
				initData = runExpression(*initializer);
			}
			else{
				initData = DataType{ false };
			}
			innermostEnvironmentPointer->define(data.varName, initData);
		}
		
		/**
		 * Resumes a var declare node. Since var declares can only stall on their initializer
		 * expression, this type of node will not pop the stack.
		 */
		void resumeVarDeclare(const AstNode& varDeclare){
			throwIfNotType(varDeclare, AstType::stmtVarDeclare, "not var declare!");
			const auto& data{
				std::get<AstStmtVarDeclareData>(varDeclare.dataVariant)
			};
			const auto& initializer{ data.initializer };
			DataType initData;	//uninitialized!
			if(initializer){
				initData = resumeExpression(*initializer);
			}
			else{
				throwError("somehow resumed var declare with no initializer");
			}
			innermostEnvironmentPointer->define(data.varName, initData);
		}
		
		/**
		 * Runs a func declare node. This type of node should never stall.
		 */
		void runFuncDeclare(const AstNode& funcDeclare){
			throwIfNotType(funcDeclare, AstType::stmtFuncDeclare, "not func declare!");
			const auto& data{
				std::get<AstStmtFuncDeclareData>(funcDeclare.dataVariant)
			};
			UserFunctionWrapper userFunctionWrapper{
				data.paramNames,
				data.body
			};
			FunctionWrapper functionWrapper{ userFunctionWrapper };
			DataType environmentData{ functionWrapper };
			innermostEnvironmentPointer->define(data.funcName, environmentData);
		}
		
		/**
		 * Runs an if node. This type of node may stall on its condition, true,
		 * or false branch.
		 */
		void runIf(const AstNode& ifStatement){
			throwIfNotType(ifStatement, AstType::stmtIf, "not an if statement!");
			const auto& data{ std::get<AstStmtIfData>(ifStatement.dataVariant) };
			DataType conditionValue;//uninitialized!
			try {
				conditionValue = runExpression(*data.condition);
			}
			catch(const StallFlag&){
				pushStallNodeInfo({ AstType::stmtIf, StallNodeInfo::ifCondition });
				throw;
			}
			if( conditionValue.index() != boolIndex ) {
				throwError("if statement condition was not bool!");
			}
			if( std::get<bool>(conditionValue) ) {
				try {
					runStatement(*data.trueBranch);
				}
				catch(const StallFlag&){
					pushStallNodeInfo({ AstType::stmtIf, StallNodeInfo::ifTrue });
					throw;
				}
			}
			else if( data.falseBranch ) {
				try {
					runStatement(*data.falseBranch);
				}
				catch(const StallFlag&){
					pushStallNodeInfo({ AstType::stmtIf, StallNodeInfo::ifFalse });
					throw;
				}
			}
		}
		
		/**
		 * Resumes an if node. The stall may have occurred on its condition, true, or false
		 * branch.
		 */
		void resumeIf(const AstNode& ifStatement){
			throwIfNotType(ifStatement, AstType::stmtIf, "not an if statement!");
			const auto& data{ std::get<AstStmtIfData>(ifStatement.dataVariant) };
			//get the stall info
			const StallNodeInfo& stallNodeInfo{ popLastStallNodeInfo() };
			throwIfNotType(stallNodeInfo, AstType::stmtIf, "bad stall type: not if!");
			switch(stallNodeInfo.index){
				//case 1: resume the true branch
				case StallNodeInfo::ifTrue: {
					//it is possible that we stall again on the true branch.
					try {
						resumeStatement(*data.trueBranch);
					}
					catch(const StallFlag&){
						pushStallNodeInfo({ AstType::stmtIf, StallNodeInfo::ifTrue });
						throw;
					}
					break;
				}
				//case 2: resume the false branch
				case StallNodeInfo::ifFalse: {
					if(!data.falseBranch){
						throwError("trying to resume false branch but doesn't exist!");
					}
					//it is possible that we stall again on the false branch
					try {
						resumeStatement(*data.falseBranch);
					}
					catch(const StallFlag&){
						pushStallNodeInfo({ AstType::stmtIf, StallNodeInfo::ifFalse });
						throw;
					}
					break;
				}
				//case 3: try to evaluate the condition again
				case StallNodeInfo::ifCondition: {
					DataType conditionValue;//uninitialized!
					//it is possible that we stall again on the condition
					try {
						conditionValue = resumeExpression(*data.condition);
					}
					catch(const StallFlag&){
						pushStallNodeInfo({ AstType::stmtIf, StallNodeInfo::ifCondition });
						throw;
					}
					if( conditionValue.index() != boolIndex ) {
						throwError("if statement condition was not bool!");
					}
					//the condition did not stall, meaning we can now run either true or false
					if( std::get<bool>(conditionValue) ) {
						try {
							runStatement(*data.trueBranch);
						}
						catch(const StallFlag&){
							pushStallNodeInfo({ AstType::stmtIf, StallNodeInfo::ifTrue });
							throw;
						}
					}
					else if( data.falseBranch ) {
						try {
							runStatement(*data.falseBranch);
						}
						catch(const StallFlag&){
							pushStallNodeInfo({ AstType::stmtIf, StallNodeInfo::ifFalse });
							throw;
						}
					}
					break;
				}
				default:
					throwError("bad stall index for if: " + stallNodeInfo.index);
			}
		}
		
		/**
		 * Runs a while node. This type of node may stall on its condition or its body.
		 */
		void runWhile(const AstNode& whileStatement){
			throwIfNotType(whileStatement, AstType::stmtWhile, "not a while statement!");
			const auto& data{ std::get<AstStmtWhileData>(whileStatement.dataVariant) };
			DataType conditionValue;//uninitialized!
			//evaluate condition the first time
			try {
				conditionValue = runExpression(*data.condition);
			}
			catch(const StallFlag&){
				pushStallNodeInfo({ AstType::stmtWhile, StallNodeInfo::whileCondition });
				throw;
			}
			if(conditionValue.index() != boolIndex){
				throwError("while statement condition was not bool!");
			}
			//while the condition evaluates to true, run the body of the loop
			while(std::get<bool>(conditionValue)){
				//run body
				try {
					runStatement(*data.body);
				}
				catch(const StallFlag&){
					pushStallNodeInfo({ AstType::stmtWhile, StallNodeInfo::whileBody });
					throw;
				}
				//reevaluate condition
				try {
					conditionValue = runExpression(*data.condition);
				}
				catch(const StallFlag&){
					pushStallNodeInfo({ AstType::stmtWhile, StallNodeInfo::whileCondition });
					throw;
				}
				if(conditionValue.index() != boolIndex){
					throwError("while statement condition was not bool!");
				}
			}
		}
		
		/**
		 * Resumes a while node. The stall may have occurred on its condition or its body.
		 */
		void resumeWhile(const AstNode& whileStatement){
			throwIfNotType(whileStatement, AstType::stmtWhile, "not a while statement!");
			const auto& data{ std::get<AstStmtWhileData>(whileStatement.dataVariant) };
			//get the stall info
			const StallNodeInfo& stallNodeInfo{ popLastStallNodeInfo() };
			throwIfNotType(stallNodeInfo, AstType::stmtWhile, "bad stall type: not while!");
			
			//re-execute the stalled node
			DataType conditionValue;//uninitialized!
			switch(stallNodeInfo.index){
				//case 1: stall occurred in the body - resume the body once
				case StallNodeInfo::whileBody:{
					//the body may stall again
					try {
						resumeStatement(*data.body);
					}
					catch(const StallFlag&){
						pushStallNodeInfo({ AstType::stmtWhile, StallNodeInfo::whileBody });
						throw;
					}
					//body did not stall again, reevaluate condition
					try {
						conditionValue = runExpression(*data.condition);
					}
					catch(const StallFlag&){
						pushStallNodeInfo(
							{ AstType::stmtWhile, StallNodeInfo::whileCondition }
						);
						throw;
					}
					break;
				}
				//case 2: stall occurred in the condition - resume the condition
				case StallNodeInfo::whileCondition:{
					//the condition may stall again
					try {
						conditionValue = resumeExpression(*data.condition);
					}
					catch(const StallFlag&){
						pushStallNodeInfo(
							{ AstType::stmtWhile, StallNodeInfo::whileCondition }
						);
						throw;
					}
					break;
				}
				default:
					throwError("bad stall index for while: " + stallNodeInfo.index);
			}
			//resume the loop - all control paths have evaluated the condition once by now
			if(conditionValue.index() != boolIndex){
				throwError("while statement condition was not bool!");
			}
			while(std::get<bool>(conditionValue)){
				//run body
				try {
					runStatement(*data.body);
				}
				catch(const StallFlag&){
					pushStallNodeInfo({ AstType::stmtWhile, StallNodeInfo::whileBody });
					throw;
				}
				//reevaluate condition
				try {
					conditionValue = runExpression(*data.condition);
				}
				catch(const StallFlag&){
					pushStallNodeInfo({ AstType::stmtWhile, StallNodeInfo::whileCondition });
					throw;
				}
				if(conditionValue.index() != boolIndex){
					throwError("while statement condition was not bool!");
				}
			}
		}
		
		/**
		 * Runs a return node. Since returns can only stall on evaluating their value, this
		 * type of node does not vomit onto the stack
		 */
		void runReturn(const AstNode& returnStatement){
			throwIfNotType(returnStatement, AstType::stmtReturn, "not a return statement!");
			const auto& data{ std::get<AstStmtReturnData>(returnStatement.dataVariant) };
			
			//void returns actually just return false
			if(data.hasValue){
				throw runExpression(*data.value);
			}
			else{
				throw DataType{ false };
			}
			//blocks and functions will reset environment even if throw
		}
		
		/**
		 * Resumes a return node. Since returns can only stall on evaluating their value, this
		 * type of node will not pop the stack.
		 */
		void resumeReturn(const AstNode& returnStatement){
			throwIfNotType(returnStatement, AstType::stmtReturn, "not a return statement!");
			const auto& data{ std::get<AstStmtReturnData>(returnStatement.dataVariant) };
			
			//void returns actually just return false
			if(data.hasValue){
				throw resumeExpression(*data.value);
			}
			else{
				throwError("somehow resumed return with no value");
			}
			//blocks and functions will reset environment even if throw
		}
		
		/**
		 * Runs a block node. A block may stall on any of its statements.
		 */
		void runBlock(const AstNode& block){
			throwIfNotType(block, AstType::stmtBlock, "not a block!");
			int currentIndex{ 0 };
			const auto& statements{
				std::get<AstStmtBlockData>(block.dataVariant).statements
			};
			//create a new environment who is a child of the old environment
			pushEmptyEnvironment();
			try{
				for(; currentIndex < statements.size(); ++currentIndex){
					runStatement(statements[currentIndex]);
				}
				//reset the environment to its parent
				popEnvironment();
			}
			catch(const StallFlag&){
				/*
				 * stalled on a native function! vomit onto the stack and rethrow, but
				 * DO NOT RESET THE ENVIRONMENT POINTER since we must resume in the innermost
				 * environment.
				 */
				pushStallNodeInfo({ AstType::stmtBlock, currentIndex });
				throw;
			}
			catch(...){
				//caught something else, reset environment and rethrow.
				popEnvironment();
				throw;
			}
		}
		
		/**
		 * Resumes a stalled block. The block may stall on the same statement, or it may stall
		 * on a new statement.
		 */
		void resumeBlock(const AstNode& block){
			/**
			 * This method DOES NOT CREATE A NEW ENVIRONMENT since we resume execution in the
			 * inner-most environment.
			 */
			throwIfNotType(block, AstType::stmtBlock, "not a block!");
			//get the stall info
			const StallNodeInfo& stallNodeInfo{ popLastStallNodeInfo() };
			throwIfNotType(stallNodeInfo, AstType::stmtBlock, "bad stall type: not block!");
			//resume running the block
			int currentIndex{ stallNodeInfo.index };
			const auto& statements{
				std::get<AstStmtBlockData>(block.dataVariant).statements
			};
			try{
				//resume the stalled statement
				resumeStatement(statements[currentIndex]);
				++currentIndex;
				//run the rest of the statements like normal
				for(; currentIndex < statements.size(); ++currentIndex){
					runStatement(statements[currentIndex]);
				}
				//reset the environment to its parent
				popEnvironment();
			}
			catch(const StallFlag&){
				/**
				 * stalled on a different native function! vomit onto the stack and rethrow,
				 * but DO NOT RESET THE ENVIRONMENT POINTER since we must resume in the
				 * inner-most environment.
				 */
				pushStallNodeInfo({ AstType::stmtBlock, currentIndex });
				throw;
			}
			catch(...){
				//caught something else, reset environment and rethrow.
				popEnvironment();
				throw;
			}
		}
		
		/**
		 * Runs an expression, which are nodes which evaluate to a value. Some expressions
		 * may stall, while others are guaranteed not to stall. This method checks the type
		 * of AstNode it is given and delegates to the appropriate run method.
		 */
		DataType runExpression(const AstNode& expression){
			switch(expression.type){
				case AstType::litBool:
					return std::get<AstLitBoolData>(expression.dataVariant).value;
				case AstType::litInt:
					return std::get<AstLitIntData>(expression.dataVariant).value;
				case AstType::litFloat:
					return std::get<AstLitFloatData>(expression.dataVariant).value;
				case AstType::litString:
					return std::get<AstLitStringData>(expression.dataVariant).value;
				case AstType::parenthesis:
					return runExpression(
						*std::get<AstParenthesisData>(expression.dataVariant).inside
					);
				case AstType::unaryBang:
					return runUnaryBang(expression);
				case AstType::unaryPlus:
					return runUnaryPlus(expression);
				case AstType::unaryMinus:
					return runUnaryMinus(expression);
				case AstType::binPlus:
					return runBinaryPlus(expression);
				case AstType::binMinus:
					return runBinaryMinus(expression);
				case AstType::binStar:
					return runBinaryStar(expression);
				case AstType::binForwardSlash:
					return runBinaryForwardSlash(expression);
				case AstType::binDualEqual:
					return runBinaryDualEqual(expression);
				case AstType::binBangEqual:
					return runBinaryBangEqual(expression);
				case AstType::binGreater:
					return runBinaryGreater(expression);
				case AstType::binGreaterEqual:
					return runBinaryGreaterEqual(expression);
				case AstType::binLess:
					return runBinaryLess(expression);
				case AstType::binLessEqual:
					return runBinaryLessEqual(expression);
				case AstType::binAmpersand:
					return runBinaryAmpersand(expression);
				case AstType::binVerticalBar:
					return runBinaryVerticalBar(expression);
				case AstType::binAssign:
					return runBinaryAssignment(expression);
				
				case AstType::variable:
					return runVariable(expression);
				case AstType::call:
					return runCall(expression);
					
				default:
					throwError("trying to run not an expression!");
					return false;//dummy return
			}
		}
		
		/**
		 * Resumes an expression. Expressions evaluate to a value. This method checks the type
		 * of AstNode it is given and delegates to the appropriate resume method. Throws an
		 * error if it is discovered that the node should have never stalled in the first
		 * place.
		 */
		DataType resumeExpression(const AstNode& expression){
			switch(expression.type){
				case AstType::litBool:
				case AstType::litInt:
				case AstType::litFloat:
				case AstType::litString:
					throwError("somehow stalled on a literal expression!");
				case AstType::parenthesis:
					return resumeExpression(
						*std::get<AstParenthesisData>(expression.dataVariant).inside
					);
				case AstType::unaryBang:
					return resumeUnaryBang(expression);
				case AstType::unaryPlus:
					return resumeUnaryPlus(expression);
				case AstType::unaryMinus:
					return resumeUnaryMinus(expression);
				case AstType::binPlus:
					return resumeBinaryPlus(expression);
				case AstType::binMinus:
					return resumeBinaryMinus(expression);
				case AstType::binStar:
					return resumeBinaryStar(expression);
				case AstType::binForwardSlash:
					return resumeBinaryForwardSlash(expression);
				case AstType::binDualEqual:
					return resumeBinaryDualEqual(expression);
				case AstType::binBangEqual:
					return resumeBinaryBangEqual(expression);
				case AstType::binGreater:
					return resumeBinaryGreater(expression);
				case AstType::binGreaterEqual:
					return resumeBinaryGreaterEqual(expression);
				case AstType::binLess:
					return resumeBinaryLess(expression);
				case AstType::binLessEqual:
					return resumeBinaryLessEqual(expression);
				case AstType::binAmpersand:
					return resumeBinaryAmpersand(expression);
				case AstType::binVerticalBar:
					return resumeBinaryVerticalBar(expression);
				case AstType::binAssign:
					return resumeBinaryAssignment(expression);
				
				case AstType::variable:
					throwError("somehow stalled on a get-variable expression!");
				case AstType::call:
					return resumeCall(expression);
				
				default:
					throwError("trying to resume not an expression!");
					return false;//dummy return
			}
		}
		
		/**
		 * Runs a unary bang node. This node will negate a boolean arg and throw if its arg
		 * is of any other built in type. Delegates to the native unary bang function if the
		 * arg is of a user-defined type. Because users must guarantee that the native unary
		 * bang function will not stall, this method will only stall on its argument. Thus,
		 * this type of node does not vomit onto the stack.
		 */
		DataType runUnaryBang(const AstNode& unary){
			throwIfNotType(unary, AstType::unaryBang, "not unary bang!");
			DataType argValue{ runExpression(
				*std::get<AstUnaryData>(unary.dataVariant).arg
			) };
			return evaluateUnaryBang(argValue);
		}
		
		/**
		 * Resumes a unary bang node. This node will negate a boolean arg and throw if its arg
		 * is of any other built in type. Delegates to the native unary bang function if the
		 * arg is of a user-defined type. Since unary operators can only stall on their
		 * argument, this type of node will not pop the stack.
		 */
		DataType resumeUnaryBang(const AstNode& unary){
			throwIfNotType(unary, AstType::unaryBang, "not unary bang!");
			DataType argValue{ resumeExpression(
				*std::get<AstUnaryData>(unary.dataVariant).arg
			) };
			return evaluateUnaryBang(argValue);
		}
		
		/**
		 * Given an input argument, runs either the built in unary bang or the native unary
		 * bang on that argument, and returns the result.
		 */
		DataType evaluateUnaryBang(const DataType& argValue){
			if(holdsAlternatives<bool>(argValue)){
				return !std::get<bool>(argValue);
			}
			if(holdsAlternatives<int, float, std::string, FunctionWrapper>(argValue)){
				throwError("bad arg for unary bang!");
			}
			//our arg is NOT a built-in type! look for a native function
			const NativeFunction nativeFunction{
				unwrapNativeFunctionFromData(
					nativeEnvironmentPointer->get(
						reservedFunctionNames::unaryBang,
						"no native unary bang function!"
					)
				)};
			return nativeFunction({ argValue });
		}
		
		/**
		 * Runs a unary plus node. This node will simply return the value of an integer or a
		 * float. Throws an error if the arg is of any other built in type. Delegates to the
		 * native unary plus function if the arg is of a user-defined type. Because users must
		 * guarantee that the native unary plus function will not stall, this method will only
		 * stall on its argument. Thus, this type of node does not vomit onto the stack.
		 */
		DataType runUnaryPlus(const AstNode& unary){
			throwIfNotType(unary, AstType::unaryPlus, "not unary plus!");
			DataType argValue{ runExpression(
				*std::get<AstUnaryData>(unary.dataVariant).arg
			) };
			return evaluateUnaryPlus(argValue);
		}
		
		/**
		 * Resumes a unary plus node. This node will simply return the value of an integer or a
		 * float. Throws an error if the arg is of any other built in type. Delegates to the
		 * native unary plus function if the arg is of a user-defined type. Since unary
		 * operators can only stall on their argument, this type of node will not pop
		 * the stack.
		 */
		DataType resumeUnaryPlus(const AstNode& unary){
			throwIfNotType(unary, AstType::unaryPlus, "not unary plus!");
			DataType argValue{ resumeExpression(
				*std::get<AstUnaryData>(unary.dataVariant).arg
			) };
			return evaluateUnaryPlus(argValue);
		}
		
		/**
		 * Given an input argument, runs either the built in unary plus or the native unary
		 * plus on that argument, and returns the result.
		 */
		DataType evaluateUnaryPlus(const DataType& argValue){
			if(holdsAlternatives<bool, std::string, FunctionWrapper>(argValue)){
				throwError("bad arg for unary plus!");
			}
			if(holdsAlternatives<int, float>(argValue)){
				return argValue;
			}
			//our arg is NOT a built-in type! look for a native function
			const NativeFunction nativeFunction{
				unwrapNativeFunctionFromData(
					nativeEnvironmentPointer->get(
						reservedFunctionNames::unaryPlus,
						"no native unary plus function!"
					)
				)};
			return nativeFunction({ argValue });
		}
		
		/**
		 * Runs a unary minus node. This node will negate an integer or a float and throw if
		 * the arg is of any other built in type. Delegates to the native unary minus function
		 * if the arg is of a user-defined type. Because users must guarantee that the native
		 * unary minus function will not stall, this method will only stall on its argument.
		 * Thus, this type of node does not vomit onto the stack.
		 */
		DataType runUnaryMinus(const AstNode& unary){
			throwIfNotType(unary, AstType::unaryMinus, "not unary minus!");
			DataType argValue{ runExpression(
				*std::get<AstUnaryData>(unary.dataVariant).arg
			) };
			return evaluateUnaryMinus(argValue);
		}
		
		/**
		 * Resumes a unary minus node. This node will negate an integer or a float and throw
		 * if the arg is of any other built in type. Delegates to the native unary plus
		 * function if the arg is of a user-defined type. Since unary operators can only stall
		 * on their argument, this type of node will not pop the stack.
		 */
		DataType resumeUnaryMinus(const AstNode& unary){
			throwIfNotType(unary, AstType::unaryMinus, "not unary minus!");
			DataType argValue{ resumeExpression(
				*std::get<AstUnaryData>(unary.dataVariant).arg
			) };
			return evaluateUnaryMinus(argValue);
		}
		
		/**
		 * Given an input argument, runs either the built in unary minus or the native unary
		 * minus on that argument, and returns the result.
		 */
		DataType evaluateUnaryMinus(const DataType& argValue){
			if(holdsAlternatives<bool, std::string, FunctionWrapper>(argValue)){
				throwError("bad arg for unary minus!");
			}
			if(holdsAlternatives<int>(argValue)){
				return -std::get<int>(argValue);
			}
			if(holdsAlternatives<float>(argValue)){
				return -std::get<float>(argValue);
			}
			//our arg is NOT a built-in type! look for a native function
			const NativeFunction nativeFunction{
				unwrapNativeFunctionFromData(
					nativeEnvironmentPointer->get(
						reservedFunctionNames::unaryMinus,
						"no native unary minus function!"
					)
				)};
			return nativeFunction({ argValue });
		}
		
		/**
		 * Runs a binary plus node. This node will add integers, floats, and concatenate
		 * strings. It will throw if the args are of any other built in type. Delegates to the
		 * native binary plus function if the arg is of a user-defined type. Because users must
		 * guarantee that the native binary plus function will not stall, this method will only
		 * stall on its left or right argument. If this method stalls on the left argument, it
		 * will vomit the left index onto the stack. However, if the method stalls on the right
		 * argument, it will also first vomit the left value onto the stack, so as to eliminate
		 * the need to reevaluate the left node upon resuming execution.
		 */
		DataType runBinaryPlus(const AstNode& binary){
			throwIfNotType(binary, AstType::binPlus, "not binary plus!");
			const auto& [leftValue, rightValue] = evaluateBinaryArgs(binary);
			return evaluateBinaryPlus(leftValue, rightValue);
		}
		
		/**
		 * Evaluates the two sides of a binary expression. Does not check for ast type
		 */
		std::pair<DataType, DataType> evaluateBinaryArgs(const AstNode& binary){
			DataType leftValue;//uninitialized
			try {
				leftValue = runExpression(*std::get<AstBinData>(binary.dataVariant).left);
			}
			catch(const StallFlag&){
				pushStallNodeInfo({ binary.type, StallNodeInfo::binLeft });
				throw;
			}
			DataType rightValue;//uninitialized
			try {
				rightValue = runExpression(*std::get<AstBinData>(binary.dataVariant).right);
			}
			catch(const StallFlag&){
				pushStallNodeData(leftValue);
				pushStallNodeInfo({ binary.type, StallNodeInfo::binRight });
				throw;
			}
			return { leftValue, rightValue };
		}
		
		/**
		 * Resumes a binary plus node. This node will add integers, floats, and concatenate
		 * strings. It will throw if the args are of any other built in type. Delegates to the
		 * native binary plus function if the args are of a user-defined type. If the stall
		 * occurred on the left argument, execution will resume as normal. However, if the stall
		 * occurred on the right argument, this method will extract the stored left argument
		 * from the stack as well.
		 */
		DataType resumeBinaryPlus(const AstNode& binary){
			throwIfNotType(binary, AstType::binPlus, "not binary plus!");
			//get the stall info
			const StallNodeInfo& stallNodeInfo{ popLastStallNodeInfo() };
			throwIfNotType(stallNodeInfo, AstType::binPlus, "bad stall type: not bin plus!");
			const auto& [leftValue, rightValue] = resumeEvaluatingBinaryArgs(
				binary,
				stallNodeInfo
			);
			return evaluateBinaryPlus(leftValue, rightValue);
		}
		
		/**
		 * Resumes evaluating the two sides of a binary expression. Handles the case of
		 * resuming the right argument needing to extract the stored left argument. Does not
		 * check for ast type.
		 */
		std::pair<DataType, DataType> resumeEvaluatingBinaryArgs(
			const AstNode& binary,
			const StallNodeInfo& stallNodeInfo
		){
			DataType leftValue;//uninitialized
			DataType rightValue;//uninitialized
			switch(stallNodeInfo.index){
				case StallNodeInfo::binLeft: {
					//the left expression may stall again
					try{
						leftValue = resumeExpression(
							*std::get<AstBinData>(binary.dataVariant).left
						);
					}
					catch(const StallFlag&){
						pushStallNodeInfo({ binary.type, StallNodeInfo::binLeft });
						throw;
					}
					//the left expression did not stall again, evaluate to right side
					try {
						rightValue = runExpression(
							*std::get<AstBinData>(binary.dataVariant).right
						);
					}
					catch(const StallFlag&){
						pushStallNodeData(leftValue);
						pushStallNodeInfo({ binary.type, StallNodeInfo::binRight });
						throw;
					}
					break;
				}
				case StallNodeInfo::binRight: {
					//must pop stack in case lower expressions also need stack
					leftValue = popLastStallData();
					//the right expression may stall again
					try{
						rightValue = resumeExpression(
							*std::get<AstBinData>(binary.dataVariant).right
						);
					}
					catch(const StallFlag&){
						pushStallNodeData(leftValue);
						pushStallNodeInfo({ binary.type, StallNodeInfo::binRight });
						throw;
					}
				}
				default:
					throwError("bad stall index for binary expr: " + stallNodeInfo.index);
			}
			return { false, false };//dummy return
		}
		
		/**
		 * Given two input arguments, runs either the built in binary plus or the native binary
		 * plus on those arguments, and returns the result.
		 */
		DataType evaluateBinaryPlus(const DataType& leftValue, const DataType& rightValue){
			auto leftIndex{ leftValue.index() };
			auto rightIndex{ rightValue.index() };
			if(leftIndex < numBuiltInTypes && rightIndex < numBuiltInTypes){
				switch(leftIndex){
					case intIndex: {
						int leftInt{ std::get<int>(leftValue) };
						switch( rightIndex ) {
							case intIndex: {
								int rightInt { std::get<int>(rightValue) };
								return leftInt + rightInt;
							}
							case floatIndex: {
								float rightFloat { std::get<float>(rightValue) };
								return static_cast<float>(leftInt) + rightFloat;
							}
							case stringIndex: {
								const auto& rightString{ std::get<std::string>(rightValue) };
								return std::to_string(leftInt) + rightString;
							}
						}
					}
					case floatIndex: {
						float leftFloat{ std::get<float>(leftValue) };
						switch( rightIndex ) {
							case intIndex: {
								int rightInt { std::get<int>(rightValue) };
								return leftFloat + static_cast<float>(rightInt);
							}
							case floatIndex: {
								float rightFloat { std::get<float>(rightValue) };
								return leftFloat + rightFloat;
							}
							case stringIndex: {
								const auto& rightString{ std::get<std::string>(rightValue) };
								return std::to_string(leftFloat) + rightString;
							}
						}
					}
					case stringIndex: {
						const auto& leftString{ std::get<std::string>(leftValue) };
						switch( rightIndex ) {
							case intIndex: {
								int rightInt { std::get<int>(rightValue) };
								return leftString + std::to_string(rightInt);
							}
							case floatIndex: {
								float rightFloat { std::get<float>(rightValue) };
								return leftString + std::to_string(rightFloat);
							}
							case stringIndex: {
								const auto& rightString{ std::get<std::string>(rightValue) };
								return leftString + rightString;
							}
						}
					}
				}
				if(leftIndex == boolIndex || rightIndex == boolIndex){
					throwError("trying to add a bool!");
				}
				if(leftIndex == functionIndex || rightIndex == functionIndex){
					throwError("trying to add a function!");
				}
			}
			//one of our args is NOT a built-in type! look for a native function
			const NativeFunction nativeFunction{
				unwrapNativeFunctionFromData(
					nativeEnvironmentPointer->get(
						reservedFunctionNames::binaryPlus,
						"no native binary plus function!"
					)
				)};
			return nativeFunction({ leftValue, rightValue });
		}
		
		/**
		 * Runs a binary minus node. This node will subtract integers and floats. It will throw
		 * if the args are of any other built in type. Delegates to the native binary minus
		 * function if the args are of a user-defined type. Because users must guarantee that
		 * the native binary minus function will not stall, this method will only stall on its
		 * left or right argument. If this method stalls on the left argument, it will vomit the
		 * left index onto the stack. However, if the method stalls on the right argument, it
		 * will also first vomit the left value onto the stack, so as to eliminate the need to
		 * reevaluate the left node upon resuming execution.
		 */
		DataType runBinaryMinus(const AstNode& binary){
			throwIfNotType(binary, AstType::binMinus, "not binary minus!");
			const auto& [leftValue, rightValue] = evaluateBinaryArgs(binary);
			return evaluateBinaryMinus(leftValue, rightValue);
		}
		
		/**
		 * Resumes a binary minus node. This node will subtract integers and floats. It will
		 * throw if the args are of any other built in type. Delegates to the native binary
		 * minus function if the arg is of a user-defined type. If the stall occurred on the
		 * left argument, execution will resume as normal. However, if the stall occurred on
		 * the right argument, this method will extract the stored left argument from the stack
		 * as well.
		 */
		DataType resumeBinaryMinus(const AstNode& binary){
			throwIfNotType(binary, AstType::binMinus, "not binary minus!");
			//get the stall info
			const StallNodeInfo& stallNodeInfo{ popLastStallNodeInfo() };
			throwIfNotType(stallNodeInfo, AstType::binMinus, "bad stall type: not bin minus!");
			const auto& [leftValue, rightValue] = resumeEvaluatingBinaryArgs(
				binary,
				stallNodeInfo
			);
			return evaluateBinaryMinus(leftValue, rightValue);
		}
		
		/**
		 * Given two input arguments, runs either the built in binary minus or the native
		 * binary minus on those arguments, and returns the result.
		 */
		DataType evaluateBinaryMinus(const DataType& leftValue, const DataType& rightValue){
			auto leftIndex{ leftValue.index() };
			auto rightIndex{ rightValue.index() };
			if(leftIndex < numBuiltInTypes && rightIndex < numBuiltInTypes){
				switch(leftIndex){
					case intIndex: {
						int leftInt{ std::get<int>(leftValue) };
						switch( rightIndex ) {
							case intIndex: {
								int rightInt { std::get<int>(rightValue) };
								return leftInt - rightInt;
							}
							case floatIndex: {
								float rightFloat { std::get<float>(rightValue) };
								return static_cast<float>(leftInt) - rightFloat;
							}
						}
					}
					case floatIndex: {
						float leftFloat{ std::get<float>(leftValue) };
						switch( rightIndex ) {
							case intIndex: {
								int rightInt { std::get<int>(rightValue) };
								return leftFloat - static_cast<float>(rightInt);
							}
							case floatIndex: {
								float rightFloat { std::get<float>(rightValue) };
								return leftFloat - rightFloat;
							}
						}
					}
				}
				if(leftIndex == boolIndex || rightIndex == boolIndex){
					throwError("trying to minus a bool!");
				}
				if(leftIndex == functionIndex || rightIndex == functionIndex){
					throwError("trying to minus a function!");
				}
				if(leftIndex == stringIndex || rightIndex == stringIndex){
					throwError("trying to minus a string!");
				}
			}
			//one of our args is NOT a built-in type! look for a native function
			const NativeFunction nativeFunction{
				unwrapNativeFunctionFromData(
					nativeEnvironmentPointer->get(
						reservedFunctionNames::binaryMinus,
						"no native binary minus function!"
					)
				)};
			return nativeFunction({ leftValue, rightValue });
		}
		
		/**
		 * Runs a binary star node. This node will multiply integers and floats. It will throw
		 * if the args are of any other built in type. Delegates to the native binary star
		 * function if the arg is of a user-defined type. Because users must guarantee that the
		 * native binary star function will not stall, this method will only stall on its left
		 * or right argument. If this method stalls on the left argument, it will vomit the
		 * left index onto the stack. However, if the method stalls on the right argument, it
		 * will also first vomit the left value onto the stack, so as to eliminate the need to
		 * reevaluate the left node upon resuming execution.
		 */
		DataType runBinaryStar(const AstNode& binary){
			throwIfNotType(binary, AstType::binStar, "not binary star!");
			const auto& [leftValue, rightValue] = evaluateBinaryArgs(binary);
			return evaluateBinaryStar(leftValue, rightValue);
		}
		
		/**
		 * Resumes a binary star node. This node will multiply integers and floats. It will
		 * throw if the args are of any other built in type. Delegates to the native binary
		 * star function if the arg is of a user-defined type. If the stall occurred on the
		 * left argument, execution will resume as normal. However, if the stall occurred on
		 * the right argument, this method will extract the stored left argument from the stack
		 * as well.
		 */
		DataType resumeBinaryStar(const AstNode& binary){
			throwIfNotType(binary, AstType::binStar, "not binary star!");
			//get the stall info
			const StallNodeInfo& stallNodeInfo{ popLastStallNodeInfo() };
			throwIfNotType(stallNodeInfo, AstType::binStar, "bad stall type: not bin star!");
			const auto& [leftValue, rightValue] = resumeEvaluatingBinaryArgs(
				binary,
				stallNodeInfo
			);
			return evaluateBinaryStar(leftValue, rightValue);
		}
		
		/**
		 * Given two input arguments, runs either the built in binary star or the native binary
		 * star on those arguments, and returns the result.
		 */
		DataType evaluateBinaryStar(const DataType& leftValue, const DataType& rightValue){
			auto leftIndex{ leftValue.index() };
			auto rightIndex{ rightValue.index() };
			if(leftIndex < numBuiltInTypes && rightIndex < numBuiltInTypes){
				switch(leftIndex){
					case intIndex: {
						int leftInt{ std::get<int>(leftValue) };
						switch( rightIndex ) {
							case intIndex: {
								int rightInt { std::get<int>(rightValue) };
								return leftInt * rightInt;
							}
							case floatIndex: {
								float rightFloat { std::get<float>(rightValue) };
								return static_cast<float>(leftInt) * rightFloat;
							}
						}
					}
					case floatIndex: {
						float leftFloat{ std::get<float>(leftValue) };
						switch( rightIndex ) {
							case intIndex: {
								int rightInt { std::get<int>(rightValue) };
								return leftFloat * static_cast<float>(rightInt);
							}
							case floatIndex: {
								float rightFloat { std::get<float>(rightValue) };
								return leftFloat * rightFloat;
							}
						}
					}
				}
				if(leftIndex == boolIndex || rightIndex == boolIndex){
					throwError("trying to star a bool!");
				}
				if(leftIndex == functionIndex || rightIndex == functionIndex){
					throwError("trying to star a function!");
				}
				if(leftIndex == stringIndex || rightIndex == stringIndex){
					throwError("trying to star a string!");
				}
			}
			//one of our args is NOT a built-in type! look for a native function
			const NativeFunction nativeFunction{
				unwrapNativeFunctionFromData(
					nativeEnvironmentPointer->get(
						reservedFunctionNames::binaryStar,
						"no native binary star function!"
					)
				)};
			return nativeFunction({ leftValue, rightValue });
		}
		
		/**
		 * Runs a binary forward slash node. This node will divide integers and floats. It will
		 * throw if the args are of any other built in type. Delegates to the native binary
		 * forward slash function if the arg is of a user-defined type. Because users must
		 * guarantee that the native binary forward slash function will not stall, this method
		 * will only stall on its left or right argument. If this method stalls on the left
		 * argument, it will vomit the left index onto the stack. However, if the method stall
		 * on the right argument, it will also first vomit the left value onto the stack, so
		 * as to eliminate the need to reevaluate the left node upon resuming execution.
		 */
		DataType runBinaryForwardSlash(const AstNode& binary){
			throwIfNotType(binary, AstType::binForwardSlash, "not binary forward slash!");
			const auto& [leftValue, rightValue] = evaluateBinaryArgs(binary);
			return evaluateBinaryForwardSlash(leftValue, rightValue);
		}
		
		/**
		 * Resumes a binary forward slash node. This node will divide integers and floats. It
		 * will throw if the args are of any other built in type. Delegates to the native
		 * binary forward slash function if the arg is of a user-defined type. If the stall
		 * occurred on the left argument, execution will resume as normal. However, if the
		 * stall occurred on the right argument, this method will extract the stored left
		 * argument from the stack as well.
		 */
		DataType resumeBinaryForwardSlash(const AstNode& binary){
			throwIfNotType(binary, AstType::binForwardSlash, "not binary forward slash!");
			//get the stall info
			const StallNodeInfo& stallNodeInfo{ popLastStallNodeInfo() };
			throwIfNotType(
				stallNodeInfo,
				AstType::binForwardSlash,
				"bad stall type: not bin forward slash!"
			);
			const auto& [leftValue, rightValue] = resumeEvaluatingBinaryArgs(
				binary,
				stallNodeInfo
			);
			return evaluateBinaryForwardSlash(leftValue, rightValue);
		}
		
		/**
		 * Given two input arguments, runs either the built in binary forward slash or the
		 * native binary forward slash on those arguments, and returns the result.
		 */
		DataType evaluateBinaryForwardSlash(
			const DataType& leftValue,
			const DataType& rightValue
		){
			auto leftIndex{ leftValue.index() };
			auto rightIndex{ rightValue.index() };
			if(leftIndex < numBuiltInTypes && rightIndex < numBuiltInTypes){
				switch(leftIndex){
					case intIndex: {
						int leftInt{ std::get<int>(leftValue) };
						switch( rightIndex ) {
							case intIndex: {
								int rightInt { std::get<int>(rightValue) };
								return leftInt / rightInt;
							}
							case floatIndex: {
								float rightFloat { std::get<float>(rightValue) };
								return static_cast<float>(leftInt) / rightFloat;
							}
						}
					}
					case floatIndex: {
						float leftFloat{ std::get<float>(leftValue) };
						switch( rightIndex ) {
							case intIndex: {
								int rightInt { std::get<int>(rightValue) };
								return leftFloat / static_cast<float>(rightInt);
							}
							case floatIndex: {
								float rightFloat { std::get<float>(rightValue) };
								return leftFloat / rightFloat;
							}
						}
					}
				}
				if(leftIndex == boolIndex || rightIndex == boolIndex){
					throwError("trying to forward slash a bool!");
				}
				if(leftIndex == functionIndex || rightIndex == functionIndex){
					throwError("trying to forward slash a function!");
				}
				if(leftIndex == stringIndex || rightIndex == stringIndex){
					throwError("trying to forward slash a string!");
				}
			}
			//one of our args is NOT a built-in type! look for a native function
			const NativeFunction nativeFunction{
				unwrapNativeFunctionFromData(
					nativeEnvironmentPointer->get(
						reservedFunctionNames::binaryForwardSlash,
						"no native binary forward slash function!"
					)
				)};
			return nativeFunction({ leftValue, rightValue });
		}
		
		/**
		 * Runs a binary dual equal node, which checks for equality. This method will throw if
		 * given a function argument. Delegates to the native dual equal function if the args
		 * are of a user-defined type. Because users must guarantee that the native dual equal
		 * function will not stall, this method will only stall on its left or right argument.
		 * If this method stalls on the left argument, it will vomit the left index onto the
		 * stack. However, if the method stalls on the right argument, it  will also first
		 * vomit the left value onto the stack, so as to eliminate the need to reevaluate the
		 * left node upon resuming execution.
		 */
		DataType runBinaryDualEqual(const AstNode& binary){
			throwIfNotType(binary, AstType::binDualEqual, "not binary dual equal!");
			const auto& [leftValue, rightValue] = evaluateBinaryArgs(binary);
			return evaluateBinaryDualEqual(leftValue, rightValue);
		}
		
		/**
		 * Resumes a binary dual equal node, which checks for equality. This method will throw
		 * if given a function argument. Delegates to the native dual equal function if the
		 * args are of a user-defined type. If the stall occurred on the left argument,
		 * execution will resume as normal. However, if the stall occurred on the right
		 * argument, this method will extract the stored left argument from the stack as well.
		 */
		DataType resumeBinaryDualEqual(const AstNode& binary){
			throwIfNotType(binary, AstType::binDualEqual, "not binary dual equal!");
			//get the stall info
			const StallNodeInfo& stallNodeInfo{ popLastStallNodeInfo() };
			throwIfNotType(
				stallNodeInfo,
				AstType::binDualEqual,
				"bad stall type: not bin dual equal!"
			);
			const auto& [leftValue, rightValue] = resumeEvaluatingBinaryArgs(
				binary,
				stallNodeInfo
			);
			return evaluateBinaryDualEqual(leftValue, rightValue);
		}
		
		/**
		 * Runs a binary bang equal node, which checks for inequality. This method will throw
		 * if given a function argument. Uses the native dual equal function if the args are
		 * of a user-defined type. Because users must guarantee that the native dual equal
		 * function will not stall, this method will only stall on its left or right argument.
		 * If this method stalls on the left argument, it will vomit the left index onto the
		 * stack. However, if the method stalls on the right argument, it  will also first
		 * vomit the left value onto the stack, so as to eliminate the need to reevaluate the
		 * left node upon resuming execution.
		 */
		DataType runBinaryBangEqual(const AstNode& binary){
			throwIfNotType(binary, AstType::binBangEqual, "not binary bang equal!");
			const auto& [leftValue, rightValue] = evaluateBinaryArgs(binary);
			return !evaluateBinaryDualEqual(leftValue, rightValue);
		}
		
		/**
		 * Resumes a binary bang equal node, which checks for inequality. This method will
		 * throw  if given a function argument. Uses the native dual equal function if the
		 * args are of a user-defined type. If the stall occurred on the left argument,
		 * execution will resume as normal. However, if the stall occurred on the right
		 * argument, this method will extract the stored left argument from the stack as well.
		 */
		DataType resumeBinaryBangEqual(const AstNode& binary){
			throwIfNotType(binary, AstType::binBangEqual, "not binary bang equal!");
			//get the stall info
			const StallNodeInfo& stallNodeInfo{ popLastStallNodeInfo() };
			throwIfNotType(
				stallNodeInfo,
				AstType::binBangEqual,
				"bad stall type: not bin bang equal!"
			);
			const auto& [leftValue, rightValue] = resumeEvaluatingBinaryArgs(
				binary,
				stallNodeInfo
			);
			return !evaluateBinaryDualEqual(leftValue, rightValue);
		}
		
		/**
		 * Given two input arguments, runs either the built in binary dual equal or the native
		 * binary dual equal on those arguments, and returns the result.
		 */
		bool evaluateBinaryDualEqual(const DataType& leftValue, const DataType& rightValue){
			auto leftIndex{ leftValue.index() };
			auto rightIndex{ rightValue.index() };
			if(leftIndex < numBuiltInTypes && rightIndex < numBuiltInTypes){
				switch(leftIndex){
					case intIndex: {
						int leftInt{ std::get<int>(leftValue) };
						switch( rightIndex ) {
							case intIndex: {
								int rightInt { std::get<int>(rightValue) };
								return leftInt == rightInt;
							}
							case floatIndex: {
								float rightFloat { std::get<float>(rightValue) };
								return static_cast<float>(leftInt) == rightFloat;
							}
							case stringIndex: {
								const auto& rightString{ std::get<std::string>(rightValue) };
								return std::to_string(leftInt) == rightString;
							}
							case functionIndex:
								throwError("trying to dual equal int vs func!");
						}
					}
					case floatIndex: {
						float leftFloat{ std::get<float>(leftValue) };
						switch( rightIndex ) {
							case intIndex: {
								int rightInt { std::get<int>(rightValue) };
								return leftFloat == static_cast<float>(rightInt);
							}
							case floatIndex: {
								float rightFloat { std::get<float>(rightValue) };
								return leftFloat == rightFloat;
							}
							case stringIndex: {
								const auto& rightString{ std::get<std::string>(rightValue) };
								return std::to_string(leftFloat) == rightString;
							}
							case functionIndex:
								throwError("trying to dual equal float vs func!");
						}
					}
					case stringIndex: {
						const auto& leftString{ std::get<std::string>(leftValue) };
						switch( rightIndex ) {
							case intIndex: {
								int rightInt { std::get<int>(rightValue) };
								return leftString == std::to_string(rightInt);
							}
							case floatIndex: {
								float rightFloat { std::get<float>(rightValue) };
								return leftString == std::to_string(rightFloat);
							}
							case stringIndex: {
								const auto& rightString{ std::get<std::string>(rightValue) };
								return leftString == rightString;
							}
							case functionIndex:
								throwError("trying to dual equal string vs func!");
						}
					}
					case functionIndex: {
						throwError("trying to dual equal a func!");
					}
				}
				if(leftIndex == boolIndex){
					if(rightIndex == boolIndex){
						return std::get<bool>(leftValue) == std::get<bool>(rightValue);
					}
					else{
						throwError("trying to use == with only 1 bool!");
					}
				}
				if(rightIndex == boolIndex){
					//we already know the left isn't a bool, so throw
					throwError("trying to use == with only 1 bool!");
				}
			}
			//one of our args is NOT a built-in type! look for a native function
			const NativeFunction nativeFunction{
				unwrapNativeFunctionFromData(
					nativeEnvironmentPointer->get(
						reservedFunctionNames::binaryDualEqual,
						"no native binary dual equal function!"
					)
				)};
			const DataType& nativeFunctionResult{ nativeFunction({ leftValue, rightValue }) };
			if(std::holds_alternative<bool>(nativeFunctionResult)){
				return std::get<bool>(nativeFunctionResult);
			}
			else{
				throwError("bad type from native dual equal function!");
				return false;//dummy return
			}
		}
		
		/**
		 * Runs a binary greater node, which compares ints, floats, and strings. This method
		 * will throw if given any other built in type. Delegates to the native binary greater
		 * function if the args are of a user-defined type. Because users must guarantee that
		 * the native binary greater function will not stall, this method will only stall on
		 * its left or right argument. If this method stalls on the left argument, it will
		 * vomit the left index onto the stack. However, if the method stalls on the right
		 * argument, it will also first vomit the left value onto the stack, so as to
		 * eliminate the need to reevaluate the left node upon resuming execution.
		 */
		DataType runBinaryGreater(const AstNode& binary){
			throwIfNotType(binary, AstType::binGreater, "not binary greater!");
			const auto& [leftValue, rightValue] = evaluateBinaryArgs(binary);
			return evaluateBinaryGreater(leftValue, rightValue);
		}
		
		/**
		 * Resumes a binary greater node, which compares ints, floats, and strings. This method
		 * will throw if given any other built in type. Delegates to the native binary greater
		 * function if the args are of a user-defined type. If the stall occurred on the left
		 * argument, execution will resume as normal. However, if the stall occurred on the
		 * right argument, this method will extract the stored left argument from the stack
		 * as well.
		 */
		DataType resumeBinaryGreater(const AstNode& binary){
			throwIfNotType(binary, AstType::binGreater, "not binary greater!");
			//get the stall info
			const StallNodeInfo& stallNodeInfo{ popLastStallNodeInfo() };
			throwIfNotType(
				stallNodeInfo,
				AstType::binGreater,
				"bad stall type: not bin greater!"
			);
			const auto& [leftValue, rightValue] = resumeEvaluatingBinaryArgs(
				binary,
				stallNodeInfo
			);
			return evaluateBinaryGreater(leftValue, rightValue);
		}
		
		/**
		 * Runs a binary greater equal node, which compares ints, floats, and strings. This
		 * method will throw if given any other built in type. Uses the native binary greater
		 * function if the args are of a user-defined type. Because users must guarantee that
		 * the native binary greater function will not stall, this method will only stall on
		 * its left or right argument. If this method stalls on the left argument, it will
		 * vomit the left index onto the stack. However, if the method stalls on the right
		 * argument, it will also first vomit the left value onto the stack, so as to
		 * eliminate the need to reevaluate the left node upon resuming execution.
		 */
		DataType runBinaryGreaterEqual(const AstNode& binary){
			throwIfNotType(binary, AstType::binGreaterEqual, "not binary greater equal!");
			const auto& [leftValue, rightValue] = evaluateBinaryArgs(binary);
			//switch left and right and also negate
			return !evaluateBinaryGreater(rightValue, leftValue);
		}
		
		/**
		 * Resumes a binary greater equal node, which compares ints, floats, and strings. This
		 * method will throw if given any other built in type. Uses the native binary greater
		 * function if the args are of a user-defined type. If the stall occurred on the left
		 * argument, execution will resume as normal. However, if the stall occurred on the
		 * right argument, this method will extract the stored left argument from the stack
		 * as well.
		 */
		DataType resumeBinaryGreaterEqual(const AstNode& binary){
			throwIfNotType(binary, AstType::binGreaterEqual, "not binary greater equal!");
			//get the stall info
			const StallNodeInfo& stallNodeInfo{ popLastStallNodeInfo() };
			throwIfNotType(
				stallNodeInfo,
				AstType::binGreaterEqual,
				"bad stall type: not bin greater equal!"
			);
			const auto& [leftValue, rightValue] = resumeEvaluatingBinaryArgs(
				binary,
				stallNodeInfo
			);
			//switch left and right and also negate
			return !evaluateBinaryGreater(rightValue, leftValue);
		}
		
		/**
		 * Runs a binary less node, which compares ints, floats, and strings. This method will
		 * throw if given any other built in type. Uses the native binary greater function if
		 * the args are of a user-defined type. Because users must guarantee that the native
		 * binary greater function will not stall, this method will only stall on its left or
		 * right argument. If this method stalls on the left argument, it will vomit the left
		 * index onto the stack. However, if the method stalls on the right argument, it will
		 * also first vomit the left value onto the stack, so as to eliminate the need to
		 * reevaluate the left node upon resuming execution.
		 */
		DataType runBinaryLess(const AstNode& binary){
			throwIfNotType(binary, AstType::binLess, "not binary less!");
			const auto& [leftValue, rightValue] = evaluateBinaryArgs(binary);
			//switch left and right
			return evaluateBinaryGreater(rightValue, leftValue);
		}
		
		/**
		 * Resumes a binary less node, which compares ints, floats, and strings. This method
		 * will throw if given any other built in type. Uses the native binary greater function
		 * if the args are of a user-defined type. If the stall occurred on the left argument,
		 * execution will resume as normal. However, if the stall occurred on the right
		 * argument, this method will extract the stored left argument from the stack as well.
		 */
		DataType resumeBinaryLess(const AstNode& binary){
			throwIfNotType(binary, AstType::binLess, "not binary less!");
			//get the stall info
			const StallNodeInfo& stallNodeInfo{ popLastStallNodeInfo() };
			throwIfNotType(stallNodeInfo, AstType::binLess, "bad stall type: not bin less!");
			const auto& [leftValue, rightValue] = resumeEvaluatingBinaryArgs(
				binary,
				stallNodeInfo
			);
			//switch left and right
			return evaluateBinaryGreater(rightValue, leftValue);
		}
		
		/**
		 * Runs a binary less equal node, which compares ints, floats, and strings. This
		 * method will throw if given any other built in type. Uses the native binary greater
		 * function if the args are of a user-defined type. Because users must guarantee that
		 * the native binary greater function will not stall, this method will only stall on
		 * its left or right argument. If this method stalls on the left argument, it will
		 * vomit the left index onto the stack. However, if the method stalls on the right
		 * argument, it will also first vomit the left value onto the stack, so as to
		 * eliminate the need to reevaluate the left node upon resuming execution.
		 */
		DataType runBinaryLessEqual(const AstNode& binary){
			throwIfNotType(binary, AstType::binLessEqual, "not binary less equal!");
			const auto& [leftValue, rightValue] = evaluateBinaryArgs(binary);
			//negate
			return !evaluateBinaryGreater(leftValue, rightValue);
		}
		
		/**
		 * Resumes a binary less equal node, which compares ints, floats, and strings. This
		 * method will throw if given any other built in type. Uses the native binary greater
		 * function if the args are of a user-defined type. If the stall occurred on the left
		 * argument, execution will resume as normal. However, if the stall occurred on the
		 * right argument, this method will extract the stored left argument from the stack
		 * as well.
		 */
		DataType resumeBinaryLessEqual(const AstNode& binary){
			throwIfNotType(binary, AstType::binLessEqual, "not binary less equal!");
			//get the stall info
			const StallNodeInfo& stallNodeInfo{ popLastStallNodeInfo() };
			throwIfNotType(
				stallNodeInfo,
				AstType::binLessEqual,
				"bad stall type: not bin less equal!"
			);
			const auto& [leftValue, rightValue] = resumeEvaluatingBinaryArgs(
				binary,
				stallNodeInfo
			);
			//negate
			return !evaluateBinaryGreater(leftValue, rightValue);
		}
		
		/**
		 * Given two input arguments, runs either the built in binary greater or the native
		 * binary greater on those arguments, and returns the result.
		 */
		bool evaluateBinaryGreater(const DataType& leftValue, const DataType& rightValue){
			auto leftIndex{ leftValue.index() };
			auto rightIndex{ rightValue.index() };
			if(leftIndex < numBuiltInTypes && rightIndex < numBuiltInTypes){
				switch(leftIndex){
					case intIndex: {
						int leftInt{ std::get<int>(leftValue) };
						switch( rightIndex ) {
							case intIndex: {
								int rightInt { std::get<int>(rightValue) };
								return leftInt > rightInt;
							}
							case floatIndex: {
								float rightFloat { std::get<float>(rightValue) };
								return static_cast<float>(leftInt) > rightFloat;
							}
							case stringIndex: {
								const auto& rightString{ std::get<std::string>(rightValue) };
								return std::to_string(leftInt) > rightString;
							}
						}
					}
					case floatIndex: {
						float leftFloat{ std::get<float>(leftValue) };
						switch( rightIndex ) {
							case intIndex: {
								int rightInt { std::get<int>(rightValue) };
								return leftFloat > static_cast<float>(rightInt);
							}
							case floatIndex: {
								float rightFloat { std::get<float>(rightValue) };
								return leftFloat > rightFloat;
							}
							case stringIndex: {
								const auto& rightString{ std::get<std::string>(rightValue) };
								return std::to_string(leftFloat) > rightString;
							}
						}
					}
					case stringIndex: {
						const auto& leftString{ std::get<std::string>(leftValue) };
						switch( rightIndex ) {
							case intIndex: {
								int rightInt { std::get<int>(rightValue) };
								return leftString > std::to_string(rightInt);
							}
							case floatIndex: {
								float rightFloat { std::get<float>(rightValue) };
								return leftString > std::to_string(rightFloat);
							}
							case stringIndex: {
								const auto& rightString{ std::get<std::string>(rightValue) };
								return leftString > rightString;
							}
						}
					}
				}
				if(leftIndex == boolIndex || rightIndex == boolIndex){
					throwError("trying to compare a bool!");
				}
				if(leftIndex == functionIndex || rightIndex == functionIndex){
					throwError("trying to compare a func!");
				}
			}
			//one of our args is NOT a built-in type! look for a native function
			const NativeFunction nativeFunction{
				unwrapNativeFunctionFromData(
					nativeEnvironmentPointer->get(
						reservedFunctionNames::binaryGreater,
						"no native binary greater function!"
					)
				)};
			const DataType& nativeFunctionResult{ nativeFunction({ leftValue, rightValue }) };
			if(std::holds_alternative<bool>(nativeFunctionResult)){
				return std::get<bool>(nativeFunctionResult);
			}
			else{
				throwError("bad type from native greater function!");
				return false;//dummy return
			}
		}
		
		/**
		 * Runs a binary ampersand node, which runs a short-circuiting AND operation. This
		 * method will throw if given anything but booleans. A stall may occur on either the
		 * left or right side. However, because of the short circuiting behavior, there is
		 * no need to store the result of the left side if the right side stalls.
		 */
		DataType runBinaryAmpersand(const AstNode& binary){
			throwIfNotType(binary, AstType::binAmpersand, "not binary ampersand!");
			DataType leftValue;//uninitialized
			try {
				leftValue = runExpression(*std::get<AstBinData>(binary.dataVariant).left);
			}
			catch(const StallFlag&){
				pushStallNodeInfo({ binary.type, StallNodeInfo::binLeft });
				throw;
			}
			if(leftValue.index() != boolIndex){
				throwError("left side of '&' not bool!");
			}
			if(!std::get<bool>(leftValue)){
				//if the left side was false, short circuit and return false
				return false;
			}
			DataType rightValue;//uninitialized
			try {
				rightValue = runExpression(*std::get<AstBinData>(binary.dataVariant).right);
			}
			catch(const StallFlag&){
				pushStallNodeInfo({ binary.type, StallNodeInfo::binRight });
				throw;
			}
			if(rightValue.index() != boolIndex){
				throwError("right side of '&' not bool!");
			}
			//when the left side is true, the entire expression evaluates to the right side
			return std::get<bool>(rightValue);
		}
		
		/**
		 * Resumes a binary ampersand node, which runs a short-circuiting AND operation. This
		 * method will throw if given anything but booleans. The stall may have occurred on
		 * either the left or right side. However, because of the short circuiting behavior,
		 * there is no need to retrieve the result of the left side if the right side stalled.
		 */
		DataType resumeBinaryAmpersand(const AstNode& binary){
			throwIfNotType(binary, AstType::binAmpersand, "not binary ampersand!");
			//get the stall info
			const StallNodeInfo& stallNodeInfo{ popLastStallNodeInfo() };
			throwIfNotType(
				stallNodeInfo,
				AstType::binAmpersand,
				"bad stall type: not bin ampersand!"
			);
			switch(stallNodeInfo.index){
				case StallNodeInfo::binLeft: {
					//the left expression may stall again
					DataType leftValue;//uninitialized
					try{
						leftValue = resumeExpression(
							*std::get<AstBinData>(binary.dataVariant).left
						);
					}
					catch(const StallFlag&){
						pushStallNodeInfo({ binary.type, StallNodeInfo::binLeft });
						throw;
					}
					//left side did not stall again -> execute remainder as normal
					if(leftValue.index() != boolIndex){
						throwError("left side of '&' not bool!");
					}
					if(!std::get<bool>(leftValue)){
						//if the left side was false, short circuit and return false
						return false;
					}
					DataType rightValue;//uninitialized
					try {
						rightValue = runExpression(
							*std::get<AstBinData>(binary.dataVariant).right
						);
					}
					catch(const StallFlag&){
						pushStallNodeInfo({ binary.type, StallNodeInfo::binRight });
						throw;
					}
					if(rightValue.index() != boolIndex){
						throwError("right side of '&' not bool!");
					}
					//the entire expression evaluates to the right side
					return std::get<bool>(rightValue);
				}
				case StallNodeInfo::binRight:{
					//if we stalled on the right side, left must be false
					DataType rightValue;//uninitialized
					try {
						rightValue = resumeExpression(
							*std::get<AstBinData>(binary.dataVariant).right
						);
					}
					catch(const StallFlag&){
						pushStallNodeInfo({ binary.type, StallNodeInfo::binRight });
						throw;
					}
					if(rightValue.index() != boolIndex){
						throwError("right side of '&' not bool!");
					}
					//the entire expression evaluates to the right side
					return std::get<bool>(rightValue);
				}
				default:
					throwError("bad stall index for binary '&' : " + stallNodeInfo.index);
					return false;//dummy return
			}
		}
		
		/**
		 * Runs a binary vertical bar node, which runs a short-circuiting OR operation. This
		 * method will throw if given anything but booleans. A stall may occur on either the
		 * left or right side. However, because of the short circuiting behavior, there is
		 * no need to store the result of the left side if the right side stalls.
		 */
		DataType runBinaryVerticalBar(const AstNode& binary){
			throwIfNotType(binary, AstType::binVerticalBar, "not binary vertical bar!");
			DataType leftValue;//uninitialized
			try {
				leftValue = runExpression(*std::get<AstBinData>(binary.dataVariant).left);
			}
			catch(const StallFlag&){
				pushStallNodeInfo({ binary.type, StallNodeInfo::binLeft });
				throw;
			}
			if(leftValue.index() != boolIndex){
				throwError("left side of '|' not bool!");
			}
			if(std::get<bool>(leftValue)){
				//if the left side was true, short circuit and return true
				return true;
			}
			DataType rightValue;//uninitialized
			try {
				rightValue = runExpression(*std::get<AstBinData>(binary.dataVariant).right);
			}
			catch(const StallFlag&){
				pushStallNodeInfo({ binary.type, StallNodeInfo::binRight });
				throw;
			}
			if(rightValue.index() != boolIndex){
				throwError("right side of '|' not bool!");
			}
			//when the left side is false, the entire expression evaluates to the right side
			return std::get<bool>(rightValue);
		}
		
		/**
		 * Resumes a binary vertical bar node, which runs a short-circuiting OR operation. This
		 * method will throw if given anything but booleans. The stall may have occurred on
		 * either the left or right side. However, because of the short circuiting behavior,
		 * there is no need to retrieve the result of the left side if the right side stalled.
		 */
		DataType resumeBinaryVerticalBar(const AstNode& binary){
			throwIfNotType(binary, AstType::binVerticalBar, "not binary vertical bar!");
			//get the stall info
			const StallNodeInfo& stallNodeInfo{ popLastStallNodeInfo() };
			throwIfNotType(
				stallNodeInfo,
				AstType::binVerticalBar,
				"bad stall type: not bin vertical bar!"
			);
			switch(stallNodeInfo.index){
				case StallNodeInfo::binLeft: {
					DataType leftValue;//uninitialized
					try{
						leftValue = resumeExpression(
							*std::get<AstBinData>(binary.dataVariant).left
						);
					}
					catch(const StallFlag&){
						pushStallNodeInfo({ binary.type, StallNodeInfo::binLeft });
						throw;
					}
					//left side did not stall again -> execute remainder as normal
					if(leftValue.index() != boolIndex){
						throwError("left side of '|' not bool!");
					}
					if(std::get<bool>(leftValue)){
						//if the left side was true, short circuit and return true
						return true;
					}
					DataType rightValue;//uninitialized
					try {
						rightValue = runExpression(
							*std::get<AstBinData>(binary.dataVariant).right
						);
					}
					catch(const StallFlag&){
						pushStallNodeInfo({ binary.type, StallNodeInfo::binRight });
						throw;
					}
					if(rightValue.index() != boolIndex){
						throwError("right side of '|' not bool!");
					}
					//the entire expression evaluates to the right side
					return std::get<bool>(rightValue);
				}
				case StallNodeInfo::binRight:{
					//if we stalled on the right side, left must be false
					DataType rightValue;//uninitialized
					try {
						rightValue = resumeExpression(
							*std::get<AstBinData>(binary.dataVariant).right
						);
					}
					catch(const StallFlag&){
						pushStallNodeInfo({ binary.type, StallNodeInfo::binRight });
						throw;
					}
					if(rightValue.index() != boolIndex){
						throwError("right side of '|' not bool!");
					}
					//the entire expression evaluates to the right side
					return std::get<bool>(rightValue);
				}
				default:
					throwError("bad stall index for binary '|' : " + stallNodeInfo.index);
					return false;//dummy return
			}
		}
		
		/**
		 * Runs an assignment node, which binds the right side to the variable on the left.
		 * Since this method can only stall on evaluating the right side, it will not vomit
		 * onto the stack.
		 */
		DataType runBinaryAssignment(const AstNode& assign){
			throwIfNotType(assign, AstType::binAssign, "not an assignment!");
			const auto& data{
				std::get<AstAssignData>(assign.dataVariant)
			};
			DataType value{ runExpression(*data.right) };
			innermostEnvironmentPointer->assign(data.varName, value);
			return value;
		}
		
		/**
		 * Resumes an assignment node. Since assigns can only stall on evaluating their right
		 * side, this type of node will not pop the stack.
		 */
		DataType resumeBinaryAssignment(const AstNode& assign){
			throwIfNotType(assign, AstType::binAssign, "not an assignment!");
			const auto& data{
				std::get<AstAssignData>(assign.dataVariant)
			};
			DataType value{ resumeExpression(*data.right) };
			innermostEnvironmentPointer->assign(data.varName, value);
			return value;
		}
		
		/**
		 * Runs a variable node. This type of node should never stall.
		 */
		DataType runVariable(const AstNode& variable){
			throwIfNotType(variable, AstType::variable, "not a variable node!");
			const auto& varName{
				std::get<AstVariableData>(variable.dataVariant).varName
			};
			return innermostEnvironmentPointer->get(varName);
		}
		
		/**
		 * Runs a function call node. This type of node may stall on its function expression,
		 * any one of its argument expressions, or the call to the function itself. This method
		 * will vomit its function expression and all the previously evaluated arguments in
		 * case of a stall.
		 */
		DataType runCall(const AstNode& call){
			throwIfNotType(call, AstType::call, "not a call node!");
			const auto& data{ std::get<AstCallData>(call.dataVariant) };
			
			//get the function wrapper, which may stall
			DataType functionWrapperData;
			try {
				functionWrapperData = runExpression(*data.funcExpr);
			}
			//stalled on evaluating the funcExpr!
			catch(const StallFlag&){
				pushStallNodeInfo({ AstType::call, StallNodeInfo::callFuncExpr });
				throw;
			}
			if(!std::holds_alternative<FunctionWrapper>(functionWrapperData)){
				throwError("tried to call non-function!");
			}
			const auto& functionWrapper{ std::get<FunctionWrapper>(functionWrapperData) };
			return evaluateFunctionWrapperCall(data, functionWrapperData, functionWrapper);
		}
		
		/**
		 * Given a function wrapper, calls the function as either a native or a user function.
		 * This method may stall on evaluating arguments or executing the function itself.
		 */
		DataType evaluateFunctionWrapperCall(
			const AstCallData& data,
			const DataType& functionWrapperData,
			const FunctionWrapper& functionWrapper
		){
			//case 1: native function
			if(std::holds_alternative<NativeFunctionWrapper>(functionWrapper)){
				const auto& nativeFunction{ unwrapNativeFunctionFromData(functionWrapper) };
				return evaluateNativeFunctionCall(data, functionWrapperData, nativeFunction);
			}
			//case 2: user function
			else{
				const UserFunctionWrapper& userFunctionWrapper{
					std::get<UserFunctionWrapper>(functionWrapper)
				};
				return evaluateUserFunctionCall(data, functionWrapper, userFunctionWrapper);
			}
		}
		
		/**
		 * Given a native function, evaluates the arguments and runs the function. This method
		 * may stall on evaluating arguments or executing the native function itself.
		 */
		DataType evaluateNativeFunctionCall(
			const AstCallData& data,
			const DataType& functionWrapperData,
			const NativeFunction& nativeFunction
		){
			//evaluate all args and store in a vector; each arg may stall
			std::vector<DataType> args{};
			int currentIndex{ 0 };
			try {
				for(; currentIndex < data.args.size(); ++currentIndex ) {
					args.push_back(runExpression(data.args[currentIndex]));
				}
			}
			//stalled on evaluating an arg!
			catch(const StallFlag&){
				int stalledIndex{ currentIndex };
				//first, push all the args
				//currentIndex points to the stalled arg (which wasn't complete)
				for(--currentIndex; currentIndex >= 0; --currentIndex){
					//by pushing the back element, the top will be the first arg
					pushStallNodeData(args.back());
					args.pop_back();
				}
				//second, push the function wrapper
				pushStallNodeData(functionWrapperData);
				//last, push a stall info pointing to the stalled arg
				pushStallNodeInfo({ AstType::call, stalledIndex });
				throw;
			}
			//pass to native function, which may stall (in fact this is where stalls start)
			return evaluateNativeFunctionWithArgs(nativeFunction, args, functionWrapperData);
		}
		
		/**
		 * Runs and returns the result of a native function. Begins a stall cycle if it stalls.
		 */
		DataType evaluateNativeFunctionWithArgs(
			const NativeFunction& nativeFunction,
			const std::vector<DataType>& args,
			const DataType& functionWrapperData
		){
			//try to run the native function
			try {
				return nativeFunction(args);
			}
			//stalled on native function!
			catch(const StallFlag&){
				//don't push the args - instead set the stalling native function call
				stallingNativeFunctionCall = { nativeFunction, args };
				//next, push the function wrapper
				pushStallNodeData(functionWrapperData);
				//last, push a stall info for a native call stall
				pushStallNodeInfo({ AstType::call, StallNodeInfo::callNative });
				throw;
			}
		}
		
		/**
		 * Given a user function wrapper, evaluates the arguments and passes execution to that
		 * function. This method may stall on evaluating arguments or executing the function
		 * body itself.
		 */
		DataType evaluateUserFunctionCall(
			const AstCallData& data,
			const DataType& functionWrapperData,
			const UserFunctionWrapper& userFunctionWrapper
		){
			//create a new environment for the function
			pushEmptyEnvironment();
			//evaluate args and store in the function environment; each arg may stall
			int currentIndex{ 0 };
			try {
				for(; currentIndex < data.args.size(); ++currentIndex ) {
					innermostEnvironmentPointer->define(
						userFunctionWrapper.paramNames[currentIndex],
						runExpression(data.args[currentIndex])
					);
				}
			}
			//stalled on an arg
			catch(const StallFlag&){
				//in this case, there is no need to push all the args since we aren't
				//resetting the environment. Do push the function and info though.
				pushStallNodeData(functionWrapperData);
				pushStallNodeInfo({ AstType::call, currentIndex });
				throw;
			}
			//all args evaluated, pass to function body
			return evaluateUserFunctionWithArgs(functionWrapperData, userFunctionWrapper);
		}
		
		/**
		 * Runs and returns the result of a user function. Execution of the function body may
		 * stall.
		 */
		DataType evaluateUserFunctionWithArgs(
			const DataType& functionWrapperData,
			const UserFunctionWrapper& userFunctionWrapper
		){
			try{
				//pass to the function body
				runStatement(*userFunctionWrapper.body);
				popEnvironment();
				//if no throw/return occurred, return false
				return DataType{ false };
			}
			//if we caught a DataType, that's our return value
			catch(const DataType& returnValue){
				popEnvironment();
				return returnValue;
			}
			//stalled on the function body; DO NOT RESET ENVIRONMENT
			catch(const StallFlag&){
				/*
				 * stalled on a native function! vomit onto the stack and rethrow, but
				 * DO NOT RESET THE ENVIRONMENT POINTER since we must resume in the
				 * innermost environment.
				 */
				pushStallNodeData(functionWrapperData);
				pushStallNodeInfo({ AstType::call, StallNodeInfo::callUser });
				throw;
			}
			//error! reset environment
			catch(...){
				//if anything else was thrown, that's an error, and we reset the environment
				popEnvironment();
				throw;
			}
		}
		
		/**
		 * Resumes a function call node. A native function is the root of all stalls, but a user
		 * function may be treated as a block of sorts.
		 */
		DataType resumeCall(const AstNode& call){
			throwIfNotType(call, AstType::call, "not a call node!");
			const auto& data{ std::get<AstCallData>(call.dataVariant) };
			//get the stall info
			const StallNodeInfo& stallNodeInfo{ popLastStallNodeInfo() };
			throwIfNotType(stallNodeInfo, AstType::call, "bad stall type: not call!");
			
			//case 1: we stalled on a native call, which means we finally return
			if(stallNodeInfo.index == StallNodeInfo::callNative){
				DataType returnValue = stallReturn;
				stallReturn = { false };
				return returnValue;
			}
			
			//case 2: we stalled on the getting the function wrapper
			if(stallNodeInfo.index == StallNodeInfo::callFuncExpr){
				//try to get the function wrapper again
				DataType functionWrapperData;
				try {
					functionWrapperData = resumeExpression(*data.funcExpr);
				}
				//stalled again on the funcExpr!
				catch(const StallFlag&){
					pushStallNodeInfo({ AstType::call, StallNodeInfo::callFuncExpr });
					throw;
				}
				if(!std::holds_alternative<FunctionWrapper>(functionWrapperData)){
					throwError("tried to call non-function!");
				}
				const auto& functionWrapper{ std::get<FunctionWrapper>(functionWrapperData) };
				return evaluateFunctionWrapperCall(data, functionWrapperData, functionWrapper);
			}
			
			//case 3: we stalled on either a native function or a user function. Get the stored
			//function wrapper off the stack
			const DataType& functionWrapperData{ popLastStallData() };
			const auto& functionWrapper{ std::get<FunctionWrapper>(functionWrapperData) };
			return resumeEvaluatingFunctionWrapperCall(
				data,
				stallNodeInfo,
				functionWrapperData,
				functionWrapper
			);
		}
		
		/**
		 * Resumes evaluating a function wrapper call. The call may have been a native function
		 * call, in which case it is the root of the stall, or it may have been a user function
		 * call, in which case it is not. Furthermore, the stall may have occurred when trying
		 * to evaluate an argument.
		 */
		DataType resumeEvaluatingFunctionWrapperCall(
			const AstCallData& data,
			const StallNodeInfo& stallNodeInfo,
			const DataType& functionWrapperData,
			const FunctionWrapper& functionWrapper
		){
			//case 1: native function
			if(std::holds_alternative<NativeFunctionWrapper>(functionWrapper)){
				const auto& nativeFunction{ unwrapNativeFunctionFromData(functionWrapper) };
				return resumeEvaluatingNativeFunctionCall(
					data,
					stallNodeInfo,
					functionWrapperData,
					nativeFunction
				);
			}
			//case 2: user function
			else{
				const UserFunctionWrapper& userFunctionWrapper{
					std::get<UserFunctionWrapper>(functionWrapper)
				};
				return resumeEvaluatingUserFunctionCall(
					data,
					stallNodeInfo,
					functionWrapper,
					userFunctionWrapper
				);
			}
		}
		
		/**
		 * Resumes a native function call. This method should only be called if the stall
		 * occurred during evaluation of an argument expression for a native function.
		 */
		DataType resumeEvaluatingNativeFunctionCall(
			const AstCallData& data,
			const StallNodeInfo& stallNodeInfo,
			const DataType& functionWrapperData,
			const NativeFunction& nativeFunction
		){
			if(stallNodeInfo.index < 0){
				throwError("trying to resume a native function call arg but stall index < 0!");
			}
			std::vector<DataType> args{};
			//retrieve stored args
			int currentIndex{ 0 };
			for(; currentIndex < stallNodeInfo.index; ++currentIndex){
				args.push_back(popLastStallData());
			}
			//try to get the rest of the args - may stall again!
			try {
				//resume evaluating the stalled arg
				args.push_back(resumeExpression(data.args[currentIndex]));
				++currentIndex;
				//evaluate remainder of args as normal
				for(; currentIndex < data.args.size(); ++currentIndex ) {
					args.push_back(runExpression(data.args[currentIndex]));
				}
			}
			//stalled on an arg again!
			catch(const StallFlag&){
				int stalledIndex{ currentIndex };
				//first, push all the args
				//currentIndex points to the stalled arg (which wasn't complete)
				for(--currentIndex; currentIndex >= 0; --currentIndex){
					//by pushing the back element, the top will be the first arg
					pushStallNodeData(args.back());
					args.pop_back();
				}
				//second, push the function wrapper
				pushStallNodeData(functionWrapperData);
				//last, push a stall info pointing to the stalled arg
				pushStallNodeInfo({ AstType::call, stalledIndex });
				throw;
			}
			//pass to native function, which may stall (in fact this is where stalls start)
			return evaluateNativeFunctionWithArgs(nativeFunction, args, functionWrapperData);
		}
		
		DataType resumeEvaluatingUserFunctionCall(
			const AstCallData& data,
			const StallNodeInfo& stallNodeInfo,
			const DataType& functionWrapperData,
			const UserFunctionWrapper& userFunctionWrapper
		){
			//case 1: we stalled on an arg
			if(stallNodeInfo.index >= 0){
				int currentIndex{ stallNodeInfo.index };
				try{
					//try to evaluate the stalled arg
					/*
					 * The statements below are split up because when we resume from a stall,
					 * we are currently in the environment containing the native function call.
					 * resumeExpression() will reset the environment to the correct one
					 * containing the parameter environment, but we should not access
					 * innermostEnvironmentPointer prior to calling resumeExpression on the
					 * stalled arg.
					 */
					const DataType& stalledArg{ resumeExpression(data.args[currentIndex]) };
					innermostEnvironmentPointer->define(
						userFunctionWrapper.paramNames[currentIndex],
						stalledArg
					);
					++currentIndex;
					//try to evaluate the remaining args
					for(; currentIndex < data.args.size(); ++currentIndex ) {
						innermostEnvironmentPointer->define(
							userFunctionWrapper.paramNames[currentIndex],
							runExpression(data.args[currentIndex])
						);
					}
				}
				//stalled on an arg again!
				catch(const StallFlag&){
					//in this case, there is no need to push all the args since we aren't
					//resetting the environment. Do push the function and info though.
					pushStallNodeData(functionWrapperData);
					pushStallNodeInfo({ AstType::call, currentIndex });
					throw;
				}
				//all args evaluated and put into the environment, now pass to function body
				return evaluateUserFunctionWithArgs(functionWrapperData, userFunctionWrapper);
			}
			//case 2: we stalled in the function body
			else if(stallNodeInfo.index == StallNodeInfo::callUser){
				//attempt to resume executing the body statement
				try{
					//pass to the function body
					resumeStatement(*userFunctionWrapper.body);
					popEnvironment();
					//if no throw/return occurred, return false
					return DataType{ false };
				}
				//if we caught a DataType, that's our return value
				catch(const DataType& returnValue){
					popEnvironment();
					return returnValue;
				}
				//stalled on the function body again; DO NOT RESET ENVIRONMENT
				catch(const StallFlag&){
					/*
					 * stalled on a native function! vomit onto the stack and rethrow, but
					 * DO NOT RESET THE ENVIRONMENT POINTER since we must resume in the
					 * innermost environment.
					 */
					pushStallNodeData(functionWrapperData);
					pushStallNodeInfo({ AstType::call, StallNodeInfo::callUser });
					throw;
				}
				//error! reset environment
				catch(...){
					//if anything else was thrown, that's an error, and we reset the environment
					popEnvironment();
					throw;
				}
			}
			else{
				throwError("bad stall index in resume user func call!");
				return false;//dummy return
			}
		}
		
		/**
		 * Returns true if the given data is of one of the types specified as type parameters.
		 */
		template <typename T, typename... Ts>
		bool holdsAlternatives(const DataType& dataType){
			//recursive case
			if constexpr(static_cast<bool>(sizeof...(Ts))){
				return std::holds_alternative<T>(dataType)
					|| holdsAlternatives<Ts...>(dataType);
			}
			//base case
			else{
				return std::holds_alternative<T>(dataType);
			}
		}
		
		/**
		 * Does a type-checked conversion of a given data to a native function.
		 */
		NativeFunction unwrapNativeFunctionFromData(const DataType& data){
			if(std::holds_alternative<FunctionWrapper>(data)){
				return unwrapNativeFunction(std::get<FunctionWrapper>(data));
			}
			else{
				throwError("tried to unwrapNativeFunctionFromData a non-function!");
				return false;//dummy return
			}
		}
		
		/**
		 * Does a type-checked conversion of a function wrapper to a native function.
		 */
		NativeFunction unwrapNativeFunction(const FunctionWrapper& functionWrapper){
			if(std::holds_alternative<NativeFunctionWrapper>(functionWrapper)){
				return std::any_cast<NativeFunction>(
					std::get<NativeFunctionWrapper>(functionWrapper)
				);
			}
			else{
				throwError("tried to unwrapNativeFunctionFromData a user function!");
				return false;//dummy return
			}
		}
		
		/**
		 * Pushes the given info element onto the stall stack
		 */
		void pushStallNodeInfo(const StallNodeInfo& stallNodeInfo){
			stallStack.push_back(stallNodeInfo);
		}
		
		/**
		 * Pushes the given data element onto the stall stack
		 */
		void pushStallNodeData(const DataType& data){
			stallStack.push_back(data);
		}
		
		/**
		 * Pops and returns the top element of the stall stack as an info element
		 */
		StallNodeInfo popLastStallNodeInfo(){
			StallNodeInfo toRet{ std::get<StallNodeInfo>(stallStack.back()) };
			stallStack.pop_back();
			return toRet;
		}
		
		/**
		 * Pops and returns the top element of the stall stack as a data element
		 */
		DataType popLastStallData(){
			DataType toRet{ std::get<DataType>(stallStack.back()) };
			stallStack.pop_back();
			return toRet;
		}
		
		/**
		 * Sets the innermost environment pointer to a new environment which is the direct
		 * child of the previous environment.
		 */
		void pushEmptyEnvironment(){
			innermostEnvironmentPointer = std::make_shared<Environment>(
				innermostEnvironmentPointer
			);
		}
		
		/**
		 * Sets the innermost environment pointer to the parent of the current environment.
		 */
		void popEnvironment(){
			innermostEnvironmentPointer
				= innermostEnvironmentPointer->getEnclosingEnvironmentPointer();
		}
		
		/**
		 * Throws an error if the given ast node is not of the specified ast type.
		 */
		static void throwIfNotType(
			const AstNode& node,
			AstType type,
			const std::string& errorMsg
		){
			if(node.type != type){
				throwError(errorMsg);
			}
		}
		
		/**
		 * Throws an error if the given stall node info is not of the specified ast type
		 */
		static void throwIfNotType(
			const StallNodeInfo& stallNodeInfo,
			AstType type,
			const std::string& errorMsg
		){
			if(stallNodeInfo.type != type){
				throwError(errorMsg);
			}
		}
		
		static void throwError(const std::string& errorMsg){
			throw std::runtime_error{ "Darkness interpreter " + errorMsg };
		}
	
	protected:
		class Environment{
		private:
			//fields
			std::shared_ptr<Environment> enclosingEnvironmentPointer{};
			std::unordered_map<std::string, DataType> identifierMap{};
		
		public:
			Environment()
				: enclosingEnvironmentPointer{ nullptr }
				, identifierMap{} {
			}
			
			explicit Environment(
				const std::shared_ptr<Environment>& enclosingEnvironmentPointer
			)	: enclosingEnvironmentPointer{ enclosingEnvironmentPointer }
				, identifierMap{} {
			}
			
			void define(const std::string& name, const DataType& value){
				identifierMap[name] = value;
			}
			
			void assign(const std::string& name, const DataType& value){
				const auto& found{ identifierMap.find(name) };
				if(found != identifierMap.end()){
					define(name, value);
				}
				else if(enclosingEnvironmentPointer){
					enclosingEnvironmentPointer->assign(name, value);
				}
				else{
					throwError("bad assign var name: " + name);
				}
			}
			
			DataType get(const std::string& name){
				const auto& found{ identifierMap.find(name) };
				if(found != identifierMap.end()){
					return found->second;
				}
				else if(enclosingEnvironmentPointer){
					return enclosingEnvironmentPointer->get(name);
				}
				else{
					throwError("bad get var name: " + name);
					return false;//dummy return
				}
			}
			
			DataType get(const std::string& name, const std::string& errorMsg){
				const auto& found{ identifierMap.find(name) };
				if(found != identifierMap.end()){
					return found->second;
				}
				else if(enclosingEnvironmentPointer){
					return enclosingEnvironmentPointer->get(name);
				}
				else{
					throwError(errorMsg);
					return false;//dummy return
				}
			}
			
			bool contains(const std::string& name){
				const auto& found{ identifierMap.find(name) };
				return found != identifierMap.end();
			}
			
			std::shared_ptr<Environment> getEnclosingEnvironmentPointer(){
				return enclosingEnvironmentPointer;
			}
		};
	};
}