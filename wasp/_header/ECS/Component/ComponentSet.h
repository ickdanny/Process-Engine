#pragma once

#include <initializer_list>
#include <vector>
#include <bitset>
#include <stdexcept>
#include <memory>

#include "ComponentIndexer.h"

namespace wasp::ecs::component {

    constexpr std::size_t maxComponents{ 96 };

    //forward declaration of Archetype to handle circular dependency
    class Archetype;

    class ComponentSet {
    private:
        //friend declarations
        friend std::hash<ComponentSet>;
        friend class ComponentSetFactory;

        //typedefs
        using Bitset = std::bitset<maxComponents>;

        //fields
        Bitset bitset{};
        std::size_t numComponents{};
        mutable std::vector<std::size_t> presentTypeIndices{};

        mutable std::weak_ptr<Archetype> archetypeWeakPointer{};

        //public constructors

    public:
        //constructs an empty component set
        ComponentSet() = default;

        //copy constructor does NOT copy the archetypeWeakPointer
        ComponentSet(const ComponentSet& toCopy)
            : bitset{ toCopy.bitset }
            , numComponents{ toCopy.numComponents }
            , presentTypeIndices{ toCopy.presentTypeIndices } {
        }

        //private constructors

    private:

        //constructs a component set based on the provided type index
        ComponentSet(std::size_t typeIndex);

        //constructs a component set based on the provided type indices
        ComponentSet(const std::vector<std::size_t>& typeIndices);

        //factory method for constructing a component set based on the 
        //provided template component types
        template <typename... Ts>
        static ComponentSet makeComponentSetFromVariadicTemplate() {
            std::size_t numComponents{ sizeof... (Ts) };
            Bitset bitset{};
            std::vector<std::size_t> presentTypeIndices{};
            (bitset.set(ComponentIndexer::getIndex<Ts>()), ...);
            (presentTypeIndices.push_back(ComponentIndexer::getIndex<Ts>()), ...);

            return ComponentSet{ bitset, numComponents, presentTypeIndices };
        }

        //helper constructor for the factory method
        ComponentSet(
            const Bitset& bitset,
            std::size_t numComponents,
            const std::vector<std::size_t>& presentTypeIndices
        )
            : bitset{ bitset }
            , numComponents{ numComponents }
            , presentTypeIndices{ presentTypeIndices } {
        }

    public:

        template <typename T>
        bool containsComponent() const {
            return bitset[ComponentIndexer::getIndex<T>()];
        }

        template <typename T>
        bool doesNotContainComponent() const {
            return !containsComponent<T>();
        }

        template <typename... Ts>
        bool containsAllComponents() const {
            if ((doesNotContainComponent<Ts>() || ...)) {
                return false;
            }
            return true;
        }

        template <typename... Ts>
        bool containsAnyComponent() const {
            if ((containsComponent<Ts>() || ...)) {
                return true;
            }
            return false;
        }

        bool isContainedIn(const ComponentSet& other) const;

        int getNumComponents() const {
            return numComponents;
        }

        const std::vector<std::size_t>& getPresentTypeIndices() const;

        //archetype stuff
        void associateArchetype(
            const std::weak_ptr<Archetype> archetypeWeakPointer
        ) const {
            this->archetypeWeakPointer = archetypeWeakPointer;
        }

        std::weak_ptr<Archetype> getAssociatedArchetypeWeakPointer() const {
            return archetypeWeakPointer;
        }

        //conversion to string
        explicit operator std::string() const;

    private:
        //modifiers
        template <typename T>
        ComponentSet addComponent() const {
            makePresentTypeIndices();   //make sure our state is good for cloning
            const std::size_t index{ ComponentIndexer::getIndex<T>() };
            //if we need to add a component
            if (!bitset[index]) {
                ComponentSet toRet{ *this };
                toRet.bitset.set(index);
                toRet.numComponents = numComponents + 1;
                toRet.presentTypeIndices.push_back(index);
                return toRet;
            }
            //otherwise return ourselves
            return *this;
        }

        template <typename T>
        ComponentSet removeComponent() const {
            const std::size_t index{ ComponentIndexer::getIndex<T>() };
            //if we need to remove a component
            if (bitset[index]) {
                ComponentSet toRet{};
                toRet.bitset = bitset;
                toRet.bitset.reset(index);
                toRet.makePresentTypeIndices();
                return toRet;
            }
            //otherwise return ourselves
            return *this;
        }

        template <typename... Ts>
        ComponentSet addComponents() const {
            if (sizeof...(Ts) <= 0) {
                throw std::runtime_error{ "zero type parameters!" };
            }
            makePresentTypeIndices();   //make sure our state is good for cloning
            std::vector<std::size_t> indicesToAdd{};
            (indicesToAdd.push_back(ComponentIndexer.getIndex<Ts>), ...);
            ComponentSet toRet{ *this };
            for (std::size_t index : indicesToAdd) {
                if (!bitset[index]) {
                    toRet.bitset.set(index);
                    ++(toRet.numComponents);
                    toRet.presentTypeIndices.push_back(index);
                }
            }
            return toRet;
        }

        template <typename... Ts>
        ComponentSet removeComponents() const {
            if (sizeof...(Ts) <= 0) {
                throw std::runtime_error{ "zero type parameters!" };
            }
            std::vector<std::size_t> indicesToRemove{};
            (indicesToRemove.push_back(ComponentIndexer.getIndex<Ts>), ...);
            ComponentSet toRet{};
            toRet.bitset = bitset;
            for (std::size_t index : indicesToRemove) {
                if (bitset[index]) {
                    toRet.bitset.reset(index);
                }
            }
            toRet.makePresentTypeIndices();
            return toRet;
        }

        //helper functions
        void makePresentTypeIndices() const;

    //operators
    public:
        friend bool operator==(const ComponentSet& a, const ComponentSet& b);
        friend bool operator!= (const ComponentSet& a, const ComponentSet& b);
    };
}

//specialization for hash
namespace std {
    template <> 
    class hash<wasp::ecs::component::ComponentSet> {
    private:
        using ComponentSet = wasp::ecs::component::ComponentSet;

    public:
        size_t operator()(const ComponentSet& componentSet) const {
            return hash<ComponentSet::Bitset>()(componentSet.bitset);
        }
    };
}