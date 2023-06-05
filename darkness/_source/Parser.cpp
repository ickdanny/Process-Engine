#include "Parser.h"

namespace darkness{
	
	AstNode Parser::parse(const std::vector<Token>& tokens) {
		tokenPosition = 0u;
		//todo: base of tree may NOT be list! could be cycle or routine or whatever
		AstNode baseNode{ AstType::call, { AstCallData{ "list" }}};
		while(tokenPosition < tokens.size()){
			//todo: construct base list node
		}
		return baseNode;
	}
	
	AstNode Parser::parseStatement(const std::vector<Token>& tokens) {
		return { AstType::call, { AstCallData{ "list" }}};
	}
}