#include "ECS/Component/GroupFactory.h"

namespace wasp::ecs::component {
    GroupFactory::GroupFactory(
        ComponentSetFactory& componentSetFactory,
        ArchetypeFactory& archetypeFactory
    ) {
        initGroups(componentSetFactory);
        archetypeFactory.setNewArchetypeCallback(
            [&](std::shared_ptr<Archetype> archetypePointer) {
                if (zeroGroupPointer) {
                    zeroGroupPointer->receiveNewArchetype(archetypePointer);
                }
            }
        );
    }

    void GroupFactory::recreate(ComponentSetFactory& componentSetFactory) {
        keyToGroupMap.clear();
        zeroGroupPointer = nullptr;
        initGroups(componentSetFactory);
    }

    Group* GroupFactory::getGroupPointer(const ComponentSet& componentKey) {
        auto found{ keyToGroupMap.find(componentKey) };
        //create a new group if necessary
        if (found == keyToGroupMap.end()) {
            keyToGroupMap.emplace(componentKey, &componentKey);
            Group* newGroupPointer{ &keyToGroupMap[componentKey] };
            if (!zeroGroupPointer->addNewGroup(newGroupPointer)) {
                throw std::runtime_error{ "error in getGroupPointer" };
            }
            return newGroupPointer;
        }
        //otherwise return our found group
        else {
            return &(found->second);
        }
    }

    void GroupFactory::initGroups(ComponentSetFactory& componentSetFactory) {
        const auto& zeroSet{ componentSetFactory.makeSet() };
        keyToGroupMap.emplace(zeroSet, &zeroSet);
        zeroGroupPointer = &keyToGroupMap[zeroSet];
        initSingleComponentGroups(componentSetFactory);
    }

    void GroupFactory::initSingleComponentGroups(
        ComponentSetFactory& componentSetFactory
    ) {
        for (
            std::size_t typeIndex = 0;
            typeIndex < maxComponents;
            ++typeIndex
            ) {
            const auto& singleSet = componentSetFactory.makeSet(typeIndex);
            keyToGroupMap.emplace(singleSet, &singleSet);
            if (!zeroGroupPointer->addNewGroup(&keyToGroupMap[singleSet])) {
                throw std::runtime_error{ "error in initSingleComponentGroups" };
            }
        }
    }
}