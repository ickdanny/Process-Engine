#pragma once

#include <vector>
#include <memory>
#include <functional>

#include "ComponentSet.h"
#include "ComponentIndexer.h"
#include "ArchetypeIterator.h"
#include "Container/IntLookupTable.h"
#include "ECS/Entity/EntityID.h"

namespace wasp::ecs::component {

    class Archetype {
    private:
        //friend declarations
        friend class ArchetypeFactory;

        //typedefs
        template <typename T>
        using IntLookupTable = container::IntLookupTable<T>;
        using IntLookupTableBase = container::IntLookupTableBase;
        using EntityID = entity::EntityID;

        //static fields

        //the move component vtable maps from the type index to the appropriate
        //template instantiation of move component
        static std::vector<std::function<void(const EntityID, Archetype&, Archetype&)>>
            moveComponentVTable;

        //fields
        const ComponentSet* const componentKeyPointer{};
        const std::size_t initEntityCapacity{};
        const std::size_t initComponentCapacity{};
        //using unique_ptr to point to base class
        std::vector<std::unique_ptr<IntLookupTableBase>> componentStorages;

        //constructors

        Archetype(
            const ComponentSet* const componentKeyPointer,
            std::size_t initEntityCapacity,
            std::size_t initComponentCapacity
        ) 
            : componentKeyPointer{ componentKeyPointer }
            , initEntityCapacity{ initEntityCapacity }
            , initComponentCapacity{ initComponentCapacity }
            , componentStorages(maxComponents) {
        }
        //deleting the copy constructor (since we use unique ptrs)
        Archetype(const Archetype& toCopy) = delete;

    public:

        //component access
        template <typename T>
        T& getComponent(const EntityID entityID) {
            return getComponentStorage<T>().get(entityID);
        }

        template <typename T>
        const T& getComponent(const EntityID entityID) const {
            return getComponentStorage<T>().get(entityID);
        }

        template <typename T>
        bool setComponent(const EntityID entityID, T& component) {
            //this bit of code causes the compiler to generate a moveComponent func
            static auto dummyToInstantiateMoveComponent{
                moveComponentVTable[ComponentIndexer::getIndex<T>()] =
                    moveComponent<T>
            };

            return getComponentStorage<T>().set(entityID, component);
        }
        
        void moveEntity(const EntityID entityID, Archetype& newArchetype);

        bool removeEntity(const EntityID entityID);

        //iteration
        template <typename... Ts>
        ArchetypeIterator<Ts...> begin() {
            return ArchetypeIterator<Ts...>{
                std::tuple{ getComponentStorage<Ts>().begin()... }
            };
        }
        template <typename... Ts>
        ArchetypeIterator<Ts...> end() {
            return ArchetypeIterator<Ts...>{
                std::tuple{ getComponentStorage<Ts>().end()... }
            };
        }

        const ComponentSet* getComponentKeyPointer() const {
            return componentKeyPointer;
        }

    private:

        //helper functions

        template <typename T>
        IntLookupTable<T>& getComponentStorage() {
            std::size_t typeIndex{ ComponentIndexer::getIndex<T>() };
            //initialize if necessary
            if (typeIndex >= componentStorages.size()) {
                throw std::runtime_error{
                    "type index out of bounds for component storage"
                };
            }
            if (!componentKeyPointer->containsComponent<T>()) {
                throw std::runtime_error{
                    "somehow does not contain component!"
                };
            }
            if (!componentStorages[typeIndex]) {
                componentStorages[typeIndex] = std::make_unique<IntLookupTable<T>>(
                    initEntityCapacity,
                    initComponentCapacity
                );
            }
            IntLookupTableBase& base{ *componentStorages[typeIndex] };
            return static_cast<IntLookupTable<T>&>(base);
        }

        //one move component function gets instantiated for every set component
        //and is placed in the move component v table
        template <typename T>
        static void moveComponent(
            const EntityID entityID, 
            Archetype& oldArchetype,
            Archetype& newArchetype
        ) {
            if (oldArchetype.getComponentKeyPointer()->containsComponent<T>()
                && newArchetype.getComponentKeyPointer()-> containsComponent<T>()) {
                newArchetype.setComponent(
                    entityID,
                    oldArchetype.getComponentStorage<T>().get(entityID)
                );
            }
        }
    };
}