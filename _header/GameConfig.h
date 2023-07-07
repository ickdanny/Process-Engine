#pragma once

#include "Math/Geometry.h"

namespace process::game::config {
	
	//field of play
	constexpr float gameWidth{ 170.0f };
	constexpr float gameHeight{ 214.0f };
	constexpr wasp::math::Vector2 gameOffset{ 15.0f, 13.0f };
	
	constexpr float collisionOutbound{ 100.0f };
	constexpr wasp::math::AABB collisionBounds = wasp::math::AABB{
		-collisionOutbound,
		gameWidth + collisionOutbound,
		-collisionOutbound,
		gameHeight + collisionOutbound
	} + gameOffset;
	
	//player
	constexpr wasp::math::Point2 playerSpawn = wasp::math::Point2{
		gameWidth / 2.0f,
		gameHeight - 25.0f
	} + gameOffset;
	
	constexpr wasp::math::AABB playerHitbox{ 2.0f };
	
	constexpr float playerSpeed{ 3.0f };
	constexpr float focusSpeedMulti{ 0.4f };
	constexpr float focusedSpeed{ playerSpeed * focusSpeedMulti };
	constexpr float playerInbound{ 5.0f };
	
	constexpr int deathBombPeriod{ 15 };
	constexpr int bombInvulnerabilityPeriod{ 4 * 60 };
	constexpr int deathPeriod{ 30 };
	constexpr int respawnPeriod{ 20 };
	constexpr int respawnInvulnerabilityPeriod{ 3 * 60 };
	
	constexpr int initLives{ 2 };
	constexpr int initBombs{ 2 };
	constexpr int initContinues{ 2 };
	
	constexpr int maxLives{ 6 };
	constexpr int maxBombs{ maxLives };
	
	constexpr int maxPower{ 80 };
	
	constexpr int continueLives{ 2 };
	constexpr int respawnBombs{ 2 };
	
	//pickups
	constexpr int smallPowerGain{ 1 };
	constexpr int largePowerGain{ 5 };
	
	//graphics
	constexpr int backgroundDepth{ -9000 };
	constexpr int effectDepth{ -6000 };
	constexpr int playerBulletDepth{ -5000 };
	constexpr int enemyDepth{ -4000 };
	constexpr int pickupDepth{ -3000 };
	constexpr int playerDepth{ 0 };
	constexpr int enemyBulletDepth{ 1000 };
	constexpr int foregroundDepth{ 5000 };
}