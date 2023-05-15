#pragma once

#include <string>
#include <vector>
#include <memory>

#include "Resource/Loadable.h"
#include "Resource/ResourceBase.h"

namespace process::resource {

    using ManifestPrefixes = std::vector<std::wstring>;
    using ManifestArguments = std::vector<std::wstring>;

    class ResourceLoader;

    struct ManifestOrigin {
        ManifestArguments manifestArguments {};
    };

    class ManifestLoadable : virtual public wasp::resource::Loadable {
    private:
        ManifestPrefixes manifestPrefixes {};
    public:
        ManifestLoadable(const ManifestPrefixes&manifestPrefixes)
            : manifestPrefixes { manifestPrefixes } {
        }

        virtual ~ManifestLoadable() = default;

        bool isManifestLoadable() const override {
            return true;
        }

        const ManifestPrefixes&getAcceptableManifestPrefixes() const {
            return manifestPrefixes;
        }

        virtual wasp::resource::ResourceBase*loadFromManifest(
            const ManifestOrigin&manifestOrigin,
            const ResourceLoader&resourceLoader
        ) = 0;
    };
}