#include "ECS/Component/ArchetypeFactory.h"

namespace wasp::ecs::component {

    ArchetypeFactory::ArchetypeFactory(
        std::size_t initEntityCapacity,
        std::size_t initComponentCapacity,
        ComponentSetFactory& componentSetFactory
    )
        : initEntityCapacity{ initEntityCapacity }
        , initComponentCapacity{ initComponentCapacity }
    {
        componentSetFactory.setNewComponentSetCallback(
            [&](const ComponentSet& componentSet) {
                makeArchetype(componentSet);
            }
        );
    }

    void ArchetypeFactory::clear() {
        archetypePointers.clear();
    }

    void ArchetypeFactory::makeArchetype(const ComponentSet& componentSet) {
        archetypePointers.emplace_back(
            new Archetype{
                &componentSet,
                initEntityCapacity,
                initComponentCapacity
            }
        );
        componentSet.associateArchetype(archetypePointers.back());
        if (newArchetypeCallback) {
            newArchetypeCallback(archetypePointers.back());
        }
    }
}