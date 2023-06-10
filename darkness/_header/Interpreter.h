#pragma once

#include "Ast.h"

#include <stdexcept>
#include <functional>

namespace darkness{
	
	namespace reservedFunctionNames{
		static const std::string unaryBang{ "unaryBang" };
		static const std::string unaryPlus{ "unaryPlus" };
		static const std::string unaryMinus{ "unaryMinus" };
		
		static const std::string binPlus{ "binPlus" };
		static const std::string binMinus{ "binMinus" };
		static const std::string binStar{ "binStar" };
		static const std::string binForwardSlash{ "binForwardSlash" };
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
		static constexpr auto numBuiltInTypes{ numTypes - numBuiltInTypes };
		
		//fields
		FunctionMap nativeFunctions{};
	public:
		//Runs a darkness script. If the given AstNode is of any other type, throws an error.
		void runScript(const AstNode& script){
		
		}
	private:
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
			throwIfNotType(binary, AstType::unaryMinus, "not binary plus!");
			DataType leftValue{ runExpression(
				std::get<AstBinData>(binary.dataVariant).left
			) };
			DataType rightValue{ runExpression(
				std::get<AstBinData>(binary.dataVariant).right
			) };
			auto leftIndex{ leftValue.index() };
			auto rightIndex{ rightValue.index() };
			if(leftIndex < numBuiltInTypes && rightIndex < numBuiltInTypes){
				//this is a built-in vs built-in operation defined here
				//todo: switch here
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
		
		void throwIfNotType(const AstNode& node, AstType type, const std::string& msg){
			if(node.type != type){
				throw std::runtime_error{ "Darkness interpreter " + msg };
			}
		}
	};
}