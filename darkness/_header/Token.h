#pragma once

#include <string>

namespace darkness{
	enum class TokenType{
		error,
		
		symbol,		// names must begin with a letter and only contain a-z, 0-9, _
		number,		// starts with a digit
		string,		// strings within double quotes
		plus,		// +
		minus,		// -
		asterisk,	// *
		fSlash,		// /
		equals,		// =
		lParen,		// (
		rParen,		// )
		lCurly,		// {
		rCurly,		// }
		semicolon,	// ; used as delimiter
		
		numTokenTypes
	};
	
	struct Token{
		TokenType type{};
		std::string value{};
	};
}
