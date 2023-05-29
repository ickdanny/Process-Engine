#pragma once

namespace wasp::game::components {

	struct PickupType {
		enum class Types {
			life,
			bomb,
			powerSmall,
			powerLarge
		} type{};

		PickupType(Types type)
			: type{ type } {
		}
	};
}