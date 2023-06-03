#pragma once

#include <vector>
#include <string>
#include <variant>

namespace darkness{
	enum class AstType{
		error,
		
		list,		//a list of statements basically
		call,		//a function call
		
		numAstTypes
	};
	
	struct AstNode; //forward declare
	
	struct AstListData{
		std::vector<AstNode> children{};
	};
	
	struct AstCallData{
		std::string functionName{};
		std::vector<AstNode> arguments{};
	};
	
	struct AstNode{
		AstType type{};
		std::variant<AstListData, AstCallData> dataVariant{};
	};
}