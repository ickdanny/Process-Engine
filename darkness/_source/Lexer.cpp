#include "Lexer.h"
#include "Logging.h"

#include <cctype>
#include <unordered_map>

namespace darkness{
	
	namespace{
		bool shouldSkip(unsigned char c){
			static unsigned char lastValidAsciiChar{ 127 };
			return std::isspace(c) || std::iscntrl(c) || c > lastValidAsciiChar;
		}
	}
	
	#pragma warning(suppress : 4068) //suppress unknown pragma
	#pragma clang diagnostic push
	#pragma warning(suppress : 4068) //suppress unknown pragma
	#pragma clang diagnostic ignored "-Wshadow"
	std::vector<Token> Lexer::lex(const std::string_view& input) {
		this->input = input;
		output.clear();
		nextPos = 0;
		while(!isEndOfInput()) {
			extractToken();
		}
		pushToken({ TokenType::endMarker });
		return std::move(output);
	}
	#pragma warning(suppress : 4068) //suppress unknown pragma
	#pragma clang diagnostic pop
	
	void Lexer::extractToken(){
		advance();
		
		//skip whitespaces and other stuff
		if(shouldSkip(currentChar)) {
			return;
		}
		
		switch(currentChar){
			//comments begin with a '#' character and last until the end of the line
			case '#': discardComment(); break;
			
			//strings begin with a double quote
			case '"': extractString(); break;
			
			//single char operators
			case '+': pushToken({ TokenType::plus }); break;
			case '-': pushToken({ TokenType::minus }); break;
			case '*': pushToken({ TokenType::star }); break;
			case '/': pushToken({ TokenType::fSlash }); break;
			case '.': pushToken({ TokenType::dot }); break;
			case ',': pushToken({ TokenType::comma }); break;
			case '(': pushToken({ TokenType::lParen }); break;
			case ')': pushToken({ TokenType::rParen }); break;
			case '{': pushToken({ TokenType::lCurly }); break;
			case '}': pushToken({ TokenType::rCurly }); break;
			case '[': pushToken({ TokenType::lBrace }); break;
			case ']': pushToken({ TokenType::rBrace }); break;
			case ';': pushToken({ TokenType::semicolon }); break;
			case '&': pushToken({ TokenType::ampersand }); break;
			case '|': pushToken({ TokenType::verticalBar }); break;
			
			//multi char operators
			case '!':
				pushToken({advanceIfMatch('=')
					? TokenType::bangEqual : TokenType::bang
				});
				break;
			case '=':
				pushToken({advanceIfMatch('=')
					? TokenType::dualEqual : TokenType::equal
				});
				break;
			case '<':
				pushToken({advanceIfMatch('=')
					? TokenType::lessEqual : TokenType::less
				});
				break;
			case '>':
				pushToken({advanceIfMatch('=')
					? TokenType::greaterEqual : TokenType::greater
				});
				break;
			
			//numbers, identifiers, and keywords
			default:
				if(std::isdigit(currentChar)) {
					extractNumber();
				}
				else if(std::isalpha(currentChar)){
					extractSymbol();
				}
				else{
					throw std::runtime_error{
						"Darkness lexer bad base char: "
									+ std::to_string(currentChar)
					};
				}
		}
	}
	
	void Lexer::discardComment(){
		while(currentChar != '\n' && !isEndOfInput()){
			advance();
		}
	}
	
	void Lexer::extractSymbol() {
		static std::unordered_map<std::string, TokenType> keywordMap{
			{ "if", TokenType::keyIf },
			{ "else", TokenType::keyElse },
			{ "true", TokenType::keyTrue },
			{ "false", TokenType::keyFalse },
			{ "func", TokenType::keyFunc },
			{ "while", TokenType::keyWhile },
			{ "for", TokenType::keyFor },
			{ "let", TokenType::keyLet },
			{ "return", TokenType::keyReturn },
		};
		
		std::ostringstream valueOutputStream{};
		std::string valueString;
		
		//insert the first char of the symbol
		valueOutputStream << currentChar;
		//insert the following chars of the symbol
		while(( isalnum(peek()) || peek() == '_' ) && !isEndOfInput()) {
			advance();
			valueOutputStream << currentChar;
		}
		
		valueString = valueOutputStream.str();
		auto found{ keywordMap.find(valueString) };
		
		//if we found a matching keyword, push that token
		if(found != keywordMap.end()){
			pushToken({ found->second });
		}
		//otherwise, this is an identifier
		else{
			pushToken({ TokenType::identifier, valueString });
		}
	}
	
	void Lexer::extractNumber() {
		std::ostringstream valueOutputStream{};
		std::string valueString;
		
		//insert the first char of the number
		valueOutputStream << currentChar;
		//insert following chars of the number
		while(std::isdigit(peek()) && !isEndOfInput()){
			advance();
			valueOutputStream << currentChar;
		}
		
		//if we stopped reading chars because the next char was a dot, this is a float
		if (peek() == '.') {
			advance();
			valueOutputStream << currentChar;
			//insert rest of the chars of the float
			while(std::isdigit(peek()) && !isEndOfInput()){
				advance();
				valueOutputStream << currentChar;
			}
			//push float token
			valueString = valueOutputStream.str();
			float value{ std::stof(valueString) };
			pushToken({ TokenType::floating, value });
		}
		else{
			//push int token if we stopped reading chars for any other reason
			valueString = valueOutputStream.str();
			int value{ std::stoi(valueString) };
			pushToken({ TokenType::integer, value });
		}
	}
	
	void Lexer::extractString() {
		std::ostringstream valueOutputStream{};
		//do NOT insert the first character, since it is the opening double quote
		advance();
		while(currentChar != '"' && !isEndOfInput()) {
			//if the next character is the closing double quote, we are done
			if(currentChar == '"'){
				break;
			}
			//otherwise, append to the output stream
			valueOutputStream << currentChar;
			advance();
		}
		if(currentChar != '"'){
			throw std::runtime_error{ "Darkness lexer string no close quote!" };
		}
		pushToken({ TokenType::string, valueOutputStream.str() });
	}
	
	void Lexer::pushToken(const Token& token){
		output.push_back(token);
	}
	
	void Lexer::advance(){
		currentChar = input.at(nextPos);
		++nextPos;
	}
	
	bool Lexer::advanceIfMatch(unsigned char testFor) {
		if( isEndOfInput()){
			return false;
		}
		if(input.at(nextPos) != testFor){
			return false;
		}
		++nextPos;
		return true;
	}
	
	char Lexer::peek(){
		if( isEndOfInput()){
			return '\0';
		}
		return input.at(nextPos);
	}
	
	bool Lexer::isEndOfInput() {
		return nextPos >= input.size();
	}
}