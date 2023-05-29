#include "ECS/Component/Group.h"

#include "Logging.h"

namespace wasp::ecs::component {
    Group::Group(const ComponentSet* const componentKeyPointer)
        : componentKeyPointer{ componentKeyPointer }
    {
        //we do NOT receive the new archetype here
    }

    void Group::receiveNewArchetype(std::shared_ptr<Archetype> archetypePointer) {
        if (componentSetFitsIntoGroup(
            archetypePointer->getComponentKeyPointer()
        )) {
            //add archetype to group, and propogate up the group tree
            archetypePointers.push_back(std::move(archetypePointer));
            auto archetypePointerRef{ archetypePointers.back() };
            for (Group* childPointer : childGroupPointers) {
                childPointer->receiveNewArchetype(archetypePointerRef);
            }
        }
    }

    bool Group::addNewGroup(Group* groupPointer) {
        if (groupPointer == this) {
            throw std::runtime_error{
                "Tried to add group to itself!"
            };
        }
        auto otherComponentKeyPointer{ groupPointer->getComponentKeyPointer() };
        if (otherComponentKeyPointer == componentKeyPointer) {
            throw new std::runtime_error{
                "Tried to add a group with the same key!"
            };
        }

        if (componentSetFitsIntoGroup(otherComponentKeyPointer)) {
            for (Group* childPointer : childGroupPointers) {
                if (childPointer->addNewGroup(groupPointer)) {
                    return true;
                }
            }
            //if fits this group but no children, add as a direct child
            addChildGroup(groupPointer);
            return true;
        }
        return false;
    }

    void Group::addChildGroup(Group* childGroupPointer) {
        childGroupPointers.push_back(childGroupPointer);
        for (std::shared_ptr<Archetype> archetypePointer : archetypePointers) {
            childGroupPointer->receiveNewArchetype(archetypePointer);
        }
    }

    bool Group::componentSetFitsIntoGroup(
        const ComponentSet* const componentSetPointer
    ) const {
        return componentKeyPointer->isContainedIn(*componentSetPointer);
    }
}