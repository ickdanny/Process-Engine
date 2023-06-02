#pragma once

#include "Logging.h"
#include "Lexer.h"

namespace darkness{
	void test() {
		std::istringstream input("int i = 0;");
		Lexer lexer{};
		auto tokens = lexer.lex(input);
		for(const auto& token : tokens){
			wasp::debug::log(token.value);
		}
	}
}
