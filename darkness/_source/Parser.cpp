#include "Parser.h"

namespace darkness{
	
	AstNode Parser::parse(const std::vector<Token>& tokens) {
		tokenPosition = 0u;
		AstNode baseNode{ AstType::list, { AstListData{} } };
		while(tokenPosition < tokens.size()){
			//todo: construct base list node
		}
	}
	
	AstNode Parser::parseStatement(const std::vector<Token>& tokens) {
	
	}
}