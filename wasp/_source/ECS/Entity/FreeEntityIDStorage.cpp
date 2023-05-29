#include "ECS/Entity/FreeEntityIDStorage.h"

namespace wasp::ecs::entity {

    namespace {
        constexpr float usageCapacityLimit{ 0.75f };
        constexpr float resizeRatio{ 2.0f };
    }

    FreeEntityIDStorage::FreeEntityIDStorage(std::size_t initCapacity)
        : entityIDSet(initCapacity, false)
        , currentLiveEntities{ 0 }
        , currentPos{ 0 }
    {
        if (initCapacity <= 1) {
            throw std::runtime_error{ "init capacity too small!" };
        }
    }

    void FreeEntityIDStorage::clear() {
        std::fill(entityIDSet.begin(), entityIDSet.end(), false);
        currentLiveEntities = 0;
        currentPos = 0;
    }

    bool FreeEntityIDStorage::isIDUsed(EntityID entityID) const {
        if (entityID >= entityIDSet.size()) {
            return false;
        }
        return entityIDSet[entityID];
    }

    std::size_t FreeEntityIDStorage::retrieveID() {
        if (currentPos >= entityIDSet.size()) {
            currentPos = 0;
        }
        //find a dead entity ID
        while (entityIDSet[currentPos]) {
            ++currentPos;
            if (currentPos >= entityIDSet.size()) {
                currentPos = 0;
            }
        }
        //set that entity ID to alive
        entityIDSet[currentPos] = true;

        //update our live entity count
        ++currentLiveEntities;

        //resize
        resizeIfNecessary();

        //return our entity ID, and step currentPos for efficiency
        return currentPos++;
    }

    void FreeEntityIDStorage::reclaimID(EntityID entityID) {
        //if this entity is currently in use
        if (auto ref{ entityIDSet[entityID] }) {
            //kill that entity
            ref = false;
            --currentLiveEntities;
        }
        //otherwise something went wrong
        else {
            throw std::runtime_error{ "error in reclaimID" };
        }
    }

    void FreeEntityIDStorage::resizeIfNecessary() {
        float usageCapacity{
            static_cast<float>(currentLiveEntities) / entityIDSet.size()
        };
        if (usageCapacity > usageCapacityLimit) {
            int newSize{ static_cast<int>(entityIDSet.size() * resizeRatio) };
            entityIDSet.resize(newSize, false);
        }
    }
}