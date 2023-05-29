#pragma once

#include <vector>
#include <stdexcept>

#include "EntityID.h"

namespace wasp::ecs::entity {

    class FreeEntityIDStorage {
    private:
        //typedefs
        using Iterator = std::vector<bool>::iterator;

        //fields
        
        //an element at index n is true if that entity ID is in use, false otherwise
        std::vector<bool> entityIDSet{};
        std::vector<bool>::size_type currentLiveEntities{};
        std::vector<bool>::size_type currentPos{};

    public:
        FreeEntityIDStorage(std::size_t initCapacity);

        void clear();

        bool isIDUsed(EntityID entityID) const;

        std::size_t retrieveID();

        void reclaimID(EntityID entityID);

    private:
        void resizeIfNecessary();
    };
}