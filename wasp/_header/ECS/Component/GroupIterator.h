#pragma once

#include <vector>

#include "ArchetypeIterator.h"

namespace wasp::ecs::component {

	template <typename... Ts>
	class GroupIterator {
	private:
		//typedefs
		using InnerIteratorType = ArchetypeIterator<Ts...>;
		using InnerIteratorVectorType = 
			std::vector<std::pair<InnerIteratorType, InnerIteratorType>>;
	public:
		using ReturnType = typename InnerIteratorType::ReturnType;

	private:
		//fields
		std::size_t currentIteratorPairIndex{};
		//held in pairs of current/end iterators
		InnerIteratorVectorType innerIterators;

	public:
		GroupIterator(InnerIteratorVectorType innerIterators)
			: currentIteratorPairIndex{ 0 }
			, innerIterators { innerIterators } 
		{
			skipToNextValidArchetype();
		}

		void skipToNextValidArchetype() {
			while (isValid() && (getCurrentIterator() == getCurrentEndIterator())) {
				++currentIteratorPairIndex;
			}
		}

		std::size_t getEntityID() {
			return getCurrentIterator().getEntityID();
		}

		ReturnType operator*() {
			return *getCurrentIterator();
		}

		bool isValid() {
			return currentIteratorPairIndex < innerIterators.size();
		}

		//prefix increment
		GroupIterator& operator++() {
			++getCurrentIterator();
			skipToNextValidArchetype();
			return *this;
		}

		//postfix increment
		GroupIterator operator++(int) {
			GroupIterator temp{ *this };
			++(*this);
			return temp;
		}

		//no point in equality operators because we use isValid instead

	private:
		InnerIteratorType& getCurrentIterator() {
			return std::get<0>(innerIterators[currentIteratorPairIndex]);
		}

		InnerIteratorType& getCurrentEndIterator() {
			return std::get<1>(innerIterators[currentIteratorPairIndex]);
		}
	};
}