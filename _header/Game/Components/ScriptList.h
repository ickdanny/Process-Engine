#pragma once

#include "Interpreter.h"

namespace process::game::components {
	
	struct ScriptContainer{
		darkness::AstNode script{};
		bool runForever{};
		darkness::Interpreter<>::ScriptExecutionState state{};
		int timer{};
	};
	
	using ScriptList = std::vector<ScriptContainer>;
}