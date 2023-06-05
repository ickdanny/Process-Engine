#pragma once

#include "ECS/Entity/EntityHandle.h"
#include "Components/MenuCommand.h"
#include "Components/ButtonData.h"
#include "Graphics/SpriteDrawInstruction.h"
#include "Components/AnimationList.h"
#include "Components/PlayerData.h"
#include "Components/CollisionType.h"
#include "Components/DeathCommand.h"
#include "Components/SpawnProgramList.h"
#include "Components/ScriptProgramList.h"
#include "Components/PickupType.h"
#include "Components/TwoFramePosition.h"
#include "PolarVector.h"
#include "AABB.h"
#include "Rectangle.h"

namespace process::game {

    //ui
    struct MenuCommandUp : components::MenuCommand {
        using components::MenuCommand::MenuCommand;
    };
    struct MenuCommandDown : components::MenuCommand {
        using components::MenuCommand::MenuCommand;
    };
    struct MenuCommandLeft : components::MenuCommand {
        using components::MenuCommand::MenuCommand;
    };
    struct MenuCommandRight : components::MenuCommand {
        using components::MenuCommand::MenuCommand;
    };
    struct MenuCommandSelect : components::MenuCommand {
        using components::MenuCommand::MenuCommand;
    };

    struct NeighborElementUp : wasp::ecs::entity::EntityHandle {};
    struct NeighborElementDown : wasp::ecs::entity::EntityHandle {};
    struct NeighborElementLeft : wasp::ecs::entity::EntityHandle {};
    struct NeighborElementRight : wasp::ecs::entity::EntityHandle {};

    struct ButtonData : wasp::game::components::ButtonData {
        using wasp::game::components::ButtonData::ButtonData;
    };

    //graphics
    struct VisibleMarker {};
    struct SpriteInstruction : graphics::SpriteDrawInstruction {
        using graphics::SpriteDrawInstruction::SpriteDrawInstruction;
    };
    struct SubImage : wasp::math::Rectangle {
        using wasp::math::Rectangle::Rectangle;
    };
    struct SubImageScroll : wasp::math::Rectangle {
        using wasp::math::Rectangle::Rectangle;
    };
    struct TextInstruction {
        std::wstring text{};
        std::pair<float, float> bounds{};
    };

    struct RotateSpriteForwardMarker {};
    struct SpriteSpin {
        float spin{};
    };

    struct AnimationList : components::AnimationList {
        using components::AnimationList::AnimationList;
    };

    struct Position : wasp::game::components::TwoFramePosition {
        using wasp::game::components::TwoFramePosition::TwoFramePosition;

        //Explicitly define copy and move constructors taking Point2
        Position(const wasp::math::Point2& toCopy)
            : wasp::game::components::TwoFramePosition(toCopy) {
        }
        Position(wasp::math::Point2&& toMove)
            : wasp::game::components::TwoFramePosition(toMove) {
        }
    };
    struct Velocity : wasp::math::PolarVector {
    private:
        using PolarVector = wasp::math::PolarVector;
        using Vector2 = wasp::math::Vector2;

    public:
        using PolarVector::PolarVector;

        //Explicitly define copy and move constructors taking PolarVector
        Velocity(const PolarVector& toCopy)
            : PolarVector(toCopy) {
        }
        Velocity(PolarVector&& toMove)
            : PolarVector(toMove) {
        }

        //Explicitly define copy and move constructors taking Vector2
        Velocity(const Vector2& toCopy)
            : PolarVector(toCopy) {
        }
        Velocity(Vector2&& toMove)
            : PolarVector(toMove) {
        }

        //Explicitly define assignment operators
        Velocity& operator=(const Velocity& right) = default;
        Velocity& operator=(const PolarVector& right) {
            PolarVector::operator=(right);
            return *this;
        }
    };

    //game
    struct CollidableMarker {};

    struct PlayerCollisions : components::CollisionType<PlayerCollisions> {};
    struct EnemyCollisions : components::CollisionType<EnemyCollisions> {};
    struct BulletCollisions : components::CollisionType<BulletCollisions> {};
    struct PickupCollisions : components::CollisionType<PickupCollisions> {};

    struct Hitbox : wasp::math::AABB {
        using wasp::math::AABB::AABB;

        //Explicitly define copy and move constructors taking AABB
        Hitbox(const AABB& toCopy)
            : wasp::math::AABB(toCopy) {
        }
        Hitbox(AABB&& toMove)
            : wasp::math::AABB(toMove) {
        }
    };

    struct Health {
        int value{};
    };
    struct Damage {
        int value{};
    };

    struct PickupType : wasp::game::components::PickupType {
        using wasp::game::components::PickupType::PickupType;
    };

    struct DeathCommand : wasp::game::components::DeathCommand {
        using wasp::game::components::DeathCommand::DeathCommand;

        //Explicitly defined constructor taking the command
        DeathCommand(wasp::game::components::DeathCommand::Commands command)
            : wasp::game::components::DeathCommand{ command } {
        }
    };
    struct ScriptProgramList : wasp::game::components::ScriptProgramList {
        using wasp::game::components::ScriptProgramList::ScriptProgramList;
    };

    struct SpawnProgramList : wasp::game::components::SpawnProgramList {
        using wasp::game::components::SpawnProgramList::SpawnProgramList;
    };

    struct DeathSpawn {
        SpawnProgramList spawnProgramList{};
    };

    struct Inbound {
        float bound;
    };
    struct Outbound {
        float bound;
    };

    struct PlayerData : components::PlayerData {
        using components::PlayerData::PlayerData;

        //Explicitly define assignment operator
        PlayerData& operator=(const components::PlayerData& right) {
			components::PlayerData::operator=(right);
            return *this;
        }
    };
}