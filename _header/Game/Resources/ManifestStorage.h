#pragma once

#include "Resource\ParentResourceStorage.h"
#include "File\FileUtil.h"

#pragma warning(disable : 4250) //inherit via dominance

namespace process::game::resources {

    class ManifestStorage
        : public resource::ParentResourceStorage, public resource::FileLoadable, public resource::ManifestLoadable {

    private:
        using ResourceBase = wasp::resource::ResourceBase;

    public:
        ManifestStorage()
            : FileLoadable {{ L"mfst" }}, ManifestLoadable {{ L"manifest" }} {
        }

        void reload(const std::wstring&id) override;

        ResourceBase*loadFromFile(
            const resource::FileOrigin&fileOrigin,
            const resource::ResourceLoader&resourceLoader
        ) override;

        ResourceBase*loadFromManifest(
            const resource::ManifestOrigin&manifestOrigin,
            const resource::ResourceLoader&resourceLoader
        ) override;
    };
}