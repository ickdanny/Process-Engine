#include "ECS/Component/ComponentSet.h"

namespace wasp::ecs::component {

    //constructs a component set based on the provided type index
    ComponentSet::ComponentSet(std::size_t typeIndex)
        : numComponents{ 1 }
    {
        #pragma warning(suppress : 4834) //suppress discarding return; it throws
        bitset.test(typeIndex);
        bitset[typeIndex] = true;
        presentTypeIndices.push_back(typeIndex);
    }

    //constructs a component set based on the provided type indices
    ComponentSet::ComponentSet(const std::vector<std::size_t>& typeIndices)
        : numComponents{ typeIndices.size() }
    {
        for (std::size_t typeIndex : typeIndices) {
            #pragma warning(suppress : 4834) //suppress discarding return
            bitset.test(typeIndex);
            bitset[typeIndex] = true;
        }
        presentTypeIndices = typeIndices;
    }

    bool ComponentSet::isContainedIn(const ComponentSet& other) const {
        Bitset temp{ bitset };    //copy our bitset into our temp
        temp &= other.bitset;   //our temp is now A&B
        return bitset == temp;
    }

    const std::vector<std::size_t>& ComponentSet::getPresentTypeIndices() const {
        //assuming the empty ComponentSet will not get called very much
        if (presentTypeIndices.empty()) {
            makePresentTypeIndices();
        }
        return presentTypeIndices;
    }

    //conversion to string
    ComponentSet::operator std::string() const {
        return bitset.to_string();
    }

    //helper functions
    void ComponentSet::makePresentTypeIndices() const {
        presentTypeIndices = std::vector<std::size_t>{};
        for (int i = 0; i < maxComponents; ++i) {
            if (bitset[i]) {
                presentTypeIndices.push_back(i);
            }
        }
    }

    //operators
    bool operator==(const ComponentSet& a, const ComponentSet& b) {
        return a.bitset == b.bitset;
    }

    bool operator!= (const ComponentSet& a, const ComponentSet& b) {
        return a.bitset != b.bitset;
    }
}