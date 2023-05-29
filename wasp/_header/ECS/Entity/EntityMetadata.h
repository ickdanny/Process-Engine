#pragma once

#include "ECS/Component/ComponentSet.h"

namespace wasp::ecs::entity {
    class EntityMetadata {
    private:
        //typedefs
        using ComponentSet = component::ComponentSet;

        //fields
        const ComponentSet* componentSetPointer{};
        int generation{};   //it's almost certainly fine for generation to overflow

    public:
        EntityMetadata()
            : componentSetPointer{ nullptr }
            , generation{ 0 } {
        }

        //getters
        const ComponentSet* getComponentSetPointer() const {
            return componentSetPointer;
        }
        int getGeneration() const {
            return generation;
        }

        //setters
        void setComponentSetPointer(const ComponentSet* componentSetPointer) {
            this->componentSetPointer = componentSetPointer;
        }
        void newGeneration() {
            componentSetPointer = nullptr;
            ++generation;
        }
    };
}