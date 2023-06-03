#pragma once

#include <string>

namespace darkness{
	enum class TokenType{
		error,
		
		symbol,		// names must begin with a letter and only contain a-z, 0-9, _
		number,		// starts with a digit
		string,		// strings within double quotes
		mathOp,		// + - * /
		equals,		// =
		dot,		// .
		comma,		// ,
		lParen,		// (
		rParen,		// )
		lCurly,		// {
		rCurly,		// }
		lBrace,		// [
		rBrace,		// ]
		lPointy,	// <
		rPointy,	// >
		semicolon,	// ; used as delimiter
		
		numTokenTypes
	};
	
	struct Token{
		TokenType type{};
		std::string value{};
	};
}
