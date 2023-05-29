#include "ECS/Entity/EntityHandle.h"

namespace wasp::ecs::entity {
    bool operator==(const EntityHandle& a, const EntityHandle& b) {
        return (a.entityID == b.entityID) && (a.generation == b.generation);
    }

    bool operator!= (const EntityHandle& a, const EntityHandle& b) {
        return (a.entityID != b.entityID) || (a.generation != b.generation);
    }

    EntityHandle::operator std::string() const {
        return "E_" + std::to_string(entityID) + "G_" + std::to_string(generation);
    }
}