#include "ECS/DataStorage.h"

namespace wasp::ecs {

    //Returns an entity handle for the entity with the specified entityID of the
    //current generation. Throws runtime_error if there is no such alive entity.
    DataStorage::EntityHandle DataStorage::makeHandle(EntityID entityID) const {
        if (entityMetadataStorage.isAlive(entityID)) {
            return EntityHandle{ entityID, getMetadata(entityID).getGeneration() };
        }
        throw std::runtime_error{ "tried to make handle of dead entity!" };
    }

    //returns true if successfully removed entity, false otherwise
    bool DataStorage::removeEntity(RemoveEntityOrder removeEntityOrder) {
        if (isAlive(removeEntityOrder.entityHandle)) {
            EntityID entityID{ removeEntityOrder.entityHandle.entityID };

            componentStorage.removeEntity(
                removeEntityOrder,
                *getComponentSetPointer(entityID)
            );
            entityMetadataStorage.reclaimEntity(entityID);

            return true;
        }
        return false;
    }

    void DataStorage::setComponentSetPointer(
        EntityID entityID,
        const ComponentSet* componentSetPointer
    ) {
        if (entityMetadataStorage.isDead(entityID)) {
            throw std::runtime_error{
                "setComponentSetPointer should not be called on a dead entity : "
                + std::to_string(entityID)
            };
        }
        getMetadata(entityID).setComponentSetPointer(componentSetPointer);
    }
}