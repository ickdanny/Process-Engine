#pragma once

#include "Ast.h"

#include <stdexcept>
#include <functional>

namespace darkness{
	
	namespace reservedFunctionNames{
		static const std::string unaryBang{ "unaryBang" };
		static const std::string unaryPlus{ "unaryPlus" };
		static const std::string unaryMinus{ "unaryMinus" };
		
		static const std::string binaryPlus{ "binaryPlus" };
		static const std::string binaryMinus{ "binaryMinus" };
		static const std::string binaryStar{ "binaryStar" };
		static const std::string binaryForwardSlash{ "binaryForwardSlash" };
		static const std::string binaryDualEqual{ "binaryDualEqual" };
		static const std::string binaryGreater{ "binaryGreater" };
	}
	
	template <typename... CustomTypes>
	class Interpreter{
	protected:
		//typedefs
		using DataType = std::variant<bool, int, float, std::string, CustomTypes...>;
		using Function = std::function<DataType(std::vector<DataType>)>;
		using FunctionMap = std::unordered_map<std::string, Function>;
		
		//constants
		static constexpr auto numTypes{ std::variant_size_v<DataType> };
		static constexpr auto numCustomTypes{ sizeof...(CustomTypes) };
		static constexpr auto numBuiltInTypes{ numTypes - numCustomTypes };
		static constexpr auto boolIndex{ 0u };
		static constexpr auto intIndex{ 1u };
		static constexpr auto floatIndex{ 2u };
		static constexpr auto stringIndex{ 3u };
		
		//nested classes
		class Environment;
		
		//fields
		FunctionMap nativeFunctions{};
		std::shared_ptr<Environment> innermostEnvironmentPointer{};
	public:
		//Runs a darkness script. If the given AstNode is of any other type, throws an error.
		void runScript(const AstNode& script){
			//todo: maybe let scripts return stuff? Can define functions in different scripts
			throwIfNotType(script, AstType::script, "not script!");
			const auto& statements{ std::get<AstScriptData>(script.dataVariant).statements };
			for(const auto& statement : statements){
				runStatement(statement);
			}
		}
	
	private:
		void runStatement(const AstNode& statement){
			switch(statement.type){
				case AstType::stmtVarDeclare:
					runVarDeclare(statement);
					break;
				case AstType::stmtIf:
					runIf(statement);
					break;
				case AstType::stmtBlock:
					runBlock(statement);
					break;
				case AstType::stmtExpression:
					runExpression(
						std::get<AstStmtExpressionData>(statement.dataVariant).expression
					);
					break;
			}
		}
		
		void runVarDeclare(const AstNode& varDeclare){
			throwIfNotType(varDeclare, AstType::stmtVarDeclare, "not var declare!");
			const auto& data{
				std::get<AstStmtVarDeclareData>(varDeclare.dataVariant)
			};
			const auto& initializer{ data.initializer };
			DataType initData;	//uninitialized!
			if(initializer){
				initData = runExpression(initializer);
			}
			else{
				initData = DataType{ false };
			}
			innermostEnvironmentPointer->define(data.varName, initData);
		}
		
		void runIf(const AstNode& ifStatement){
			throwIfNotType(ifStatement, AstType::stmtIf, "not an if statement!");
			const auto& data{ std::get<AstStmtIfData>(ifStatement.dataVariant) };
			DataType conditionValue{ runExpression(data.condition) };
			if(conditionValue.index() != boolIndex){
				throw std::runtime_error{
					"Darkness interpreter if statement condition was not bool!"
				};
			}
			if(std::get<bool>(conditionValue)){
				runStatement(data.trueBranch);
			}
			else if(data.falseBranch){
				runStatement(data.falseBranch);
			}
		}
		
		void runBlock(const AstNode& block){
			throwIfNotType(block, AstType::stmtBlock, "not a block!");
			std::shared_ptr<Environment> previousEnvironment{ innermostEnvironmentPointer };
			const auto& statements{
				std::get<AstStmtBlockData>(block.dataVariant).statements
			};
			innermostEnvironmentPointer = std::make_shared<Environment>(previousEnvironment);
			try{
				for(const auto& statement : statements){
					runStatement(statement);
				}
				innermostEnvironmentPointer = previousEnvironment;
			}
			catch(...){
				innermostEnvironmentPointer = previousEnvironment;
				throw;
			}
		}
		
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
				case AstType::parenths:
					return runExpression(
						std::get<AstParenthsData>(expression.dataVariant).inside
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
				case AstType::binAssign:
					return runBinaryAssignment(expression);
					
				case AstType::variable:
					return runVariable(expression);
			}
		}
		
