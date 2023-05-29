#pragma once

#include "ECS/Entity/EntityHandle.h"

namespace wasp::ecs {

    template <typename T>
    struct AddComponentOrder {
    private:
        //typedefs
        using EntityHandle = entity::EntityHandle;

    public:
        //fields
        EntityHandle entityHandle{};
        T component; //not initialized!

        AddComponentOrder(EntityHandle entityHandle, const T& component)
            : entityHandle{ entityHandle }
            , component{ component }{
        }
    };

    template <typename T>
    struct SetComponentOrder {
    private:
        //typedefs
        using EntityHandle = entity::EntityHandle;

    public:
        //fields
        EntityHandle entityHandle{};
        T component; //not initialized!

        SetComponentOrder(EntityHandle entityHandle, const T& component)
            : entityHandle{ entityHandle }
            , component{ component }{
        }
    };

    template <typename T>
    struct RemoveComponentOrder {
    private:
        //typedefs
        using EntityHandle = entity::EntityHandle;

    public:
        EntityHandle entityHandle{};

        RemoveComponentOrder(EntityHandle entityHandle)
            : entityHandle{ entityHandle } {
        }
    };

    template <typename... Ts>
    struct AddEntityOrder {
        //fields
        std::tuple<Ts...> components{};
        std::string name{};

        AddEntityOrder(const std::tuple<Ts...>& components)
            : components{ components } {
        }

        AddEntityOrder(const std::tuple<Ts...>& components, const std::string& name)
            : components{ components }
            , name{ name }{
        }
    };

    struct RemoveEntityOrder {
    private:
        //typedefs
        using EntityHandle = entity::EntityHandle;

    public:
        EntityHandle entityHandle{};

        RemoveEntityOrder(EntityHandle entityHandle)
            : entityHandle{ entityHandle } {
        }
    };
}