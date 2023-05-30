#include "Game/Systems/OutboundSystem.h"

#include "Logging.h"

namespace process::game::systems {

    void OutboundSystem::operator()(Scene& scene) {
        //get the group iterator for Position and Outbound
        static const Topic<Group*> groupPointerStorageTopic{};
        auto groupPointer{
            getGroupPointer<Position, Outbound>(
                scene,
                groupPointerStorageTopic
            )
        };
        auto groupIterator{ groupPointer->groupIterator<Position, Outbound>() };

        auto& dataStorage{ scene.getDataStorage() };

        //remove all entities out of bounds
        std::vector<wasp::ecs::RemoveEntityOrder> removeEntityOrders{};
        while (groupIterator.isValid()) {
            auto [position, outbound] = *groupIterator;
            if (isOutOfBounds(position, outbound.bound)) {
                wasp::ecs::entity::EntityHandle entityHandle{
                    dataStorage.makeHandle(groupIterator.getEntityID())
                };
                removeEntityOrders.push_back({ entityHandle });
            }
            ++groupIterator;
        }
        for (auto& removeEntityOrder : removeEntityOrders) {
            dataStorage.removeEntity(removeEntityOrder);
        }
    }
}