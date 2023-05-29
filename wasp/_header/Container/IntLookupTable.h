#pragma once

#include <vector>
#include <string>

namespace wasp::container {

	//An IntLookupTable is an int-indexed sparse set in which the values can only be
	//accessed via iteration

	//Nomenclature: 
	//A sparse index is an index of sparseIndices which maps to a dense index. 
	//A dense index is an index of denseValues which maps to a value.

	namespace {
		constexpr int invalidIndex{ -2 };
		constexpr float sparseIndexGrowRatio{ 1.5f };
	}

	//a non-templated base class so that we can store pointers
	class IntLookupTableBase {
	public:
		virtual ~IntLookupTableBase() = default;
		virtual int size() const = 0;
		virtual bool contains(int sparseIndex) const = 0;
		virtual bool remove(int sparseIndex) = 0;
		virtual void clear() = 0;
	};

	template <typename T>
	class IntLookupTable : public IntLookupTableBase{
	private:
		std::vector<int> sparseIndices{};
		std::vector<T> denseValues{};

		//denseIndexToSparseIndex maps a dense index back to it's corresponding sparse index
		std::vector<int> denseIndexToSparseIndex{};

		int currentSize{};

	public:

		IntLookupTable()
			: IntLookupTable(0, 0) {
		}

		IntLookupTable(
			const std::size_t initialMaxIndex, 
			const std::size_t initialCapacity
		)
			: sparseIndices(initialMaxIndex)
			, denseValues{}
			, denseIndexToSparseIndex(initialCapacity)
			, currentSize{ 0 } 
		{
			denseValues.reserve(initialCapacity);
			clearSparseIndices();
			clearDenseIndexToSparseIndex();
		}

		~IntLookupTable() = default;

		int size() const override {
			return currentSize;
		}

		bool contains(int sparseIndex) const override {
			if (isInvalidSparseIndex(sparseIndex)) {
				return false;
			}
			return isValidDenseIndex(sparseIndices[sparseIndex]);
		}

		//returns true if an element was replaced, false otherwise
		bool set(int sparseIndex, const T& value) {
			if (isInvalidSparseIndex(sparseIndex)) {
				growSparseIndices(sparseIndex);
			}

			int denseIndex = sparseIndices[sparseIndex];
			if (isValidDenseIndex(denseIndex)) {
				denseValues[denseIndex] = std::move(value);
				return true;
			}
			else {
				appendToBack(sparseIndex, std::move(value));
				return false;
			}
		}

		T& get(int sparseIndex) {
			throwIfInvalidSparseIndex(sparseIndex);
			int denseIndex{ sparseIndices[sparseIndex] };

			throwIfInvalidDenseIndex(denseIndex);
			return denseValues[denseIndex];
		}

		const T& get(int sparseIndex) const {
			return get(sparseIndex);
		}

		//returns true if an element was removed, false otherwise
		bool remove(int sparseIndex) override {
			//case 1: there is no such element -> return false
			if (isInvalidSparseIndex(sparseIndex)) {
				return false;
			}
			int denseIndex{ sparseIndices[sparseIndex] };
			if (isInvalidDenseIndex(denseIndex)) {
				return false;
			}

			//case 2: there is an element -> return true
			//decrement currentSize first for the following functions to operate properly
			--currentSize;
			//if the element to remove is not the last, swap with the last element
			if (currentSize > 0 && denseIndex != currentSize) {
				overwriteWithElementAtCurrentSize(denseIndex);
			}
			//otherwise, we just removed the last element
			else {
				removeElementAtCurrentSize();
			}
			return true;
		}

		void clear() override {
			clearSparseIndices();
			clearDenseValues();
			clearDenseIndexToSparseIndex();
			currentSize = 0;
		}

		class Iterator;

		Iterator begin() {
			return Iterator{ this, 0 };
		}

		Iterator end() {
			return Iterator{ this, currentSize };
		}

		class Iterator {
			friend class IntLookupTable;
		public:
			using iterator_category = std::forward_iterator_tag;
			using difference_type = typename std::vector<T>::iterator::difference_type;
			using value_type = T;
			using pointer = T*;
			using reference = T&;

		private:
			//fields
			IntLookupTable* intLookupTablePointer{};
			typename std::vector<T>::iterator valueIterator{};
			int currentDenseIndex{};

			Iterator(IntLookupTable* intLookupTablePointer, int denseIndex)
				: intLookupTablePointer{ intLookupTablePointer }
				, valueIterator{ 
					intLookupTablePointer->denseValues.begin() + denseIndex 
				}
				, currentDenseIndex{ denseIndex } {
			}

		public:

			int getCurrentSparseIndex() {
				intLookupTablePointer->throwIfInvalidDenseIndex(currentDenseIndex);

				int sparseIndex{ 
					intLookupTablePointer->denseIndexToSparseIndex[currentDenseIndex] 
				};
				intLookupTablePointer->throwIfInvalidSparseIndex(sparseIndex);

				return sparseIndex;
			}

			//operators
			reference operator*() const { 
				return *valueIterator; 
			}
			pointer operator->() { 
				return valueIterator;
			}

			reference getReference() {
				return *valueIterator;
			}

			//prefix increment
			Iterator& operator++() { 
				++valueIterator; 
				++currentDenseIndex;
				return *this; 
			}

