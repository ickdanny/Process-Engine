#pragma once

#include "Token.h"

#include <vector>
#include <sstream>

namespace darkness{
	class Lexer{
	private:
		std::string_view input{};
		std::size_t nextPos{};
		unsigned char currentChar{};
		std::vector<Token> output{};
		
	public:
		std::vector<Token> lex(const std::string_view& input);
		
	private:
		//helper methods
		void extractToken();
		void discardComment();
		void extractSymbol();
		void extractNumber();
		void extractString();
		
		void pushToken(const Token& token);
		void advance();
		bool advanceIfMatch(unsigned char testFor);
		char peek();
		
		bool isEndOfInput();
	};
}