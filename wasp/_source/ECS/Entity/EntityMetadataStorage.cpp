#include "ECS/Entity/EntityMetadataStorage.h"

namespace wasp::ecs::entity {

    namespace {
        constexpr float resizeRatio{ 1.5f };
    }

    //Constructs an EntityMetadataStorage with the specified number of
    //default initialized entity metadatas
    EntityMetadataStorage::EntityMetadataStorage(std::size_t initCapacity)
        : freeEntityIDStorage{ initCapacity }
    {
        entityMetadataList.resize(initCapacity);
    }

    void EntityMetadataStorage::clear() {
        std::fill(
            entityMetadataList.begin(), 
            entityMetadataList.end(), 
            EntityMetadata{}
        );
        freeEntityIDStorage.clear();
    }

    EntityHandle EntityMetadataStorage::createEntity() {
        EntityID entityID{ freeEntityIDStorage.retrieveID() };
        resizeIfNecessary(entityID);
        auto generation{ entityMetadataList[entityID].getGeneration() };
        return { entityID, generation };
    }

    void EntityMetadataStorage::reclaimEntity(EntityID entityID) {
        entityMetadataList[entityID].newGeneration();
        freeEntityIDStorage.reclaimID(entityID);
    }

    bool EntityMetadataStorage::isAlive(EntityID entityID) const {
        return freeEntityIDStorage.isIDUsed(entityID);
    }

    bool EntityMetadataStorage::isAlive(EntityHandle entityHandle) const {
        //check if this entity ID is alive and the generation matches
        return isAlive(entityHandle.entityID)
            && (getMetadata(entityHandle.entityID).getGeneration()
                == entityHandle.generation);
    }

    EntityMetadata& EntityMetadataStorage::getMetadata(EntityID entityID) {
        resizeIfNecessary(entityID);
        return entityMetadataList[entityID];
    }

    const EntityMetadata EntityMetadataStorage::getMetadata(EntityID entityID) const {
        resizeIfNecessary(entityID);
        return entityMetadataList[entityID];
    }

    void EntityMetadataStorage::resizeIfNecessary(EntityID entityID) const {
        if (entityID >= entityMetadataList.size()) {
            entityMetadataList.resize(
                static_cast<EntityID>(entityID * resizeRatio)
            );
        }
    }
}