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
		AstNode parseStatement();
		AstNode parseExpressionStatement();
		
		//expressions
		AstNode parseExpression();
		AstNode parseEquality();
		AstNode parseComparison();
		AstNode parseTerm();
		AstNode parseFactor();
		AstNode parseUnary();
		AstNode parsePrimary();
		
		template <TokenType T, TokenType... Ts>
		bool advanceIfMatch() {
			if(match(T)){
				advance();
				return true;
			}
			
			if constexpr(sizeof...(Ts)){
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