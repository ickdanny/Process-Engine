#include "ECS/Component/ComponentStorage.h"

namespace wasp::ecs::component {
    ComponentStorage::ComponentStorage(
        std::size_t initEntityCapacity,
        std::size_t initComponentCapacity
    )
        : componentSetFactory{}
        , archetypeFactory{
            initEntityCapacity,
            initComponentCapacity,
            componentSetFactory
    }
        , groupFactory{ componentSetFactory, archetypeFactory } {
    }

    void ComponentStorage::recreate() {
        componentSetFactory.clear();
        archetypeFactory.clear();
        groupFactory.recreate(componentSetFactory);
    }

    void ComponentStorage::removeEntity(
        const RemoveEntityOrder& removeEntityOrder,
        const ComponentSet& componentSet
    ) {
        auto archetypePointer{
            componentSet.getAssociatedArchetypeWeakPointer().lock()
        };
        archetypePointer->removeEntity(removeEntityOrder.entityHandle.entityID);
    }
}