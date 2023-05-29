#pragma once

#include <cstddef>

namespace wasp::ecs::component {
    //thanks to a user named DragonSlayer0531

    class ComponentIndexer{
    private:
        static std::size_t indexCounter;

    public:
        template <typename T>
        static std::size_t getIndex() {
            static std::size_t typeIndex = indexCounter++;
            return typeIndex;
        }
    };
}