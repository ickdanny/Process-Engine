#pragma once

#include <variant>

#include "Resource/FileLoadable.h"
#include "Resource/ManifestLoadable.h"

namespace process::resource {
    using ResourceOriginVariant = std::variant<FileOrigin, ManifestOrigin>;
}