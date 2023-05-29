#pragma once

#include <unordered_set>
#include <functional>

#include "ComponentSet.h"

namespace wasp::ecs::component {
    class ComponentSetFactory {
    private:
        //fields
        std::unordered_set<ComponentSet> canonicalComponentSets{};
        std::function<void(const ComponentSet&)> newComponentSetCallback{};

    public:
        //default constructor
        ComponentSetFactory() = default;

        //does not reset the new component set callback
        void clear() {
            canonicalComponentSets.clear();
        }

        //component set creation
        
        //returns the empty component set
        const ComponentSet& makeSet() {
            return getCanonicalSetAndBroadcastIfNew(ComponentSet{});
        }

        //returns a component set representing the specified types
        template <typename... Ts>
        const ComponentSet& makeSet() {
            return getCanonicalSetAndBroadcastIfNew(
                ComponentSet::makeComponentSetFromVariadicTemplate<Ts...>()
            );
        }

        //returns a component set with the specified type index
        const ComponentSet& makeSet(std::size_t typeIndex) {
            return getCanonicalSetAndBroadcastIfNew(ComponentSet{ typeIndex });
        }

        template <typename T>
        const ComponentSet& addComponent(const ComponentSet& base) {
            return getCanonicalSetAndBroadcastIfNew(base.addComponent<T>());
        }

        template <typename... Ts>
        const ComponentSet& addComponents(const ComponentSet& base) {
            return getCanonicalSetAndBroadcastIfNew(base.addComponents<Ts...>());
        }

        template <typename T>
        const ComponentSet& removeComponent(const ComponentSet& base) {
            return getCanonicalSetAndBroadcastIfNew(base.removeComponent<T>());
        }

        template <typename... Ts>
        const ComponentSet& removeComponents(const ComponentSet& base) {
            return getCanonicalSetAndBroadcastIfNew(base.removeComponents<Ts...>());
        }

        //setting callbacks
        void setNewComponentSetCallback(
            const std::function<void(const ComponentSet&)>& newComponentSetCallback
        ) {
            this->newComponentSetCallback = newComponentSetCallback;
        }

    private:
        const ComponentSet& getCanonicalSetAndBroadcastIfNew(
            const ComponentSet& componentSet
        );
    };
}