#pragma once

#include "Token.h"
#include "Ast.h"

#include <vector>

namespace darkness{
	class Parser{
	private:
		std::size_t tokenPosition{};
		
	public:
		//entry point for the parser
		AstNode parse(const std::vector<Token>& tokens);
		
	private:
		AstNode parseStatement(const std::vector<Token>& tokens);
	};
}