#include "Parser.h"
#include <stdexcept>

namespace darkness{
	//this is a recursive descent parser
	
	#pragma warning(suppress : 4068) //suppress unknown pragma
	#pragma clang diagnostic push
	#pragma warning(suppress : 4068) //suppress unknown pragma
	#pragma clang diagnostic ignored "-Wshadow"
	AstNode Parser::parse(const std::vector<Token>& input) {
		this->input = input;
		nextPos = 0u;
		return parseExpression();
	}
	#pragma warning(suppress : 4068) //suppress unknown pragma
	#pragma clang diagnostic pop
	
	AstNode Parser::parseDeclaration(){
		if(advanceIfMatch<TokenType::keyLet>()){
			return parseVariableDeclaration();
		}
		return parseStatement();
	}
	
	AstNode Parser::parseVariableDeclaration() {
		Token& token{
			consumeOrThrow(TokenType::identifier, "need var name")
		};
		std::string name{ std::get<std::string>(token.value) };
		auto initializerPointer{
			advanceIfMatch<TokenType::equal>()
			    ? std::make_unique<AstNode>(parseExpression())
		        : nullptr
		};
		consumeOrThrow(
			TokenType::semicolon,
			"Darkness parser missing semicolon!"
		);
		return {
			AstType::stmtVarDeclare,
			AstStmtVarDeclareData{
				name,
				std::move(initializerPointer)
			}
		};
	}
	
	AstNode Parser::parseStatement() {
		if(advanceIfMatch<TokenType::keyIf>()){
			return parseIf();
		}
		if(advanceIfMatch<TokenType::keyWhile>()){
			return parseWhile();
		}
		if(advanceIfMatch<TokenType::keyFor>()){
			return parseFor();
		}
		if(advanceIfMatch<TokenType::lBrace>()){
			return parseBlock();
		}
		return parseExpressionStatement();
	}
	
	AstNode Parser::parseIf(){
		consumeOrThrow(
			TokenType::lParen,
			"Darkness parser missing '('!"
		);
		AstNode condition{ parseExpression() };
		consumeOrThrow(
			TokenType::rParen,
			"Darkness parser missing ')'!"
		);
		AstNode trueBranch{ parseStatement() };
		std::unique_ptr<AstNode> falseBranch{ nullptr };
		if(advanceIfMatch<TokenType::keyElse>()){
			falseBranch = std::make_unique<AstNode>(parseStatement());
		}
		return{
			AstType::stmtIf,
			AstStmtIfData{
				std::make_unique<AstNode>(std::move(condition)),
				std::make_unique<AstNode>(std::move(trueBranch)),
				std::move(falseBranch)
			}
		};
	}
	
	AstNode Parser::parseWhile(){
		consumeOrThrow(
			TokenType::lParen,
			"Darkness parser missing '('!"
		);
		AstNode condition{ parseExpression() };
		consumeOrThrow(
			TokenType::rParen,
			"Darkness parser missing ')'!"
		);
		AstNode body{ parseStatement() };
		return{
			AstType::stmtWhile,
			AstStmtWhileData{
				std::make_unique<AstNode>(std::move(condition)),
				std::make_unique<AstNode>(std::move(body))
			}
		};
	}
	
	AstNode Parser::parseFor(){
		consumeOrThrow(
			TokenType::lParen,
			"Darkness parser missing '('!"
		);
		
		std::unique_ptr<AstNode> initializer{ nullptr };
		if(advanceIfMatch<TokenType::semicolon>()){
			//do nothing
		}
		else if(advanceIfMatch<TokenType::keyLet>()){
			initializer = std::make_unique<AstNode>(parseVariableDeclaration());
		}
		else{
			initializer = std::make_unique<AstNode>(parseExpressionStatement());
		}
		
		//todo: instead of nullptr unique ptrs, just use error code stuff
		
		std::unique_ptr<AstNode> condition{ nullptr };
		if(!match(TokenType::semicolon)){
			condition = std::make_unique<AstNode>(parseExpression());
		}
		consumeOrThrow(
			TokenType::semicolon,
			"Darkness parser missing ';'!"
		);
		
		std::unique_ptr<AstNode> increment{ nullptr };
		if(!match(TokenType::rParen)){
			increment = std::make_unique<AstNode>(parseExpression());
		}
		consumeOrThrow(
			TokenType::rParen,
			"Darkness parser missing ')'!"
		);
		
		AstNode body{ parseStatement() };
		
		if(increment){
			body = {
				AstType::stmtBlock,
				AstStmtBlockData{ {
					std::move(body),
					std::move(*increment.release())
				} }
			};
		}
	}
	
