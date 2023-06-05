#pragma once

#include "Logging.h"
#include "Lexer.h"

namespace darkness{
	void test() {
		auto input{"let i = 0;" };
		Lexer lexer{};
		auto tokens = lexer.lex(input);
		for(const auto& token : tokens){
			wasp::debug::log(std::to_string(static_cast<int>(token.type)));
		}
	}
}