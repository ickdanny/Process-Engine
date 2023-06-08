#include "Parser.h"
#include <stdexcept>

namespace darkness{
	
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
		//todo: declarations
	}
	
	AstNode Parser::parseStatement() {
		return parseExpressionStatement();
	}
	
	AstNode Parser::parseExpressionStatement() {
		AstNode expression{ parseExpression() };
		consumeOrThrow(
			TokenType::semicolon,
			"Darkness parser missing semicolon!"
		);
		return AstNode{
			AstType::stmtExpression,
			AstStmtExpressionData {
				std::make_unique<AstNode>(std::move(expression))
			}
		};
	}
	
	AstNode Parser::parseExpression() {
		return parseEquality();
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
		if(advanceIfMatch<TokenType::lParen>()){
			AstNode expression{ parseExpression() };
			consumeOrThrow(
				TokenType::rParen,
				"Darkness parser missing right paren!"
			);
			return expression;
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