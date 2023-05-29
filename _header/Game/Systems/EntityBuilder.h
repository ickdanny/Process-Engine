#pragma once

#include <tuple>
#include <type_traits>

#include "ECS/CriticalOrders.h"
#include "Game/Components.h"

namespace process::game::systems {

	//utility for creating entities

	//utility type for testing whether or not a list of types contains a type
	template <typename T, typename... Ts>
	constexpr auto contains() {
		return !std::is_same<
			std::integer_sequence<bool, false, std::is_same<T, Ts>::value...>,
			std::integer_sequence<bool, std::is_same<T, Ts>::value..., false>
		>::value;
	}

	//Base struct
	struct ComponentTupleBase {
	private:
		//typedefs
		using EntityHandle = wasp::ecs::entity::EntityHandle;

	public:
		//Adds the entity represented by this object to the given data storage.
		virtual EntityHandle addTo(wasp::ecs::DataStorage& dataStorage) const = 0;

		//returns a shared pointer to a clone of this object made on the heap
		virtual std::shared_ptr<ComponentTupleBase> heapClone() const = 0;

		//Special function for adding position to a tuple
		virtual std::shared_ptr<ComponentTupleBase> addPosition(
			const Position& position
		) const = 0;

		//Special function for adding velocity to a tuple
		virtual std::shared_ptr<ComponentTupleBase> addVelocity(
			const Velocity& velocity
		) const = 0;

		//Special function for adding position AND velocity to a tuple
		virtual std::shared_ptr<ComponentTupleBase> addPosVel(
			const Position& position,
			const Velocity& velocity
		) const = 0;
	};

	//ComponentTuple struct
	template <typename... Ts>
	struct ComponentTuple : public std::tuple<Ts...>, ComponentTupleBase {
		ComponentTuple(const Ts&... args)
			: std::tuple<Ts...>{ args... } {
		}

		ComponentTuple(std::tuple<Ts...> tuple)
			: std::tuple<Ts...>(tuple) {
		}

		//conversion to AddEntityOrder
		wasp::ecs::AddEntityOrder<Ts...> package() const {
			return wasp::ecs::AddEntityOrder{ *this };
		}

		//override base functions
		wasp::ecs::entity::EntityHandle addTo(wasp::ecs::DataStorage& dataStorage) const override {
			return dataStorage.addEntity(package());
		}

		std::shared_ptr<ComponentTupleBase> heapClone() const override {
			ComponentTuple<Ts...>* rawPointer{
				new ComponentTuple<Ts...>(*this)
			};
			return std::shared_ptr<ComponentTupleBase>(rawPointer);
		}

		std::shared_ptr<ComponentTupleBase> addPosition(
			const Position& position
		) const override {
			if constexpr (contains<Position, Ts...>()) {
				throw std::runtime_error{ "add position template wrong somewhere " };
			}
			else {
				ComponentTupleBase* newTuplePointer{
					static_cast<ComponentTupleBase*>(new auto (*this + position))
				};
				return std::shared_ptr<ComponentTupleBase>{
					newTuplePointer
				};
			}
		}

		std::shared_ptr<ComponentTupleBase> addVelocity(
			const Velocity& velocity
		) const override {
			if constexpr (contains<Velocity, Ts...>()) {
				throw std::runtime_error{ "add velocity template wrong somewhere " };
			}
			else {
				ComponentTupleBase* newTuplePointer{
					static_cast<ComponentTupleBase*>(new auto (*this + velocity))
				};
				return std::shared_ptr<ComponentTupleBase>{
					newTuplePointer
				};
			}
		}

		std::shared_ptr<ComponentTupleBase> addPosVel(
			const Position& position,
			const Velocity& velocity
		) const override {
			if constexpr (contains<Position, Ts...>()) {
				throw std::runtime_error{ "add position template wrong somewhere " };
			}
			else if constexpr (contains<Velocity, Ts...>()) {
				throw std::runtime_error{ "add velocity template wrong somewhere " };
			}
			else {
				ComponentTupleBase* newTuplePointer{
					static_cast<ComponentTupleBase*>(
						new auto ((*this + position) + velocity)
					)
				};
				return std::shared_ptr<ComponentTupleBase>{
					newTuplePointer
				};
			}
		}
	};

	//adding anything to the tuple
	template <typename... Ts, typename C>
	ComponentTuple<Ts..., C> operator+(
		const ComponentTuple<Ts...>& tuple, 
		const C& component
	) {
		return std::tuple_cat(
			static_cast<std::tuple<Ts...>>(tuple), 
			std::make_tuple(component)
		);
	}

	//concatenating two tuples
	template <typename... Ts, typename... Us>
	ComponentTuple<Ts..., Us...> operator+(
		const ComponentTuple<Ts...>& left,
		const ComponentTuple<Us...>& right
	) {
		return std::tuple_cat(
			static_cast<std::tuple<Ts...>>(left), 
			static_cast<std::tuple<Us...>>(right)
		);
	}

	struct EntityBuilder {
	private:
		//typedefs
		using Point2 = wasp::math::Point2;
		using AABB = wasp::math::AABB;

	public:
		template <typename... Ts>
		static auto makeEntity(const Ts&... args) {
			return ComponentTuple{ args... };
		}

		template <typename... Ts>
		static auto makePosition(const Point2& pos, const Ts&... args) {
			return ComponentTuple{ Position{ pos }, args... };
		}

		template <typename... Ts>
		static auto makeVisible(const Point2& pos, const Ts&... args) {
			return ComponentTuple{ Position{ pos }, VisibleMarker{}, args... };
		}

		template <typename... Ts>
		static auto makeStationaryCollidable(
			const Point2& pos,
			const AABB& hitbox,
			const Ts&... args
		) {
			return ComponentTuple{
				Position{ pos },
				VisibleMarker{},
				Hitbox{ hitbox },
				CollidableMarker{},
				args...
			};
		}

		template <typename... Ts>
		static auto makeStationaryUncollidable(
			const Point2& pos,
			const AABB& hitbox,
			const Ts&... args
		) {
			return ComponentTuple{
				Position{ pos },
				VisibleMarker{},
				Hitbox{ hitbox }
			};
		}

		template <typename... Ts>
		static auto makeLinearCollidable(
			const Point2& pos,
			const Velocity& velocity,
			const AABB& hitbox,
			const Ts&... args
		) {
			return ComponentTuple{
				Position{ pos },
				Velocity{ velocity },
				VisibleMarker{},
				Hitbox{ hitbox },
				CollidableMarker{},
				args...
			};
		}

		template <typename... Ts>
		static auto makeLinearUncollidable(
			const Point2& pos,
			const Velocity& velocity,
			const AABB& hitbox,
			const Ts&... args
		) {
			return ComponentTuple{
				Position{ pos },
				Velocity{ velocity },
				VisibleMarker{},
				Hitbox{ hitbox },
				args...
			};
		}

		template <typename... Ts>
		static auto makePosPrototype(
			const Velocity& velocity,
			const AABB& hitbox,
			const Ts&... args
		) {
			return ComponentTuple{
				Velocity{ velocity },
				VisibleMarker{},
				Hitbox{ hitbox },
				CollidableMarker{},
				args...
			};
		}

		template <typename... Ts>
		static auto makePosVelPrototype(
			const AABB& hitbox,
			const Ts&... args
		) {
			return ComponentTuple{
				VisibleMarker{},
				Hitbox{ hitbox },
				CollidableMarker{},
				args...
			};
		}
	};
}