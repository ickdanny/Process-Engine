#pragma once

#include <vector>
#include <string>
#include <variant>
#include <memory>

namespace darkness{
	enum class AstType{
		error,
		
		script,				// the base of every darkness script
		
		//statement types
		stmtExpression,		// an expression statement
		
		//binary expr types
		binPlus,			// a + b
		binMinus,			// a - b
		binStar,			// a * b
		binForwardSlash,	// a / b
		binDualEqual,		// a == b
		binBangEqual,		// a != b
		binGreater,			// a > b
		binGreaterEqual,	// a >= b
		binLess,			// a < b
		binLessEqual,		// a <= b
		binAmpersand,		// a & b
		binVerticalBar,		// a | b
		
		//unary expr types
		unaryBang,			// !a
		unaryPlus,			// +a
		unaryMinus,			// -a
		
		//literal expr types
		litBool,			// true or false
		litInt,				// any integer literal
		litFloat,			// any float literal
		litString,			// any string literal
		
		call,				// a function call
		
		numAstTypes
	};
	
	struct AstNode; //forward declare
	
	struct AstStmtExpressionData{
		std::unique_ptr<AstNode> expression{};
	};
	
	struct AstBinData{
		std::unique_ptr<AstNode> left{};
		std::unique_ptr<AstNode> right{};
	};
	
	struct AstUnaryData{
		std::unique_ptr<AstNode> arg{};
	};
	
	struct AstCallData{
		std::string functionName{};
		std::vector<AstNode> arguments{};
	};
	
	struct AstLitBoolData{
		bool value{};
	};
	
	struct AstLitIntData{
		int value{};
	};
	
	struct AstLitFloatData{
		float value{};
	};
	
	struct AstLitStringData{
		std::string value{};
	};
	
	struct AstNode{
		AstType type{};
		std::variant<
			AstStmtExpressionData,
			AstBinData,
			AstUnaryData,
			AstLitBoolData,
			AstLitIntData,
			AstLitFloatData,
			AstLitStringData,
			AstCallData
		> dataVariant{};
	};
}