		DataType runUnaryBang(const AstNode& unary){
			throwIfNotType(unary, AstType::unaryBang, "not unary bang!");
			DataType argValue{ runExpression(
				std::get<AstUnaryData>(unary.dataVariant).arg
			) };
			if(holdsAlternatives<bool>(argValue)){
				return !std::get<bool>(argValue);
			}
			if(holdsAlternatives<int, float, std::string>(argValue)){
				throw std::runtime_error{
					"Darkness interpreter bad arg for unary bang!"
				};
			}
			//our arg is NOT a built-in type! look for a native function
			auto found{ nativeFunctions.find(reservedFunctionNames::unaryBang) };
			if(found != nativeFunctions.end()){
				return found->second({ argValue });
			}
			else{
				throw std::runtime_error{
					"Darkness interpreter no native unary bang function!"
				};
			}
		}
		
		DataType runUnaryPlus(const AstNode& unary){
			throwIfNotType(unary, AstType::unaryPlus, "not unary plus!");
			DataType argValue{ runExpression(
				std::get<AstUnaryData>(unary.dataVariant).arg
			) };
			if(holdsAlternatives<bool, std::string>(argValue)){
				throw std::runtime_error{
					"Darkness interpreter bad arg for unary plus!"
				};
			}
			if(holdsAlternatives<int, float>(argValue)){
				return argValue;
			}
			//our arg is NOT a built-in type! look for a native function
			auto found{ nativeFunctions.find(reservedFunctionNames::unaryPlus) };
			if(found != nativeFunctions.end()){
				found->second({ argValue });
			}
			else{
				throw std::runtime_error{
					"Darkness interpreter no native unary plus function!"
				};
			}
		}
		
		DataType runUnaryMinus(const AstNode& unary){
			throwIfNotType(unary, AstType::unaryMinus, "not unary minus!");
			DataType argValue{ runExpression(
				std::get<AstUnaryData>(unary.dataVariant).arg
			) };
			if(holdsAlternatives<bool, std::string>(argValue)){
				throw std::runtime_error{
					"Darkness interpreter bad arg for unary minus!"
				};
			}
			if(holdsAlternatives<int>(argValue)){
				return -std::get<int>(argValue);
			}
			if(holdsAlternatives<float>(argValue)){
				return -std::get<float>(argValue);
			}
			//our arg is NOT a built-in type! look for a native function
			auto found{ nativeFunctions.find(reservedFunctionNames::unaryMinus) };
			if(found != nativeFunctions.end()){
				found->second({ argValue });
			}
			else{
				throw std::runtime_error{
					"Darkness interpreter no native unary minus function!"
				};
			}
		}
		
		DataType runBinaryPlus(const AstNode& binary){
			throwIfNotType(binary, AstType::binPlus, "not binary plus!");
			DataType leftValue{ runExpression(
				std::get<AstBinData>(binary.dataVariant).left
			) };
			DataType rightValue{ runExpression(
				std::get<AstBinData>(binary.dataVariant).right
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
					throw std::runtime_error{
						"Darkness interpreter trying to add a bool!"
					};
				}
			}
			//one of our args is NOT a built-in type! look for a native function
			auto found{ nativeFunctions.find(reservedFunctionNames::binaryPlus) };
			if(found != nativeFunctions.end()){
				found->second({ leftValue, rightValue });
			}
			else{
				throw std::runtime_error{
					"Darkness interpreter no native binary plus function!"
				};
			}
		}
		
