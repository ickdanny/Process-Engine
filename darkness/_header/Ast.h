#pragma once

#include <vector>
#include <string>
#include <variant>

namespace darkness{
	enum class AstType{
		error,
		
		call,		//a function call
		intLit,		//an integer literal
		floatLit,	//a float literal
		stringLit,	//a string literal
		
		numAstTypes
	};
	
	struct AstNode; //forward declare
	
	struct AstCallData{
		std::string functionName{};
		std::vector<AstNode> arguments{};
	};
	
	struct AstIntLitData{
		int value{};
	};
	
	struct AstFloatLitData{
		float value{};
	};
	
	struct AstStringLitData{
		std::string value{};
	};
	
	struct AstNode{
		AstType type{};
		std::variant<
			AstCallData,
			AstIntLitData,
			AstFloatLitData,
			AstStringLitData
		> dataVariant{};
	};
}