			//postfix increment
			Iterator operator++(int) { 
				Iterator temp{ *this };
				++(*this); 
				return temp; 
			}

			friend bool operator== (const Iterator& a, const Iterator& b) { 
				return (a.valueIterator == b.valueIterator)
					&& (a.currentDenseIndex == b.currentDenseIndex);
			};
			friend bool operator!= (const Iterator& a, const Iterator& b) { 
				return (a.valueIterator != b.valueIterator)
					|| (a.currentDenseIndex != b.currentDenseIndex);
			};
		};

	private:

		//clearing functions
		void clearSparseIndices() {
			std::fill(sparseIndices.begin(), sparseIndices.end(), invalidIndex);
		}
		void clearDenseValues() {
			denseValues.clear();
		}
		void clearDenseIndexToSparseIndex() {
			std::fill(
				denseIndexToSparseIndex.begin(), 
				denseIndexToSparseIndex.end(), 
				invalidIndex
			);
		}

		void growSparseIndices(int largestSparseIndex) {
			int newSize{ largestSparseIndex + 1 };
			newSize = static_cast<int>(newSize * sparseIndexGrowRatio);
			unsigned int growBy{ static_cast<unsigned int>(newSize) - sparseIndices.size() };
			sparseIndices.insert(sparseIndices.end(), growBy, invalidIndex);
		}

		//index checker functions
		bool isValidSparseIndex(int sparseIndex) const {
			return (sparseIndex >= 0) 
				&& (static_cast<std::vector<int>::size_type>(sparseIndex) < sparseIndices.size());
		}
		bool isInvalidSparseIndex(int sparseIndex) const {
			return !isValidSparseIndex(sparseIndex);
		}
		bool isValidDenseIndex(int denseIndex) const {
			return (denseIndex >= 0) 
				&& (static_cast<std::vector<T>::size_type>(denseIndex) < denseValues.size());
		}
		bool isInvalidDenseIndex(int denseIndex) const {
			return !isValidDenseIndex(denseIndex);
		}

		//modification functions
		void removeElementAtCurrentSize() {
			throwIfInvalidDenseIndex(currentSize);

			invalidateSparseIndex(denseIndexToSparseIndex[currentSize]);
			invalidateDenseIndexToSparseIndex(currentSize);
			removeDenseValueAtCurrentSize();
		}
		void invalidateSparseIndex(int sparseIndex) {
			throwIfInvalidSparseIndex(sparseIndex);
			sparseIndices[sparseIndex] = invalidIndex;
		}
		void invalidateDenseIndexToSparseIndex(int denseIndex) {
			denseIndexToSparseIndex[denseIndex] = invalidIndex;
		}
		void removeDenseValueAtCurrentSize() {
			if (currentSize != denseValues.size() - 1) {
				throw std::runtime_error{
					"Trying to erase element that isn't the last!" 
				};
			}
			//vector.erase will move all subsequent elements
			//therefore we only allow for erasing the final element
			denseValues.erase(denseValues.begin() + currentSize);
		}

		void overwriteWithElementAtCurrentSize(int denseIndex) {
			//A = element to overwrite, 
			//B = element at current size (last element)
			
			//move the value of B to A
			denseValues[denseIndex] = std::move(denseValues[currentSize]);
			//erase old B
			removeDenseValueAtCurrentSize();

			int sparseIndexA{ denseIndexToSparseIndex[denseIndex] };
			int sparseIndexB{ denseIndexToSparseIndex[currentSize] };

			//make the sparse index entry for B point to the new position at A
			if (isValidSparseIndex(sparseIndexB)) {
				sparseIndices[sparseIndexB] = denseIndex;
			}
			//invalidate the sparse index entry for A
			if (isValidSparseIndex(sparseIndexA)) {
				invalidateSparseIndex(sparseIndexA);
			}

			//make the dense index entry for the new B point to the new sparse index entry for B
			denseIndexToSparseIndex[denseIndex] = sparseIndexB;
			//invalidate the old value to index 
			invalidateDenseIndexToSparseIndex(currentSize);
		}

		void appendToBack(int sparseIndex, const T& value) {
			sparseIndices[sparseIndex] = currentSize;
			if (static_cast<std::vector<T>::size_type>(currentSize) 
				< denseValues.size()) 
			{
				denseValues[currentSize] = std::move(value);
			}
			else {
				denseValues.push_back(std::move(value));
			}

			if (static_cast<std::vector<int>::size_type>(currentSize) 
				< denseIndexToSparseIndex.size()) 
			{
				denseIndexToSparseIndex[currentSize] = sparseIndex;
			}
			else {
				denseIndexToSparseIndex.push_back(sparseIndex);
			}
			++currentSize;
		}

		//throw functions
		void throwIfInvalidSparseIndex(int sparseIndex) const {
			if (isInvalidSparseIndex(sparseIndex)) {
				throw std::runtime_error{ 
					"Invalid sparse index: " + std::to_string(sparseIndex) 
				};
			}
		}
		void throwIfInvalidDenseIndex(int denseIndex) const {
			if (isInvalidDenseIndex(denseIndex)) {
				throw std::runtime_error{ 
					"Invalid dense index: " + std::to_string(denseIndex) 
				};
			}
		}
	};
}