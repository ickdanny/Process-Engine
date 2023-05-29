#pragma once

#include <stdexcept>
#include <vector>

#include "Container/IntLookupTable.h"
#include "ComponentSet.h"
#include "Archetype.h"
#include "GroupIterator.h"

namespace wasp::ecs::component {
    
	class Group {
    private:
        //fields
        const ComponentSet* const componentKeyPointer{};
        std::vector<std::shared_ptr<Archetype>> archetypePointers{};
        std::vector<Group*> childGroupPointers{};

	public:
        Group() {
            throw std::runtime_error{ "should never be called; group ctor" };
        }

        //constructs a group representing all entities having the components
        //represented by the specified component key
        Group(const ComponentSet* const componentKeyPointer);

        void receiveNewArchetype(std::shared_ptr<Archetype> archetypePointer);

        bool addNewGroup(Group* groupPointer);

        const ComponentSet* getComponentKeyPointer() const {
            return componentKeyPointer;
        }

        //the reason this is templated and not the whole class is becuase there is an
        //actual use case for acquiring a group iterator without every component,
        //namely treating certain components as markers
        template <typename... Ts>
        GroupIterator<Ts...> groupIterator() {
            throwIfInvalidTypes<Ts...>();
            std::vector<std::pair<ArchetypeIterator<Ts...>, ArchetypeIterator<Ts...>>> 
                archetypeIterators{};
            for (std::shared_ptr<Archetype>& archetypePointer : archetypePointers) {
                archetypeIterators.push_back(
                    { 
                        archetypePointer->begin<Ts...>(), 
                        archetypePointer->end<Ts...>() 
                    }
                );
            }
            return GroupIterator{ archetypeIterators };
        }

    private:
        void addChildGroup(Group* childGroupPointer);

        bool componentSetFitsIntoGroup(
            const ComponentSet* const componentSetPointer
        ) const;

        template <typename... Ts>
        void throwIfInvalidTypes() {
            if (!componentKeyPointer->containsAllComponents<Ts...>()) {
                throw std::runtime_error{ "Group doesn't contain component!" };
            }
        }
	};
}