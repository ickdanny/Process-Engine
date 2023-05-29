#pragma once

#include <vector>

#include "EntityHandle.h"
#include "EntityMetadata.h"
#include "FreeEntityIDStorage.h"

namespace wasp::ecs::entity {

    class EntityMetadataStorage {
    private:

        //fields
        mutable std::vector<EntityMetadata> entityMetadataList{};
        FreeEntityIDStorage freeEntityIDStorage;    //uninitialized!

    public:
        //Constructs an EntityMetadataStorage with the specified number of
        //default initialized entity metadatas
        EntityMetadataStorage(std::size_t initCapacity);

        void clear();

        //creates an entity and returns its handle
        EntityHandle createEntity();

        void reclaimEntity(EntityID entityID);

        bool isAlive(EntityID entityID) const;

        bool isDead(EntityID entityID) const {
            return !isAlive(entityID);
        }

        bool isAlive(EntityHandle entityHandle) const;

        bool isDead(EntityHandle entityHandle) const {
            return !isAlive(entityHandle);
        }

        EntityMetadata& getMetadata(EntityID entityID);

        const EntityMetadata getMetadata(EntityID entityID) const;

    private:
        //helper functions
        void resizeIfNecessary(EntityID entityID) const;
    };
}