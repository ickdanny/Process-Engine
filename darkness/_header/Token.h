#pragma once

#include <string>
#include <variant>

namespace darkness{
	enum class TokenType{
		error,
		
		//literals
		identifier,		// names must begin with a letter and only contain a-z, 0-9, _
		string,			// strings within double quotes
		integer,		// starts with a digit, no dot
		floating,		// starts with a digit, has dot
		
		//operators
		plus,			// +
		minus,			// -
		star,			// *
		fSlash,			// /
		equal,			// =
		dualEqual,		// ==
		bang,			// !
		bangEqual,		// !=
		greater,		// >
		greaterEqual,	// >=
		less,			// <
		lessEqual,		// <=
		ampersand,		// &
		verticalBar,	// |
		dot,			// .
		comma,			// ,
		lParen,			// (
		rParen,			// )
		lCurly,			// {
		rCurly,			// }
		lBrace,			// [
		rBrace,			// ]
		semicolon,		// ; used as delimiter
		
		//keywords
		keyIf,
		keyElse,
		keyTrue,
		keyFalse,
		keyFunc,
		keyWhile,
		keyFor,
		keyLet,
		keyReturn,
		
		//end of source marker
		endMarker,
		
		numTokenTypes
	};
	
	struct Token{
		TokenType type{};
		std::variant<std::string, int, float> value{};
	};
}
