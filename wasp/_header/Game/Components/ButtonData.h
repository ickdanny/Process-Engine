#pragma once

#include <string>

#include "Math/Point2.h"

namespace wasp::game::components {
    struct ButtonData {
    private:
        //typedefs
        using Point2 = math::Point2;

        //fields
        std::wstring baseImageName{};
        Point2 unselPos{};
        Point2 selPos{};
    public:
        bool locked{};

        //constructs a button data with the specified pos and image name
        ButtonData(Point2 pos, const std::wstring& baseImageName)
            : unselPos{ pos }
            , selPos{ pos }
            , baseImageName{ baseImageName }
            , locked{ false } {
        }

        //constructs a button data with the specified unselPos, selPos, and image name
        ButtonData(Point2 unselPos, Point2 selPos, const std::wstring& baseImageName)
            : unselPos{ unselPos }
            , selPos{ selPos }
            , baseImageName{ baseImageName } 
            , locked{ false } {
        }

        //constructs a button data with the specified unselPos, selPos, and image name
        //which may be locked
        ButtonData(
            Point2 unselPos,
            Point2 selPos,
            const std::wstring& baseImage,
            bool locked
        )
            : unselPos{ unselPos }
            , selPos{ selPos }
            , baseImageName{ baseImageName } 
            , locked{ locked } {
        }

        const Point2& getUnselPos() const {
            return unselPos;
        }
        const Point2& getSelPos() const {
            return selPos;
        }

        const std::wstring getSelImageName() const;
        const std::wstring getUnselImageName() const;
        const std::wstring getLockedImageName() const;
    };
}