		DataType runBinaryMinus(const AstNode& binary){
			throwIfNotType(binary, AstType::binMinus, "not binary minus!");
			DataType leftValue{ runExpression(
				std::get<AstBinData>(binary.dataVariant).left
			) };
			DataType rightValue{ runExpression(
				std::get<AstBinData>(binary.dataVariant).right
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
					throw std::runtime_error{
						"Darkness interpreter trying to minus a bool!"
					};
				}
				if(leftIndex == stringIndex || rightIndex == stringIndex){
					throw std::runtime_error{
						"Darkness interpreter trying to minus a string!"
					};
				}
			}
			//one of our args is NOT a built-in type! look for a native function
			auto found{ nativeFunctions.find(reservedFunctionNames::binaryMinus) };
			if(found != nativeFunctions.end()){
				found->second({ leftValue, rightValue });
			}
			else{
				throw std::runtime_error{
					"Darkness interpreter no native binary minus function!"
				};
			}
		}
		
		DataType runBinaryStar(const AstNode& binary){
			throwIfNotType(binary, AstType::binStar, "not binary star!");
			DataType leftValue{ runExpression(
				std::get<AstBinData>(binary.dataVariant).left
			) };
			DataType rightValue{ runExpression(
				std::get<AstBinData>(binary.dataVariant).right
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
					throw std::runtime_error{
						"Darkness interpreter trying to star a bool!"
					};
				}
				if(leftIndex == stringIndex || rightIndex == stringIndex){
					throw std::runtime_error{
						"Darkness interpreter trying to star a string!"
					};
				}
			}
			//one of our args is NOT a built-in type! look for a native function
			auto found{ nativeFunctions.find(reservedFunctionNames::binaryStar) };
			if(found != nativeFunctions.end()){
				found->second({ leftValue, rightValue });
			}
			else{
				throw std::runtime_error{
					"Darkness interpreter no native binary star function!"
				};
			}
		}
		
		DataType runBinaryForwardSlash(const AstNode& binary){
			throwIfNotType(binary, AstType::binForwardSlash, "not binary forward slash!");
			DataType leftValue{ runExpression(
				std::get<AstBinData>(binary.dataVariant).left
			) };
			DataType rightValue{ runExpression(
				std::get<AstBinData>(binary.dataVariant).right
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
					throw std::runtime_error{
						"Darkness interpreter trying to forward slash a bool!"
					};
				}
				if(leftIndex == stringIndex || rightIndex == stringIndex){
					throw std::runtime_error{
						"Darkness interpreter trying to forward slash a string!"
					};
				}
			}
			//one of our args is NOT a built-in type! look for a native function
			auto found{ nativeFunctions.find(reservedFunctionNames::binaryForwardSlash) };
			if(found != nativeFunctions.end()){
				found->second({ leftValue, rightValue });
			}
			else{
				throw std::runtime_error{
					"Darkness interpreter no native binary forward slash function!"
				};
			}
		}
		
		DataType runBinaryDualEqual(const AstNode& binary){
			throwIfNotType(binary, AstType::binDualEqual, "not binary dual equal!");
			return binaryDualEqualHelper(binary);
		}
		
		DataType runBinaryBangEqual(const AstNode& binary){
			throwIfNotType(binary, AstType::binBangEqual, "not binary bang equal!");
			return !binaryDualEqualHelper(binary);
		}
		
