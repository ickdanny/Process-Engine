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
		stmtFuncDeclare,	// a function declaration
		stmtIf,				// if ( ... )
		stmtWhile,			// while ( ... )
		stmtReturn,			// a return from a function
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
		
		call,				// a function call
		
		//literal expr types
		litBool,			// true or false
		litInt,				// any integer literal
		litFloat,			// any float literal
		litString,			// any string literal
		
		parenthesis,		// ( ... )
		
		numAstTypes
	};
	
	struct AstNode; //forward declare
	
	struct AstScriptData{
		std::vector<AstNode> statements{};
	};
	
	struct AstStmtVarDeclareData{
		std::string varName{};
		std::unique_ptr<AstNode> initializer{};
	};
	
	struct AstStmtFuncDeclareData{
		std::string funcName{};
		std::vector<std::string> paramNames{};
		std::shared_ptr<AstNode> body{};	//use a shared pointer for the body for interpreting
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
	
	struct AstStmtReturnData{
		bool hasValue{};
		std::unique_ptr<AstNode> value{};
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
	
	struct AstVariableData{
		std::string varName{};
	};
	
	struct AstCallData{
		std::unique_ptr<AstNode> funcExpr{};
		std::vector<AstNode> args{};
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
	
	struct AstParenthesisData{
		std::unique_ptr<AstNode> inside{};
	};
	
	struct AstNode{
		AstType type{};
		std::variant<
			AstScriptData,
			AstStmtVarDeclareData,
			AstStmtFuncDeclareData,
			AstStmtIfData,
			AstStmtWhileData,
			AstStmtReturnData,
			AstStmtBlockData,
			AstStmtExpressionData,
			AstBinData,
			AstAssignData,
			AstUnaryData,
			AstVariableData,
			AstCallData,
			AstLitBoolData,
			AstLitIntData,
			AstLitFloatData,
			AstLitStringData,
			AstParenthesisData
		> dataVariant{};
		
		explicit operator bool() const {
			return type != AstType::error && type != AstType::numAstTypes;
		}
	};
}