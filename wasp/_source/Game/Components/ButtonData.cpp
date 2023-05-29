#include "Game/Components/ButtonData.h"

namespace wasp::game::components {

    const std::wstring ButtonData::getSelImageName() const {
        return baseImageName + L"_sel";
    }

    const std::wstring ButtonData::getUnselImageName() const {
        return baseImageName + L"_unsel";
    }

    const std::wstring ButtonData::getLockedImageName() const {
        return baseImageName + L"_locked";
    }
}