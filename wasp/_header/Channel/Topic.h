#pragma once

#include <cstddef>

#include "Utility/Void.h"

namespace wasp::channel {

	struct TopicBase {
	protected:
		static std::size_t indexer;

	public:
		const std::size_t index{};

		TopicBase()
			: index{ indexer++ } {
		}
	};

	template <typename T = utility::Void>
	struct Topic : public TopicBase{
		using Void = utility::Void;
	};
}