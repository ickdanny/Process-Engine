#pragma once

#include "Interpreter.h"

namespace process::game::components {
	
	template <typename... CustomTypes>
	struct ScriptContainer{
		//constants
		static constexpr int noTimer{ -1 };
		
		//fields
		std::shared_ptr<darkness::AstNode> scriptPointer{};
		std::string name{};
		typename darkness::Interpreter<CustomTypes...>::ScriptExecutionState state{};
		int timer{ noTimer };
	};
	
	template <typename... CustomTypes>
	using ScriptList = std::vector<ScriptContainer<CustomTypes...>>;
}