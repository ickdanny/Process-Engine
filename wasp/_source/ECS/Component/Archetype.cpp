#include "ECS/Component/Archetype.h"

namespace wasp::ecs::component {

	void Archetype::moveEntity(const EntityID entityID, Archetype& newArchetype) {
        for (std::size_t typeIndex : componentKeyPointer->getPresentTypeIndices()) {
            if (typeIndex < componentStorages.size()) {
                std::unique_ptr<IntLookupTableBase>& storagePointer =
                    componentStorages[typeIndex];

                if (storagePointer) {
                    //components present in this archetype but not present in the new
                    //archetype will simply be removed
                    moveComponentVTable[typeIndex](entityID, *this, newArchetype);
                    storagePointer->remove(entityID);
                }
            }
        }
	}

    bool Archetype::removeEntity(const EntityID entityID) {
        bool wasAnyComponentRemoved{ false };
        for (std::size_t typeIndex : componentKeyPointer->getPresentTypeIndices()) {
            if (typeIndex < componentStorages.size()) {
                std::unique_ptr<IntLookupTableBase>& storagePointer =
                    componentStorages[typeIndex];

                if (storagePointer && storagePointer->remove(entityID)) {
                    wasAnyComponentRemoved = true;
                }
            }
        }
        return wasAnyComponentRemoved;
    }

    //initializing the move component function vtable
    std::vector<std::function<void(const entity::EntityID, Archetype&, Archetype&)>>
        Archetype::moveComponentVTable{ maxComponents };
}