	AstNode Parser::parseBlock(){
		AstNode blockNode{
			AstType::stmtBlock,
			AstStmtBlockData{}
		};
		auto& statements{
			std::get<AstStmtBlockData>(blockNode.dataVariant).statements
		};
		while(!match(TokenType::rBrace) && !isEndOfInput()){
			statements.push_back(parseDeclaration());
		}
		consumeOrThrow(
			TokenType::rBrace,
			"Darkness parser missing right brace!"
		);
		return blockNode;
	}
	
	
	
	AstNode Parser::parseExpressionStatement() {
		AstNode expression{ parseExpression() };
		consumeOrThrow(
			TokenType::semicolon,
			"Darkness parser missing semicolon!"
		);
		return {
			AstType::stmtExpression,
			AstStmtExpressionData {
				std::make_unique<AstNode>(std::move(expression))
			}
		};
	}
	
	AstNode Parser::parseExpression() {
		return parseAssignment();
	}
	
	AstNode Parser::parseAssignment() {
		AstNode left{ parseOr() };
		if(advanceIfMatch<TokenType::equal>()){
			AstNode right{ parseAssignment() };
			//the left operand must be a variable name (l-value)
			if(left.type == AstType::variable){
				std::string& varName{
					std::get<AstVariableData>(left.dataVariant).varName
				};
				return {
					AstType::binAssign,
					AstAssignData{
						varName,
						std::make_unique<AstNode>(std::move(right))
					}
				};
			}
			else{
				throw std::runtime_error{
					"trying to assign to a non-variable!"
				};
			}
		}
		//if this was not an assignment
		return left;
	}
	
	AstNode Parser::parseOr(){
		AstNode left{ parseAnd() };
		while( advanceIfMatch<TokenType::verticalBar>() ){
			AstNode right{ parseAnd() };
			left = AstNode{
				AstType::binVerticalBar,
				AstBinData{
					std::make_unique<AstNode>(std::move(left)),
					std::make_unique<AstNode>(std::move(right))
				}
			};
		}
		return left;
	}
	
	AstNode Parser::parseAnd(){
		AstNode left{ parseEquality() };
		while( advanceIfMatch<TokenType::ampersand>() ){
			AstNode right{ parseEquality() };
			left = AstNode{
				AstType::binAmpersand,
				AstBinData{
					std::make_unique<AstNode>(std::move(left)),
					std::make_unique<AstNode>(std::move(right))
				}
			};
		}
		return left;
	}
	
	AstNode Parser::parseEquality() {
		AstNode left{ parseComparison() };
		while( advanceIfMatch<TokenType::dualEqual, TokenType::bangEqual>() ){
			AstType astType{
				previous().type == TokenType::dualEqual
					? AstType::binDualEqual : AstType::binBangEqual
			};
			AstNode right{ parseComparison() };
			left = AstNode{
				astType,
				AstBinData{
					std::make_unique<AstNode>(std::move(left)),
					std::make_unique<AstNode>(std::move(right))
				}
			};
		}
		return left;
	}
	
	AstNode Parser::parseComparison() {
		AstNode left{ parseTerm() };
		while( advanceIfMatch<
			TokenType::greater,
			TokenType::greaterEqual,
			TokenType::less,
			TokenType::lessEqual
		>() ){
			AstType astType;
			switch(previous().type){
				case TokenType::greater:
					astType = AstType::binGreater; break;
				case TokenType::greaterEqual:
					astType = AstType::binGreaterEqual; break;
				case TokenType::less:
					astType = AstType::binLess; break;
				case TokenType::lessEqual:
					astType = AstType::binLessEqual; break;
				default:
					throw std::runtime_error{
						"Darkness parser bad TokenType in parseComparison()!"
					};
			}
			AstNode right{ parseTerm() };
			left = AstNode{
				astType,
				AstBinData{
					std::make_unique<AstNode>(std::move(left)),
					std::make_unique<AstNode>(std::move(right))
				}
			};
		}
		return left;
	}
	
