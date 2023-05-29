#pragma once

#include <tuple>

#include "Container/IntLookupTable.h"

namespace wasp::ecs::component {

	namespace {
		//helper function for dereferencing iterator tuple
		template <typename... Ts>
		std::tuple<Ts&...> unpackTuple(
			typename container::IntLookupTable<Ts>::Iterator&... iterators
		) {
			return std::forward_as_tuple((*iterators)...);
		}
	}

	//does not actually qualify as an iterator
	template <typename... Ts>
	class ArchetypeIterator {
	private:
		//typedefs
		template<typename T>
		using InnerIteratorType = typename container::IntLookupTable<T>::Iterator;
	public:
		using ReturnType = std::tuple<Ts&...>;

	private:
		//fields
		std::tuple<InnerIteratorType<Ts>...> innerIteratorTuple{};

	public:
		ArchetypeIterator(std::tuple<InnerIteratorType<Ts>...> innerIteratorTuple)
			: innerIteratorTuple{ innerIteratorTuple } {
		}

		std::size_t getEntityID() {
			return std::get<0>(innerIteratorTuple).getCurrentSparseIndex();
		}

		ReturnType operator*() {
			return std::apply(
				unpackTuple<Ts...>,
				innerIteratorTuple
			);
		}

		//prefix increment
		ArchetypeIterator& operator++() {
			std::apply(
				[&](auto& ...x) {
					(++x, ...);
				}, 
				innerIteratorTuple
			);
			return *this;
		}

		//postfix increment
		ArchetypeIterator operator++(int) {
			ArchetypeIterator temp{ *this };
			++(*this);
			return temp;
		}

		friend bool operator== (
			const ArchetypeIterator& a, 
			const ArchetypeIterator& b
		) {
			return a.innerIteratorTuple == b.innerIteratorTuple;
		}
		friend bool operator!= (
			const ArchetypeIterator& a, 
			const ArchetypeIterator& b
		) {
			return a.innerIteratorTuple != b.innerIteratorTuple;
		}
	};
}