		//runs a dual equal operation on two sides of a binary expression without ast checking
		bool binaryDualEqualHelper(const AstNode& binary){
			DataType leftValue{ runExpression(
				std::get<AstBinData>(binary.dataVariant).left
			) };
			DataType rightValue{ runExpression(
				std::get<AstBinData>(binary.dataVariant).right
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
						}
					}
				}
				if(leftIndex == boolIndex){
					if(rightIndex == boolIndex){
						return std::get<bool>(leftValue) == std::get<bool>(rightValue);
					}
					else{
						throw std::runtime_error{
							"Darkness interpreter trying to use == with only 1 bool!"
						};
					}
				}
				if(rightIndex == boolIndex){
					//we already know the left isn't a bool, so throw
					throw std::runtime_error{
						"Darkness interpreter trying to use == with only 1 bool!"
					};
				}
			}
			//one of our args is NOT a built-in type! look for a native function
			auto found{ nativeFunctions.find(reservedFunctionNames::binaryDualEqual) };
			if(found != nativeFunctions.end()){
				found->second({ leftValue, rightValue });
			}
			else{
				throw std::runtime_error{
					"Darkness interpreter no native binary dual equal function!"
				};
			}
		}
		
		DataType runBinaryGreater(const AstNode& binary){
			throwIfNotType(binary, AstType::binGreater, "not binary greater!");
			DataType leftValue{ runExpression(
				std::get<AstBinData>(binary.dataVariant).left
			) };
			DataType rightValue{ runExpression(
				std::get<AstBinData>(binary.dataVariant).right
			) };
			return binaryGreaterHelper(leftValue, rightValue);
		}
		
		DataType runBinaryGreaterEqual(const AstNode& binary){
			throwIfNotType(binary, AstType::binGreaterEqual, "not binary greater equal!");
			DataType leftValue{ runExpression(
				std::get<AstBinData>(binary.dataVariant).left
			) };
			DataType rightValue{ runExpression(
				std::get<AstBinData>(binary.dataVariant).right
			) };
			//switch left and right and also negate
			return !binaryGreaterHelper(rightValue, leftValue);
		}
		
		DataType runBinaryLess(const AstNode& binary){
			throwIfNotType(binary, AstType::binLess, "not binary less!");
			DataType leftValue{ runExpression(
				std::get<AstBinData>(binary.dataVariant).left
			) };
			DataType rightValue{ runExpression(
				std::get<AstBinData>(binary.dataVariant).right
			) };
			//switch left and right
			return binaryGreaterHelper(rightValue, leftValue);
		}
		
		DataType runBinaryLessEqual(const AstNode& binary){
			throwIfNotType(binary, AstType::binLessEqual, "not binary less equal!");
			DataType leftValue{ runExpression(
				std::get<AstBinData>(binary.dataVariant).left
			) };
			DataType rightValue{ runExpression(
				std::get<AstBinData>(binary.dataVariant).right
			) };
			//negate
			return !binaryGreaterHelper(leftValue, rightValue);
		}
		
		DataType runBinaryAmpersand(const AstNode& binary){
			throwIfNotType(binary, AstType::binAmpersand, "not binary ampersand!");
			DataType leftValue{ runExpression(
				std::get<AstBinData>(binary.dataVariant).left
			) };
			if(leftValue.index() != boolIndex){
				throw std::runtime_error{
					"Darkness interpreter left side of '&' not bool!"
				};
			}
			if(!std::get<bool>(leftValue)){
				//if the left side was false, short circuit and return false
				return false;
			}
			DataType rightValue{ runExpression(
				std::get<AstBinData>(binary.dataVariant).right
			) };
			if(rightValue.index() != boolIndex){
				throw std::runtime_error{
					"Darkness interpreter right side of '&' not bool!"
				};
			}
			return std::get<bool>(rightValue);
		}
		
		//runs a greater operation on two sides of a binary expression without ast checking
		bool binaryGreaterHelper(const DataType& leftValue, const DataType& rightValue){
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
					throw std::runtime_error{
						"Darkness interpreter trying to compare a bool!"
					};
				}
			}
			//one of our args is NOT a built-in type! look for a native function
			auto found{ nativeFunctions.find(reservedFunctionNames::binaryGreater) };
			if(found != nativeFunctions.end()){
				found->second({ leftValue, rightValue });
			}
			else{
				throw std::runtime_error{
					"Darkness interpreter no native binary greater function!"
				};
			}
		}
		
		DataType runBinaryAssignment(const AstNode& binary){
			throwIfNotType(binary, AstType::binAssign, "not an assignment!");
			const auto& data{
				std::get<AstAssignData>(binary.dataVariant)
			};
			DataType value{ runExpression(data.right) };
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
		
		void throwIfNotType(const AstNode& node, AstType type, const std::string& msg){
			if(node.type != type){
				throw std::runtime_error{ "Darkness interpreter " + msg };
			}
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
			
			Environment(const std::shared_ptr<Environment>& enclosingEnvironmentPointer)
				: enclosingEnvironmentPointer{ enclosingEnvironmentPointer }
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
					throw std::runtime_error{
						"Darkness interpreter bad assign var name: " + name
					};
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
					throw std::runtime_error{
						"Darkness interpreter bad get var name: " + name
					};
				}
			}
		};
	};
}