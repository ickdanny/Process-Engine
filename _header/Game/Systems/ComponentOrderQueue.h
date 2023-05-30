#pragma once

#include "ECS/DataStorage.h"

namespace process::game::systems {

	//queues up add, set, and remove component orders, as well as removeEntity
	class ComponentOrderQueue {
	private:
		//typedefs
		using EntityHandle = wasp::ecs::entity::EntityHandle;

		//inner types
		struct QueuedOrderBase {
			virtual ~QueuedOrderBase() = default;
			virtual void apply(wasp::ecs::DataStorage& dataStorage) = 0;
		};

		template <typename T>
		struct QueuedAddComponentOrder : QueuedOrderBase {
			wasp::ecs::AddComponentOrder<T> addComponentOrder;

			QueuedAddComponentOrder(
				const EntityHandle& entityHandle,
				const T& component
			)
				: addComponentOrder{ entityHandle, component } {
			}

			~QueuedAddComponentOrder() override = default;

			void apply(wasp::ecs::DataStorage& dataStorage) override {
				dataStorage.addComponent(addComponentOrder);
			}
		};

		template <typename T>
		struct QueuedSetComponentOrder : QueuedOrderBase {
			wasp::ecs::SetComponentOrder<T> setComponentOrder;

			QueuedSetComponentOrder(
				const EntityHandle& entityHandle,
				const T& component
			)
				: setComponentOrder{ entityHandle, component } {
			}

			~QueuedSetComponentOrder() override = default;

			void apply(wasp::ecs::DataStorage& dataStorage) override {
				dataStorage.setComponent(setComponentOrder);
			}
		};

		template <typename T>
		struct QueuedRemoveComponentOrder : QueuedOrderBase {
			wasp::ecs::RemoveComponentOrder<T> removeComponentOrder;

			QueuedRemoveComponentOrder(const EntityHandle& entityHandle)
				: removeComponentOrder{ entityHandle } {
			}

			~QueuedRemoveComponentOrder() override = default;

			void apply(wasp::ecs::DataStorage& dataStorage) override {
				dataStorage.removeComponent(removeComponentOrder);
			}
		};

		struct QueuedRemoveEntityOrder : QueuedOrderBase {
			wasp::ecs::RemoveEntityOrder removeEntityOrder;

			QueuedRemoveEntityOrder(const EntityHandle& entityHandle)
				: removeEntityOrder{ entityHandle } {
			}

			~QueuedRemoveEntityOrder() override = default;

			void apply(wasp::ecs::DataStorage& dataStorage) override {
				dataStorage.removeEntity(removeEntityOrder);
			}
		};

		//fields
		std::vector<std::unique_ptr<QueuedOrderBase>> queuedOrders{};

	public:

		template <typename T>
		void queueAddComponent(EntityHandle entityHandle, const T& component) {
			queuedOrders.emplace_back(
				std::move(
					std::unique_ptr<QueuedOrderBase>{
						new QueuedAddComponentOrder{ {entityHandle, component} }
					}
				)
			);
		}

		template <typename T>
		void queueSetComponent(EntityHandle entityHandle, const T& component) {
			queuedOrders.emplace_back(
				std::move(
					std::unique_ptr<QueuedOrderBase>{
						new QueuedSetComponentOrder<T>{ entityHandle, component }
					}
				)
			);
		}

		template <typename T>
		void queueRemoveComponent(EntityHandle entityHandle) {
			queuedOrders.emplace_back(
				std::move(
					std::unique_ptr<QueuedOrderBase>{
						new QueuedRemoveComponentOrder<T>{ entityHandle }
					}
				)
			);
		}

		void queueRemoveEntity(EntityHandle entityHandle) {
			queuedOrders.emplace_back(
				std::move(
					std::unique_ptr<QueuedOrderBase>{
						new QueuedRemoveEntityOrder{ entityHandle }
					}
				)
			);
		}

		void apply(wasp::ecs::DataStorage& dataStorage) {
			for (auto& order : queuedOrders) {
				order->apply(dataStorage);
			}
			queuedOrders.clear();
		}
	};
}