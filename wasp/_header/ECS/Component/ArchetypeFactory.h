#pragma once

#include "Archetype.h"
#include "ComponentSetFactory.h"

namespace wasp::ecs::component {

    class ArchetypeFactory {

    private:
        std::size_t initEntityCapacity{};
        std::size_t initComponentCapacity{};

        //throwing around raw pointers to elements in a vector is a HORRIBLE idea,
        //therefore we use shared_ptr
        std::vector<std::shared_ptr<Archetype>> archetypePointers{};

        std::function<void(std::shared_ptr<Archetype>)> newArchetypeCallback{};

    public:
        ArchetypeFactory(
            std::size_t initEntityCapacity,
            std::size_t initComponentCapacity,
            ComponentSetFactory& componentSetFactory
        );

        void clear();

        //setting callbacks
        void setNewArchetypeCallback(
            const std::function<void(std::shared_ptr<Archetype>)>& newArchetypeCallback
        ) {
            this->newArchetypeCallback = newArchetypeCallback;
        }

    private:
        //gets callbacked by the component set factory
        void makeArchetype(const ComponentSet& componentSet);
    };
}