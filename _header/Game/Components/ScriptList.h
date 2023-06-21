#pragma once

#include "Interpreter.h"

namespace process::game::components {
	
	struct ScriptContainer{
		//constants
		static constexpr int noTimer{ -1 };
		
		//fields
		std::shared_ptr<darkness::AstNode> scriptPointer{};
		std::string name{};
		bool runForever{ false };
		darkness::Interpreter<wasp::math::Vector2>::ScriptExecutionState state{};
		int timer{ noTimer };
	};
	
	using ScriptList = std::vector<ScriptContainer>;
}