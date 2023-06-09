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
		stmtVarDeclare,		// a variable declaration
		stmtIf,				// if ( ... )
		stmtWhile,			// while ( ... )
		stmtBlock,			// { ... }
		
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
		binAssign,			// var = a
		
		//unary expr types
		unaryBang,			// !a
		unaryPlus,			// +a
		unaryMinus,			// -a
		
		variable,			// foo, bar, etc...
		
		//literal expr types
		litBool,			// true or false
		litInt,				// any integer literal
		litFloat,			// any float literal
		litString,			// any string literal
		
		parenths,			// ( ... )
		
		call,				// a function call
		
		numAstTypes
	};
	
	struct AstNode; //forward declare
	
	struct AstStmtVarDeclareData{
		std::string varName{};
		std::unique_ptr<AstNode> initializer{};
	};
	
	struct AstStmtIfData{
		std::unique_ptr<AstNode> condition{};
		std::unique_ptr<AstNode> trueBranch{};
		std::unique_ptr<AstNode> falseBranch{};
	};
	
	struct AstStmtWhileData{
		std::unique_ptr<AstNode> condition{};
		std::unique_ptr<AstNode> body{};
	};
	
	struct AstStmtBlockData{
		std::vector<AstNode> statements{};
	};
	
	struct AstStmtExpressionData{
		std::unique_ptr<AstNode> expression{};
	};
	
	struct AstBinData{
		std::unique_ptr<AstNode> left{};
		std::unique_ptr<AstNode> right{};
	};
	
	struct AstAssignData{
		std::string varName{};
		std::unique_ptr<AstNode> right{};
	};
	
	struct AstUnaryData{
		std::unique_ptr<AstNode> arg{};
	};
	
	struct AstCallData{
		std::string functionName{};
		std::vector<AstNode> arguments{};
	};
	
	struct AstVariableData{
		std::string varName{};
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
	
	struct AstParenthsData{
		std::unique_ptr<AstNode> inside{};
	};
	
	struct AstNode{
		AstType type{};
		std::variant<
			AstStmtVarDeclareData,
			AstStmtIfData,
			AstStmtWhileData,
			AstStmtBlockData,
			AstStmtExpressionData,
			AstBinData,
			AstAssignData,
			AstUnaryData,
			AstVariableData,
			AstLitBoolData,
			AstLitIntData,
			AstLitFloatData,
			AstLitStringData,
			AstParenthsData,
			AstCallData
		> dataVariant{};
		
		operator bool() const {
			return type != AstType::error && type != AstType::numAstTypes;
		}
	};
}