#pragma once

#include "systemInclude.h"
#include "Input/IKeyInputTable.h"

namespace process::game::systems {

	class InputParserSystem {
	private:
		//typedefs
		using IKeyInputTable = wasp::input::IKeyInputTable;
		using KeyValues = wasp::input::KeyValues;
		using KeyState = wasp::input::KeyState;

		//fields
		IKeyInputTable* keyInputTablePointer{};

	public:
		InputParserSystem(IKeyInputTable* keyInputTablePointer)
			: keyInputTablePointer { keyInputTablePointer }{
		}

		void operator()(Scene& scene);

	private:
		//helper methods
		void parseMenuInput(Scene& scene);
		void parseGameInput(Scene& scene);
		void parseDialogueInput(Scene& scene);

		bool isJustPressed(KeyValues keyValue);
		bool isBeingPressed(KeyValues keyValue);
	};
}