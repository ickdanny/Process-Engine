#pragma once

#include "Resource.h"
#include "ResourceStorage.h"

namespace process::resource {
    using ChildList = std::vector<wasp::resource::ResourceBase*>;

    class ChildListResource
        : public Resource<std::vector<wasp::resource::ResourceBase*>> {

    public:
        ChildListResource(
            const std::wstring&id,
            const ResourceOriginVariant&origin
        )
            : Resource<ChildList> { id, origin } {
        }

        ChildListResource(
            const std::wstring&id,
            const ResourceOriginVariant&origin,
            std::shared_ptr<ChildList> dataPointer
        )
            : Resource<ChildList> {
            id, origin, dataPointer
        } {
        }

        ~ChildListResource() override;

    protected:
        void removeChild(ResourceBase*child) override;
    };

    class ParentResourceStorage : public ResourceStorage<ChildList> {

    public:

        void unload(const std::wstring&id) override;

        void remove(const std::wstring&id) override;
    };
}