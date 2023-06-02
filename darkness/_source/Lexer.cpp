#include "Lexer.h"

#include <cctype>

namespace darkness{
	std::vector<Token> Lexer::lex(std::istringstream& input) {
		std::vector<Token> toRet{};
		while(input.good() && !input.eof()) {
			current = input.get();
			
			//skip whitespaces
			if(std::isspace(current)) {
				continue;
			}
			
			//symbols must begin with a letter
			if(std::isalpha(current)) {
				toRet.push_back(extractSymbol(input));
				continue;
			}
			
			//numbers must begin with a digit (dots not acceptable)
			if(std::isdigit(current)) {
				toRet.push_back(extractNumber(input));
				continue;
			}
			
			switch(current){
				//strings begin with a double quote
				case '"': toRet.push_back(extractString(input)); break;
				
				//single char operators
				case '+': toRet.push_back({ TokenType::plus, "+" }); break;
				case '-': toRet.push_back({ TokenType::minus, "-" }); break;
				case '*': toRet.push_back({ TokenType::asterisk, "*" }); break;
				case '/': toRet.push_back({ TokenType::fSlash, "/" }); break;
				case '=': toRet.push_back({ TokenType::equals, "=" }); break;
				case '(': toRet.push_back({ TokenType::lParen, "(" }); break;
				case ')': toRet.push_back({ TokenType::rParen, ")" }); break;
				case '{': toRet.push_back({ TokenType::lCurly, "{" }); break;
				case '}': toRet.push_back({ TokenType::rCurly, "}" }); break;
				case ';': toRet.push_back({ TokenType::semicolon, ";" }); break;
				default:
					throw std::runtime_error{
						"Darkness lexer bad base char: " + std::to_string(current)
					};
			}
		}
		return toRet;
	}
	
	Token Lexer::extractSymbol(std::istringstream& input) {
		std::ostringstream valueOutputStream{};
		//insert the first char of the symbol onto the output
		valueOutputStream << current;
		while(input.good() && !input.eof()) {
			current = input.get();
			//if the next character is a letter, number, or underscore, it's part of the symbol
			if(isalnum(current) || current == '_') {
				valueOutputStream << current;
			}
			//if the next character is a space, we have reached the end of the symbol
			else if(isspace(current)){
				break;
			}
			//if the next character is anything else, throw an error
			else{
				throw std::runtime_error{
					"Darkness lexer bad symbol char: " + std::to_string(current)
				};
			}
		}
		if(!input.good()){
			throw std::runtime_error{ "Darkness lexer input stringstream bad!" };
		}
		return { TokenType::symbol, valueOutputStream.str() };
	}
	
	Token Lexer::extractNumber(std::istringstream& input) {
		std::ostringstream valueOutputStream{};
		//insert the first char of the number onto the output
		valueOutputStream << current;
		while(input.good() && !input.eof()) {
			current = input.peek();
			//if the next character is a number or dot, it's part of the number
			if(isdigit(current) || current == '.') {
				valueOutputStream << current;
				//advance input
				input.get();
			}
			//if the next character is anything else, we have reached the end of the number
			else {
				break;
			}
		}
		if(!input.good()){
			throw std::runtime_error{ "Darkness lexer input stringstream bad!" };
		}
		return { TokenType::number, valueOutputStream.str() };
	}
	
	Token Lexer::extractString(std::istringstream& input) {
		std::ostringstream valueOutputStream{};
		//do NOT insert the first character, since it is the opening double quote
		while(input.good() && !input.eof()) {
			current = input.get();
			//if the next character is a double quote, we are done
			if(current == '"'){
				break;
			}
			//otherwise, append to the output stream
			valueOutputStream << current;
		}
		if(!input.good()){
			throw std::runtime_error{ "Darkness lexer input stringstream bad!" };
		}
		return { TokenType::string, valueOutputStream.str() };
	}
	
}