	AstNode Parser::parseTerm() {
		AstNode left{ parseFactor() };
		while( advanceIfMatch<TokenType::minus, TokenType::plus>() ){
			AstType astType{
				previous().type == TokenType::minus ? AstType::binMinus : AstType::binPlus
			};
			AstNode right{ parseFactor() };
			left = AstNode{
				astType,
				AstBinData{
					std::make_unique<AstNode>(std::move(left)),
					std::make_unique<AstNode>(std::move(right))
				}
			};
		}
		return left;
	}
	
	AstNode Parser::parseFactor() {
		AstNode left{ parseUnary() };
		while( advanceIfMatch<TokenType::star, TokenType::fSlash>() ){
			AstType astType{
				previous().type == TokenType::star
					? AstType::binStar : AstType::binForwardSlash
			};
			AstNode right{ parseUnary() };
			left = AstNode{
				astType,
				AstBinData{
					std::make_unique<AstNode>(std::move(left)),
					std::make_unique<AstNode>(std::move(right))
				}
			};
		}
		return left;
	}
	
	AstNode Parser::parseUnary() {
		if(advanceIfMatch<TokenType::bang, TokenType::plus, TokenType::minus>()){
			AstType astType;
			switch(previous().type){
				case TokenType::bang:
					astType = AstType::unaryBang; break;
				case TokenType::plus:
					astType = AstType::unaryPlus; break;
				case TokenType::minus:
					astType = AstType::unaryMinus; break;
				default:
					throw std::runtime_error{
						"Darkness parser bad TokenType in parseUnary()!"
					};
			}
			AstNode right{ parseUnary() };
			return {
				astType,
				AstUnaryData{
					std::make_unique<AstNode>(std::move(right))
				}
			};
		}
		return parsePrimary();
	}
	
	AstNode Parser::parsePrimary() {
		if(advanceIfMatch<TokenType::keyFalse>()){
			return{ AstType::litBool, AstLitBoolData{ false } };
		}
		if(advanceIfMatch<TokenType::keyTrue>()){
			return{ AstType::litBool, AstLitBoolData{ true } };
		}
		if(advanceIfMatch<TokenType::integer>()){
			return{
				AstType::litInt,
				AstLitIntData{ std::get<int>(previous().value) }
			};
		}
		if(advanceIfMatch<TokenType::floating>()){
			return{
				AstType::litFloat,
				AstLitFloatData{ std::get<float>(previous().value) }
			};
		}
		if(advanceIfMatch<TokenType::string>()){
			return{
				AstType::litString,
				AstLitStringData{ std::get<std::string>(previous().value) }
			};
		}
		if(advanceIfMatch<TokenType::identifier>()){
			return{
				AstType::variable,
				AstVariableData{
					std::get<std::string>(previous().value)
				}
			};
		}
		if(advanceIfMatch<TokenType::lParen>()){
			AstNode expression{ parseExpression() };
			consumeOrThrow(
				TokenType::rParen,
				"Darkness parser missing right paren!"
			);
			//needed to distinguish l and r values
			return {
				AstType::parenths,
				AstParenthsData{
					std::make_unique<AstNode>(std::move(expression))
				}
			};
		}
		throw std::runtime_error{ "Darkness parser expecting expression!" };
	}
	
	bool Parser::match(TokenType type){
		if(isEndOfInput()){
			return false;
		}
		return peek().type == type;
	}
	
	Token& Parser::peek(){
		return input.at(nextPos);
	}
	
	Token& Parser::previous(){
		return input.at(nextPos - 1);
	}
	
	Token& Parser::advance(){
		if(!isEndOfInput()){
			++nextPos;
		}
		return previous();
	}
	
	Token& Parser::consumeOrThrow(TokenType expectedToken, const std::string& throwMsg){
		if(match(expectedToken)){
			return advance();
		}
		throw std::runtime_error{ throwMsg };
	}
	
	bool Parser::isEndOfInput() {
		return peek().type == TokenType::endMarker;
	}
}