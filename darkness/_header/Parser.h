#pragma once

#include "Token.h"
#include "Ast.h"

#include <vector>

namespace darkness{
	class Parser{
	private:
		std::vector<Token> input{};
		std::size_t nextPos{};
		
	public:
		//entry point for the parser
		AstNode parse(const std::vector<Token>& input);
		
	private:
		
		//statements
		AstNode parseDeclaration();
		AstNode parseVariableDeclaration();
		AstNode parseFunctionDeclaration();
		AstNode parseStatement();
		AstNode parseIf();
		AstNode parseWhile();
		AstNode parseFor();
		AstNode parseReturn();
		AstNode parseBlock();
		AstNode parseExpressionStatement();
		
		//expressions
		AstNode parseExpression();
		AstNode parseAssignment();
		AstNode parseOr();
		AstNode parseAnd();
		AstNode parseEquality();
		AstNode parseComparison();
		AstNode parseTerm();
		AstNode parseFactor();
		AstNode parseUnary();
		AstNode parseCall();
		AstNode parsePrimary();
		
		template <TokenType tokenType, TokenType... Ts>
		bool advanceIfMatch() {
			if(match(tokenType)){
				advance();
				return true;
			}
			
			if constexpr(static_cast<bool>(sizeof...(Ts))){
				return advanceIfMatch<Ts...>();
			}
			else{
				return false;
			}
		}
		bool match(TokenType type);
		Token& peek();
		Token& previous();
		Token& advance();
		Token& consumeOrThrow(TokenType expectedToken, const std::string& throwMsg);
		
		bool isEndOfInput();
	};
}