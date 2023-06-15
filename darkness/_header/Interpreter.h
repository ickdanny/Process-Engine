#pragma once

#include "Ast.h"

#include <stdexcept>
#include <functional>
#include <any>

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
		};
		struct StallingNativeFunctionCall{
			NativeFunction stallingNativeFunction{};
			std::vector<DataType> args{};
			
			DataType run(){
				stallingNativeFunction(args);
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
		/**
		 * Runs a darkness script. If the given AstNode is of any other type, throws an error.
		 * A script may stall on any of its statements.
		 */
		void runScript(const AstNode& script){
			//todo: maybe let scripts return stuff? Can define functions in different scripts
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
				return;
			}
		}
		
		/**
		 * Resumes a stalled darkness script. If the given AstNode is of any other type, throws
		 * an error. The script may stall on the same statement, or it may stall on a new
		 * statement.
		 */
		 void resumeScript(const AstNode& script){
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
				return;
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
				return;
			}
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
				/**
				 * stalled on a native function! vomit onto the stack and rethrow, but
				 * DO NOT RESET THE ENVIRONMENT POINTER since we must resume in the inner-most
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
		bool evaluateUnaryBang(const DataType& argValue){
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
		
		//todo: continue refactoring and implementing stalling
		
		DataType runBinaryPlus(const AstNode& binary){
			throwIfNotType(binary, AstType::binPlus, "not binary plus!");
			DataType leftValue{ runExpression(
				*std::get<AstBinData>(binary.dataVariant).left
			) };
			DataType rightValue{ runExpression(
				*std::get<AstBinData>(binary.dataVariant).right
			) };
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
		
		DataType runBinaryMinus(const AstNode& binary){
			throwIfNotType(binary, AstType::binMinus, "not binary minus!");
			DataType leftValue{ runExpression(
				*std::get<AstBinData>(binary.dataVariant).left
			) };
			DataType rightValue{ runExpression(
				*std::get<AstBinData>(binary.dataVariant).right
			) };
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
		
		DataType runBinaryStar(const AstNode& binary){
			throwIfNotType(binary, AstType::binStar, "not binary star!");
			DataType leftValue{ runExpression(
				*std::get<AstBinData>(binary.dataVariant).left
			) };
			DataType rightValue{ runExpression(
				*std::get<AstBinData>(binary.dataVariant).right
			) };
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
		
		DataType runBinaryForwardSlash(const AstNode& binary){
			throwIfNotType(binary, AstType::binForwardSlash, "not binary forward slash!");
			DataType leftValue{ runExpression(
				*std::get<AstBinData>(binary.dataVariant).left
			) };
			DataType rightValue{ runExpression(
				*std::get<AstBinData>(binary.dataVariant).right
			) };
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
		
		DataType runBinaryDualEqual(const AstNode& binary){
			throwIfNotType(binary, AstType::binDualEqual, "not binary dual equal!");
			return evaluateBinaryDualEqual(binary);
		}
		
		DataType runBinaryBangEqual(const AstNode& binary){
			throwIfNotType(binary, AstType::binBangEqual, "not binary bang equal!");
			return !evaluateBinaryDualEqual(binary);
		}
		
		//runs a dual equal operation on two sides of a binary expression without ast checking
		bool evaluateBinaryDualEqual(const AstNode& binary){
			DataType leftValue{ runExpression(
				*std::get<AstBinData>(binary.dataVariant).left
			) };
			DataType rightValue{ runExpression(
				*std::get<AstBinData>(binary.dataVariant).right
			) };
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
			}
		}
		
		DataType runBinaryGreater(const AstNode& binary){
			throwIfNotType(binary, AstType::binGreater, "not binary greater!");
			DataType leftValue{ runExpression(
				*std::get<AstBinData>(binary.dataVariant).left
			) };
			DataType rightValue{ runExpression(
				*std::get<AstBinData>(binary.dataVariant).right
			) };
			return evaluateBinaryGreater(leftValue, rightValue);
		}
		
		DataType runBinaryGreaterEqual(const AstNode& binary){
			throwIfNotType(binary, AstType::binGreaterEqual, "not binary greater equal!");
			DataType leftValue{ runExpression(
				*std::get<AstBinData>(binary.dataVariant).left
			) };
			DataType rightValue{ runExpression(
				*std::get<AstBinData>(binary.dataVariant).right
			) };
			//switch left and right and also negate
			return !evaluateBinaryGreater(rightValue, leftValue);
		}
		
		DataType runBinaryLess(const AstNode& binary){
			throwIfNotType(binary, AstType::binLess, "not binary less!");
			DataType leftValue{ runExpression(
				*std::get<AstBinData>(binary.dataVariant).left
			) };
			DataType rightValue{ runExpression(
				*std::get<AstBinData>(binary.dataVariant).right
			) };
			//switch left and right
			return evaluateBinaryGreater(rightValue, leftValue);
		}
		
		DataType runBinaryLessEqual(const AstNode& binary){
			throwIfNotType(binary, AstType::binLessEqual, "not binary less equal!");
			DataType leftValue{ runExpression(
				*std::get<AstBinData>(binary.dataVariant).left
			) };
			DataType rightValue{ runExpression(
				*std::get<AstBinData>(binary.dataVariant).right
			) };
			//negate
			return !evaluateBinaryGreater(leftValue, rightValue);
		}
		
		//runs a greater operation on two sides of a binary expression without ast checking
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
			}
		}
		
		DataType runBinaryAmpersand(const AstNode& binary){
			throwIfNotType(binary, AstType::binAmpersand, "not binary ampersand!");
			DataType leftValue{ runExpression(
				*std::get<AstBinData>(binary.dataVariant).left
			) };
			if(leftValue.index() != boolIndex){
				throwError("left side of '&' not bool!");
			}
			if(!std::get<bool>(leftValue)){
				//if the left side was false, short circuit and return false
				return false;
			}
			DataType rightValue{ runExpression(
				*std::get<AstBinData>(binary.dataVariant).right
			) };
			if(rightValue.index() != boolIndex){
				throwError("right side of '&' not bool!");
			}
			//if the left side is true, the entire expression evaluates to the right side
			return std::get<bool>(rightValue);
		}
		
		DataType runBinaryVerticalBar(const AstNode& binary){
			throwIfNotType(binary, AstType::binVerticalBar, "not binary vertical bar!");
			DataType leftValue{ runExpression(
				*std::get<AstBinData>(binary.dataVariant).left
			) };
			if(leftValue.index() != boolIndex){
				throwError("left side of '|' not bool!");
			}
			if(std::get<bool>(leftValue)){
				//if the left side was true, short circuit and return true
				return true;
			}
			DataType rightValue{ runExpression(
				*std::get<AstBinData>(binary.dataVariant).right
			) };
			if(rightValue.index() != boolIndex){
				throwError("right side of '|' not bool!");
			}
			//if the left side is false, the entire expression evaluates to the right side
			return std::get<bool>(rightValue);
		}
		
		DataType runBinaryAssignment(const AstNode& binary){
			throwIfNotType(binary, AstType::binAssign, "not an assignment!");
			const auto& data{
				std::get<AstAssignData>(binary.dataVariant)
			};
			DataType value{ runExpression(*data.right) };
			innermostEnvironmentPointer->assign(data.varName, value);
			return value;
		}
		
		DataType runVariable(const AstNode& variable){
			throwIfNotType(variable, AstType::variable, "not a variable node!");
			const auto& varName{
				std::get<AstVariableData>(variable.dataVariant).varName
			};
			return innermostEnvironmentPointer->get(varName);
		}
		
		DataType runCall(const AstNode& call){
			throwIfNotType(call, AstType::call, "not a call node!");
			const auto& data{ std::get<AstCallData>(call.dataVariant) };
			
			//get the function wrapper
			const auto& functionWrapperData{ runExpression(*data.funcExpr) };
			if(!std::holds_alternative<FunctionWrapper>(functionWrapperData)){
				throwError("tried to call non-function!");
			}
			const auto& functionWrapper{ std::get<FunctionWrapper>(functionWrapperData) };
			
			//case 1: native function
			if(std::holds_alternative<NativeFunctionWrapper>(functionWrapper)){
				const auto& nativeFunction{ unwrapNativeFunctionFromData(functionWrapper) };
				//evaluate all args and store in a vector
				std::vector<DataType> args{};
				for(const auto& argNode : data.args){
					args.push_back(runExpression(argNode));
				}
				//pass to native function
				return nativeFunction(args);
			}
			//case 2: user function
			else{
				const UserFunctionWrapper& userFunctionWrapper{
					std::get<UserFunctionWrapper>(functionWrapper)
				};
				//create a new environment for the function
				std::shared_ptr<Environment> previousEnvironment{
					innermostEnvironmentPointer
				};
				innermostEnvironmentPointer = std::make_shared<Environment>(
					previousEnvironment
				);
				try{
					//evaluate all args and store in the function environment
					for(int i = 0; i < data.args.size(); ++i){
						innermostEnvironmentPointer->define(
							userFunctionWrapper.paramNames[i],
							runExpression(data.args[i])
						);
					}
					//pass to the function body which may throw a DataType as its return
					runStatement(*userFunctionWrapper.body);
					innermostEnvironmentPointer
						= innermostEnvironmentPointer->getEnclosingEnvironmentPointer();
					//if no throw/return occurred, return false
					return DataType{ false };
				}
				catch(const DataType& returnValue){
					//if we caught a DataType, that's our return value
					innermostEnvironmentPointer
						= innermostEnvironmentPointer->getEnclosingEnvironmentPointer();
					return returnValue;
				}
				catch(...){
					//if anything else was thrown, that's an error
					innermostEnvironmentPointer
						= innermostEnvironmentPointer->getEnclosingEnvironmentPointer();
					throw;
				}
			}
		}
		
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
		
		NativeFunction unwrapNativeFunctionFromData(const DataType& data){
			if(std::holds_alternative<FunctionWrapper>(data)){
				return unwrapNativeFunction(std::get<FunctionWrapper>(data));
			}
			else{
				throwError("tried to unwrapNativeFunctionFromData a non-function!");
			}
		}
		
		NativeFunction unwrapNativeFunction(const FunctionWrapper& functionWrapper){
			if(std::holds_alternative<NativeFunctionWrapper>(functionWrapper)){
				return std::any_cast<NativeFunction>(
					std::get<NativeFunctionWrapper>(functionWrapper)
				);
			}
			else{
				throwError("tried to unwrapNativeFunctionFromData a user function!");
			}
		}
		
		void pushStallNodeInfo(const StallNodeInfo& stallNodeInfo){
			stallStack.push_back(stallNodeInfo);
		}
		
		void pushStallNodeData(const DataType& data){
			stallStack.push_back(data);
		}
		
		StallNodeInfo popLastStallNodeInfo(){
			 return std::get<StallNodeInfo>(stallStack.pop_back());
		}
		
		DataType popLastStallData(){
			 return std::get<DataType>(stallStack.pop_back());
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
		
		static void throwIfNotType(
			const AstNode& node,
			AstType type,
			const std::string& errorMsg
		){
			if(node.type != type){
				throwError(errorMsg);
			}
		}
		
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
				auto& found{ identifierMap.find(name) };
				if(found != identifierMap.end()){
					return found->second;
				}
				else if(enclosingEnvironmentPointer){
					return enclosingEnvironmentPointer->get(name);
				}
				else{
					throwError("bad get var name: " + name);
				}
			}
			
			DataType get(const std::string& name, const std::string& errorMsg){
				auto& found{ identifierMap.find(name) };
				if(found != identifierMap.end()){
					return found->second;
				}
				else if(enclosingEnvironmentPointer){
					return enclosingEnvironmentPointer->get(name);
				}
				else{
					throwError(errorMsg);
				}
			}
			
			bool contains(const std::string& name){
				auto& found{ identifierMap.find(name) };
				return found != identifierMap.end();
			}
			
			std::shared_ptr<Environment> getEnclosingEnvironmentPointer(){
				return enclosingEnvironmentPointer;
			}
		};
	};
}