#include "ECS/Component/ComponentSetFactory.h"

#include "Logging.h"

namespace wasp::ecs::component {
    const ComponentSet& ComponentSetFactory::getCanonicalSetAndBroadcastIfNew(
        const ComponentSet& componentSet
    ) {
        const auto [iterator, isNew] = canonicalComponentSets.insert(componentSet);
        const ComponentSet& toRet{ *iterator };
        if (isNew) {
            newComponentSetCallback(toRet);
        }
        return toRet;
    }
}