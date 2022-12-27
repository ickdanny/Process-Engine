#pragma once

#include <random>
#include "windowsInclude.h"
//#include "windowsD2DInclude.h"
//#include "windowsDWriteInclude.h"

namespace process::game::config {
    //window
    constexpr wchar_t className[]{ L"PotUK" };
    constexpr wchar_t windowName[]{ L"PotUK" };
    constexpr int windowWidth{ 640 };
    constexpr int windowHeight{ 480 };

    //resources
    constexpr char mainManifestPath[]{ "res\\potuk.mfst" };
    constexpr char mainConfigPath[]{ "potuk.cfg" }; //todo: this in cmake build folder

    /*
    //graphics
    constexpr int graphicsWidth{ windowWidth / 2 };		//320
    constexpr int graphicsHeight{ windowHeight / 2 };	//240
    constexpr int fillColor{ D2D1::ColorF::Gray };
    constexpr int textColor{ D2D1::ColorF::White };
    constexpr wchar_t fontName[]{ L"Courier New" };
    constexpr float fontSize{ 11.0f };
    constexpr DWRITE_FONT_WEIGHT fontWeight{ DWRITE_FONT_WEIGHT_THIN };
    constexpr DWRITE_FONT_STYLE fontStyle{ DWRITE_FONT_STYLE_NORMAL };
    constexpr DWRITE_FONT_STRETCH fontStretch{ DWRITE_FONT_STRETCH_NORMAL };
    constexpr DWRITE_TEXT_ALIGNMENT textAlignment{ DWRITE_TEXT_ALIGNMENT_LEADING };
    constexpr DWRITE_PARAGRAPH_ALIGNMENT paragraphAlignment{
            DWRITE_PARAGRAPH_ALIGNMENT_NEAR
    };
    constexpr D2D1_BITMAP_INTERPOLATION_MODE interpolationMode{
            D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR
    };

    //game
    constexpr int updatesPerSecond{ 60 };
    constexpr int maxUpdatesWithoutFrame{ 5 };
    using PrngType = std::mt19937;
     */
}