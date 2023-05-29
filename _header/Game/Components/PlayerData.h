#pragma once

#include "Game/Systems/ShotType.h"
#include "Game/Systems/PlayerStates.h"

namespace process::game::components {
	
	struct PlayerData {

		//fields
		systems::ShotType shotType{};
		int lives{};
		int bombs{};
		int continues{};
		int power{};

		struct PlayerStateMachine {
			systems::PlayerStates playerState{};
			int timer{};
		} stateMachine{};

		//Constructs a player data with the given parameters
		PlayerData(
			systems::ShotType shotType,
			int lives,
			int bombs,
			int continues,
			int power
		)
			: shotType{ shotType }
			, lives{ lives }
			, bombs{ bombs }
			, continues{ continues }
			, power{ power }
			, stateMachine{ systems::PlayerStates::none, 0 } {
		}

		//Default copy and move constructors
		PlayerData(const PlayerData& toCopy) = default;
		PlayerData(PlayerData&& toMove) = default;

		//Default copy and move assignment operators
		PlayerData& operator=(const PlayerData& toCopy) = default;
		PlayerData& operator=(PlayerData&& toMove) = default;
	};
}