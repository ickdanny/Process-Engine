#pragma once

#include "Ast.h"

namespace darkness{
	
	template <typename... CustomTypes>
	class Interpreter{
	protected:
		using RuntimeType = std::variant<bool, int, float, std::string, CustomTypes...>;
	public:
		//Runs a darkness script. If the given AstNode is of any other type, throws an error.
		void runScript(const AstNode& script){
		
		}
	private:
		RuntimeType
}