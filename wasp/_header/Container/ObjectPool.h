#pragma once

#include <memory>
#include <stack>
#include <functional>

#include "Logging.h"

namespace wasp::container {

	//NOT threadsafe
	//NOT tested, no clue if this works or not (yet)
	template <
		typename T, 
		typename Allocator = std::allocator<T>
	>
	class ObjectPool {
	private:
		//type aliases
		using reclaimer_type = std::function<void(T)>;
		using internal_pointer_type = std::unique_ptr<T>;
		using internal_pointer_allocator_type =
			typename std::allocator_traits<Allocator>::template
			rebind_alloc<std::unique_ptr<T>>;
		using container_type = 
			std::deque<internal_pointer_type, internal_pointer_allocator_type>;
		using storage_type = std::stack<internal_pointer_type, container_type>;
	public:
		using value_type = T;
		using size_type = typename storage_type::size_type;

	private:

		//custom deleter which puts object back in pool if pool still exists
		class PoolElementDeleter {
		private:
			std::weak_ptr<ObjectPool> poolPointer{};
		public:
			explicit PoolElementDeleter(std::weak_ptr<ObjectPool> poolPointer)
				: poolPointer{ poolPointer } {
			}

			void operator()(T* pointer) {
				if (std::shared_ptr<ObjectPool> sharedPointer = poolPointer.lock()) {
					try {
						sharedPointer.get()->reclaim(std::unique_ptr<T>{pointer});
						return;
					}
					catch (...) {
						//swallow error
						debug::log("Exception caught in ObjectPool's Deleter()");
					}
				}
				std::default_delete<T>{}(pointer);
			}
		};

		//by default do not do any reclaim operations
		static void defaultReclaimFunction(T) {};

		//fields
		std::weak_ptr<ObjectPool> selfPointer{};
		storage_type storage{};
		reclaimer_type reclaimer{defaultReclaimFunction};

		//constructors
		ObjectPool() {} //private default constructor throws compile error
		ObjectPool(size_type initialSize) {
			for (size_type i{ 0 }; i < initialSize; ++i) {
				storage.emplace();
			}
		}
		ObjectPool(std::function<void(T)> reclaimer)
			: reclaimer{ reclaimer } {
		}
		ObjectPool(std::function<void(T)> reclaimer, size_type initialSize) 
			: reclaimer{ reclaimer } {
			for (size_type i{ 0 }; i < initialSize; ++i) {
				storage.emplace();
				this->reclaimer(*(storage.top().get()));
			}
		}

		//for now delete copy/assignment
		ObjectPool(const ObjectPool& other) = delete;
		ObjectPool operator=(const ObjectPool& other) = delete;

	public:

		using pointer_type = std::unique_ptr<T, PoolElementDeleter>;

		pointer_type acquire() {
			//construct new element if necessary
			if (storage.empty()) {
				storage.emplace();
			}
			//package with our deleter
			pointer_type toRet{
				storage.top().release(),
				PoolElementDeleter{ selfPointer }
			};
			storage.pop();
			return toRet;
		}

		void reclaim(std::unique_ptr<T> uniquePointer) {
			storage.push(std::move(uniquePointer));
			reclaimer(*(storage.top().get()));
		}

		bool isEmpty() const {
			return storage.empty();
		}

		size_type size() const {
			return storage.size();
		}

		//factory functions
		static std::shared_ptr<ObjectPool> makeObjectPool() {
			std::shared_ptr<ObjectPool> toRet{ new ObjectPool() };
			toRet.get()->selfPointer = toRet;
			return toRet;
		}

		static std::shared_ptr<ObjectPool> makeObjectPool(size_type initialSize) {
			std::shared_ptr<ObjectPool> toRet{ new ObjectPool(initialSize) };
			toRet.get()->selfPointer = toRet;
			return toRet;
		}

		static std::shared_ptr<ObjectPool> makeObjectPool(reclaimer_type reclaimer) {
			std::shared_ptr<ObjectPool> toRet{ new ObjectPool(reclaimer) };
			toRet.get()->selfPointer = toRet;
			return toRet;
		}

		static std::shared_ptr<ObjectPool> makeObjectPool(
			reclaimer_type reclaimer,
			size_type initialSize
		) {
			std::shared_ptr<ObjectPool> toRet{ new ObjectPool(reclaimer, initialSize) };
			toRet.get()->selfPointer = toRet;
			return toRet;
		}
	};
}