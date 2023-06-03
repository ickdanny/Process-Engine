#pragma once

#include "Token.h"

#include <vector>
#include <sstream>

namespace darkness{
	class Lexer{
	private:
		unsigned char current{};
		
	public:
		std::vector<Token> lex(std::istringstream& input);
		
	private:
		//helper methods
		Token extractSymbol(std::istringstream& input);
		Token extractNumber(std::istringstream& input);
		Token extractString(std::istringstream& input);
	};
}