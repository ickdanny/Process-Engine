#pragma once

#include "GroupFactory.h"
#include "ECS/CriticalOrders.h"
#include "ECS/Entity/EntityID.h"

namespace wasp::ecs::component {
    class ComponentStorage {
    private:
        //typedefs
        using EntityID = entity::EntityID;

        //fields
        ComponentSetFactory componentSetFactory{};
        ArchetypeFactory archetypeFactory;  //not initialized!
        GroupFactory groupFactory;          //not initialized!

    public:
        ComponentStorage(
            std::size_t initEntityCapacity,
            std::size_t initComponentCapacity
        );

        void recreate();

        template <typename... Ts>
        Group* getGroupPointer() {
            return groupFactory.getGroupPointer(componentSetFactory.makeSet<Ts...>());
        }

        template <typename T>
        T& getComponent(EntityID entityID, const ComponentSet& componentSet) {
            return componentSet.getAssociatedArchetypeWeakPointer().lock()
                ->getComponent<T>(entityID);
        }

        template <typename T>
        const T& getComponent(EntityID entityID, const ComponentSet& componentSet) 
            const 
        {
            return componentSet.getAssociatedArchetypeWeakPointer().lock()
                ->getComponent<T>(entityID);
        }

        //returns a pointer to the new component set if successful, nullptr otherwise
        template <typename T>
        const ComponentSet* addComponent(
            AddComponentOrder<T>& addComponentOrder,
            const ComponentSet& oldComponentSet
        ) {

            const ComponentSet& newComponentSet{ 
                componentSetFactory.addComponent<T>(oldComponentSet) 
            };

            if (oldComponentSet == newComponentSet) {
                return nullptr;
            }

            auto oldArchetypePointer{
                oldComponentSet.getAssociatedArchetypeWeakPointer().lock()
            };
            auto newArchetypePointer{
                newComponentSet.getAssociatedArchetypeWeakPointer().lock()
            };

            oldArchetypePointer->moveEntity(
                addComponentOrder.entityHandle.entityID,
                *newArchetypePointer
            );
            newArchetypePointer->setComponent<T>(
                addComponentOrder.entityHandle.entityID,
                addComponentOrder.component
            );

            return &newComponentSet;
        }

        //returns a pointer to the new component set
        template <typename T>
        const ComponentSet* setComponent(
            SetComponentOrder<T>& setComponentOrder,
            const ComponentSet& oldComponentSet
        ) {
            const ComponentSet& newComponentSet{
                componentSetFactory.addComponent<T>(oldComponentSet)
            };

            auto newArchetypePointer{
                newComponentSet.getAssociatedArchetypeWeakPointer().lock()
            };
            
            //only move if we are ADDING a component, not setting
            if (oldComponentSet != newComponentSet) {
                auto oldArchetypePointer{
                    oldComponentSet.getAssociatedArchetypeWeakPointer().lock()
                };
                oldArchetypePointer->moveEntity(
                    setComponentOrder.entityHandle.entityID,
                    *newArchetypePointer
                );
            }
            newArchetypePointer->setComponent(
                setComponentOrder.entityHandle.entityID,
                setComponentOrder.component
            );

            return &newComponentSet;
        }

        //returns a pointer to the new component set
        template <typename T>
        const ComponentSet* removeComponent(
            RemoveComponentOrder<T>& removeComponentOrder,
            const ComponentSet& oldComponentSet
        ) {
            const ComponentSet& newComponentSet{
                componentSetFactory.removeComponent<T>(oldComponentSet)
            };

            if (oldComponentSet != newComponentSet) {
                auto oldArchetypePointer{
                    oldComponentSet.getAssociatedArchetypeWeakPointer().lock()
                };
                auto newArchetypePointer{
                    newComponentSet.getAssociatedArchetypeWeakPointer().lock()
                };
                //moving cuts off hanging components
                oldArchetypePointer->moveEntity(
                    removeComponentOrder.entityHandle.entityID,
                    *newArchetypePointer
                );
            }

            return &newComponentSet;
        }

        //returns a pointer to the component set
        template <typename... Ts>
        const ComponentSet* addEntity(
            AddEntityOrder<Ts...> addEntityOrder,
            const EntityID entityID
        ) {
            const ComponentSet& componentSet{ componentSetFactory.makeSet<Ts...>() };
            auto archetypePointer{
                componentSet.getAssociatedArchetypeWeakPointer().lock()
            };
            std::apply(
                [&](auto& ...x) {
                    (addComponentForEntity(entityID, x, archetypePointer), ...);
                },
                addEntityOrder.components
            );

            return &componentSet;
        }

        void removeEntity(
            const RemoveEntityOrder& removeEntityOrder,
            const ComponentSet& componentSet
        );

    private:
        //helper for addEntity
        template <typename T>
        void addComponentForEntity(
            EntityID entityID,
            T& component,
            std::shared_ptr<Archetype>& archetypePointer
        ) {
            archetypePointer->setComponent(entityID, component);
        }
    };
}