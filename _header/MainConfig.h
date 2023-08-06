#pragma once

#include <random>
#include "windowsInclude.h"

namespace process::game::config {
	//window
	constexpr wchar_t className[] { L"POTUK" };
	constexpr wchar_t windowName[] { L"POTUK" };
	constexpr int windowWidth { 640 };	//640
	constexpr int windowHeight { 480 };	//480
	
	//resources
	constexpr wchar_t mainManifestPath[] { L"res\\potuk.mfst" };
	constexpr char mainConfigPath[] { "res\\potuk.cfg" };
	
	//graphics
	constexpr int graphicsWidth { windowWidth / 2 };        //320
	constexpr int graphicsHeight { windowHeight / 2 };    //240
	
	//Game
	constexpr int updatesPerSecond { 60 };
	constexpr int maxUpdatesWithoutFrame { 5 };
	using PrngType = std::mt19937;
}