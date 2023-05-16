#include "Input\KeyInputTable.h"

namespace wasp::input {
	
	static KeyValues getKeyValue(WPARAM wParam);
	
	void wasp::input::KeyInputTable::handleKeyDown(WPARAM wParam, LPARAM lParam) {
		dataArray[static_cast<int>(getKeyValue(wParam))] |= thisTick;
	}
	
	void wasp::input::KeyInputTable::handleKeyUp(WPARAM wParam, LPARAM lParam) {
		dataArray[static_cast<int>(getKeyValue(wParam))] &= ~thisTick;
	}
	
	void wasp::input::KeyInputTable::allKeysOff() {
		for( dataType& data : dataArray ) {
			data &= ~thisTick;
		}
	}
	
	const KeyState KeyInputTable::operator[](KeyValues key) {
		return get(key);
	}
	
	const KeyState KeyInputTable::get(KeyValues key) {
		return getKeyState(dataArray[static_cast<int>(key)]);
	}
	
	void KeyInputTable::lock(KeyValues key) {
		locks.set(static_cast<std::size_t>(key));
	}
	
	void KeyInputTable::lockAll() {
		locks.set();
	}
	
	bool KeyInputTable::isLocked(KeyValues key) {
		return locks[static_cast<int>(key)];
	}
	
	void KeyInputTable::tickOver() {
		for( dataType& data : dataArray ) {
			data = (data << 1) | (data & thisTick); //shift left but repeat last digit
		}
		locks.reset();
	}
	
	KeyState KeyInputTable::getKeyState(dataType data) {
		switch( data & lastTwoTicks ) {
			case 0:
			default: return KeyState::Up;
			case lastTwoTicks: return KeyState::Down;
			case 1: return KeyState::Press;
			case lastTick: return KeyState::Release;
		}
	}
	
	static KeyValues getKeyValue(WPARAM wParam) {
		switch( wParam ) {
			//alphabet
			case 'A': return KeyValues::k_a;
			case 'B': return KeyValues::k_b;
			case 'C': return KeyValues::k_c;
			case 'D': return KeyValues::k_d;
			case 'E': return KeyValues::k_e;
			case 'F': return KeyValues::k_f;
			case 'G': return KeyValues::k_g;
			case 'H': return KeyValues::k_h;
			case 'I': return KeyValues::k_i;
			case 'J': return KeyValues::k_j;
			case 'K': return KeyValues::k_k;
			case 'L': return KeyValues::k_l;
			case 'M': return KeyValues::k_m;
			case 'N': return KeyValues::k_n;
			case 'O': return KeyValues::k_o;
			case 'P': return KeyValues::k_p;
			case 'Q': return KeyValues::k_q;
			case 'R': return KeyValues::k_r;
			case 'S': return KeyValues::k_s;
			case 'T': return KeyValues::k_t;
			case 'U': return KeyValues::k_u;
			case 'V': return KeyValues::k_v;
			case 'W': return KeyValues::k_w;
			case 'X': return KeyValues::k_x;
			case 'Y': return KeyValues::k_y;
			case 'Z': return KeyValues::k_z;
				//numbers
			case '0': return KeyValues::k_0;
			case '1': return KeyValues::k_1;
			case '2': return KeyValues::k_2;
			case '3': return KeyValues::k_3;
			case '4': return KeyValues::k_4;
			case '5': return KeyValues::k_5;
			case '6': return KeyValues::k_6;
			case '7': return KeyValues::k_7;
			case '8': return KeyValues::k_8;
			case '9': return KeyValues::k_9;
			case VK_NUMPAD0: return KeyValues::k_numpad0;
			case VK_NUMPAD1: return KeyValues::k_numpad1;
			case VK_NUMPAD2: return KeyValues::k_numpad2;
			case VK_NUMPAD3: return KeyValues::k_numpad3;
			case VK_NUMPAD4: return KeyValues::k_numpad4;
			case VK_NUMPAD5: return KeyValues::k_numpad5;
			case VK_NUMPAD6: return KeyValues::k_numpad6;
			case VK_NUMPAD7: return KeyValues::k_numpad7;
			case VK_NUMPAD8: return KeyValues::k_numpad8;
			case VK_NUMPAD9: return KeyValues::k_numpad9;
				//arrow keys
			case VK_LEFT: return KeyValues::k_left;
			case VK_RIGHT: return KeyValues::k_right;
			case VK_UP: return KeyValues::k_up;
			case VK_DOWN: return KeyValues::k_down;
				//special
			case VK_ESCAPE: return KeyValues::k_escape;
			case VK_SPACE: return KeyValues::k_space;
			case VK_OEM_3: return KeyValues::k_backTick;
			case VK_OEM_MINUS: return KeyValues::k_minus;
			case VK_OEM_PLUS: return KeyValues::k_plus;
			case VK_OEM_5: return KeyValues::k_backSlash;
			case VK_BACK: return KeyValues::k_backSpace;
			case VK_RETURN: return KeyValues::k_enter;
			case VK_OEM_4: return KeyValues::k_openBracket;
			case VK_OEM_6: return KeyValues::k_closeBracket;
			case VK_OEM_1: return KeyValues::k_semicolon;
			case VK_OEM_7: return KeyValues::k_quote;
			case VK_OEM_COMMA: return KeyValues::k_comma;
			case VK_OEM_PERIOD: return KeyValues::k_period;
			case VK_OEM_2: return KeyValues::k_slash;
			case VK_TAB: return KeyValues::k_tab;
			case VK_SHIFT: return KeyValues::k_shift;
			case VK_CONTROL: return KeyValues::k_control;
			case VK_MENU: return KeyValues::k_alt;
			
			default: return KeyValues::undefined;
		}
	}
}