#pragma once

#include "Group.h"
#include "ArchetypeFactory.h"
#include "ComponentSet.h"
#include "ComponentSetFactory.h"

namespace wasp::ecs::component {
	class GroupFactory {

    private:
        std::unordered_map<ComponentSet, Group> keyToGroupMap{};
        Group* zeroGroupPointer{};

    public:
        GroupFactory(
            ComponentSetFactory& componentSetFactory,
            ArchetypeFactory& archetypeFactory
        );

        void recreate(ComponentSetFactory& componentSetFactory);

        Group* getGroupPointer(const ComponentSet& componentKey);

    private:
        void initGroups(ComponentSetFactory& componentSetFactory);
        void initSingleComponentGroups(
            ComponentSetFactory& componentSetFactory
        );
	};
}