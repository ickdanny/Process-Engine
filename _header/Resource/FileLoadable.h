#pragma once

#include <string>
#include <vector>
#include <memory>

#include "Resource/Loadable.h"
#include "Resource/ResourceBase.h"

namespace process::resource {

    using FileTypes = std::vector<std::wstring>;

    class ResourceLoader;

    struct FileOrigin {
        std::wstring fileName {};
    };


    class FileLoadable : virtual public wasp::resource::Loadable {
    private:
        FileTypes fileTypes {};
    public:
        FileLoadable(const FileTypes&fileTypes)
            : fileTypes { fileTypes } {
        }

        virtual ~FileLoadable() = default;

        bool isFileLoadable() const override {
            return true;
        }

        const FileTypes&getAcceptableFileTypes() const {
            return fileTypes;
        }

        virtual wasp::resource::ResourceBase*loadFromFile(
            const FileOrigin&fileOrigin,
            const ResourceLoader&resourceLoader
        ) = 0;
    };
}