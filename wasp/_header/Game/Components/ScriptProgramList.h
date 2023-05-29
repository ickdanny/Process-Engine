#pragma once

#include <vector>
#include <memory>
#include <unordered_map>
#include <unordered_set>

#include "Logging.h"

namespace wasp::game::components {
    enum class ScriptInstructions {
        error,                      //throws an error if reached
        condition,                  //if statement [predicateNode, trueNode], breaks
        stallCondition,             //if statement [predicateNode, trueNode], stalls
        conditionElse,              //if-else statement 
                                    //[predicateNode, trueNode, falseNode]
        routine,                    //runs a script    (predicate and next optional)
                                    //[scriptBaseNode, predicateNode, nextNode]
                                    //<Void, ScriptProgram>

        value,                      //returns a stored value, <T, Void>

        timer,                      //stalls until a timer has finished <int, ticker>
        removeVisible,              //makes the entity invisible; removes VisibleMarker
        shiftOpacityPeriod,         //Sets opacity to the target value over a period
                                    //of time
                                    //(target opacity, ticks), (increment)
                                    //<std::tuple<float, int>, float>
        shiftScalePeriod,           //Sets scale to the target value over a period
                                    //of time
                                    //(target scale, ticks), (increment)
                                    //<std::tuple<float, int>, float>
        setSpriteInstruction,       //Sets the sprite instruction component
                                    //<SpriteInstruction, Void>
        setAnimation,               //Sets the animation component, <Animation, Void>
        setDrawOrder,               //Sets the draw depth component, <Depth, Void>

        isSpawning,                 //returns true if entity has active spawns
        isNotSpawning,              //returns false if entity has active spawns

        isBossDead,                 //returns true if the boss died
        
        isDialogueOver,             //returns true if a dialogue ended this tick

        isWin,                      //returns true if win

        boundaryYLow,               //returns true if the entity's y coordinate is below
                                    //a stored value.
                                    //<float, Void> (boundary value)
        boundaryYHigh,              //returns true if the entity's y coordinate is above
                                    //a stored value.
                                    //<float, Void> (boundary value)
        boundaryXLow,               //returns true if the entity's x coordinate is below
                                    //a stored value.
                                    //<float, Void> (boundary value)
        boundaryXHigh,              //returns true if the entity's x coordinate is above
                                    //a stored value.
                                    //<float, Void> (boundary value)

        setCollidable,              //makes the entity collidable; adds CollidableMarker
        removeCollidable,           //makes the entity uncollidable; removes marker

        setHealth,                  //sets the entity's health, <int, Void> (hp)
        removeHealth,               //removes the entity's health component

        setDamage,                  //sets the entity's damage, <int, Void> (dmg)
        removeDamage,               //removes the entity's damage component

        setSpawn,                   //sets the entity's spawn, <SpawnProgram, Void>
        addSpawn,                   //adds a spawn to the entity, <SpawnProgram, Void>
        clearSpawn,                 //clears the entity's spawns

        setVelocity,                //sets the entity's velocity, <Void, Void>
                                    //[velocityNode, nextNode]
        setVelocityToPlayer,        //sets the entity's velocity in the direction of
                                    //the player
                                    //<float, Void> (speed)
        setRandomVelocity,          //sets the entity's velocity to a UNIFORMLY chosen
                                    //random velocity
                                    //<std::tuple<float, float, float, float>, Void>
                                    //(speedLow, speedHigh, angleLow, angleHigh)

        setInbound,                 //sets the entity's inbound <Inbound, Void>
                                    //<float, Void> (inbound)
        removeInbound,              //removes the entity's inbound component

        setOutbound,                //sets the entity's outbound <Outbound, Void>

        shiftSpeedPeriod,           //sets speed to target value over period of time
                                    //(targetSpeed, ticks), (increment)
                                    //<std::tuple<float, int>, float>
        shiftVelocityPeriod,        //sets velocity to target value over period of time,
                                    //does not turn.
                                    //(targetVelocity, ticks), (increment)
                                    //<std::tuple<Velocity, int>, float>
        shiftVelocityTurnPeriod,    //sets velocity to target value over period of time,
                                    //turns shortside.
                                    //(targetVelocity, initAngle, ticks),
                                    //(magnitudeIncrement, angleIncrement)
                                    //<std::tuple<Velocity, Angle, int>,
                                    //  std::tuple<float, float>
        shiftVelocityTurnLongPeriod,//sets velocity to target value over period of time,
                                    //turns longside.
                                    //(targetVelocity, initAngle, ticks),
                                    //(magnitudeIncrement, angleIncrement)
                                    //<std::tuple<Velocity, Angle, int>,
                                    //  std::tuple<float, float>
        shiftAnglePeriod,           //sets angle to target value over period of time,
                                    //turning shortside
                                    //(targetAngle, initAngle, ticks), (angleIncrement)
                                    //<std::tuple<Angle, Angle, int>, float>
        shiftAngleLongPeriod,       //sets angle to target value over period of time,
                                    //turning longside
                                    //(targetAngle, initAngle, ticks), (angleIncrement)
                                    //<std::tuple<Angle, Angle, int>, float>

        shiftSpeedIncrement,        //sets speed to target value via increment
                                    //(targetSpeed, increment)
                                    //<std::tuple<float, float>, Void>

        gotoDecelerating,           //goes to the specified point while decelerating
                                    //(targetPosition, maxSpeed), (initDistance)
                                    //<std::tuple<Point2, float>, float>
        boundRadiusGotoDecelerating,//goes to a random point within the specified bound
                                    //while decelerating
                                    //(bounds, minRad, maxRad, maxSpeed)
                                    //<std::tuple<AABB, float, float, float>, 
                                    //  (target, initDist)
                                    //  std::tuple<Point2, float>
        die,                        //triggers entity death
        removeEntity,               //removes entity entirely, bypassing death system

        startTrack,                 //starts the specified track 
                                    //<std::wstring, Void> (track id)
        showDialogue,               //shows a dialogue screen
                                    //<std::wstring, Void> (dialogue id)
        endStage,                   //ends the stage
        win,                        //broadcasts that the stage has been won
        
        numInstructions
    };

    struct ScriptNode {
    private:
        //typedefs
        using ScriptNodeSharedPointer = std::shared_ptr<ScriptNode>;

    public:
        //fields
        ScriptInstructions scriptInstruction{};
        std::vector<ScriptNodeSharedPointer> linkedNodePointers{};

        //Constructs a script node with the given scriptInstruction.
        ScriptNode(ScriptInstructions scriptInstruction)
            : scriptInstruction{ scriptInstruction } {
        }

        //virtual destructor
        virtual ~ScriptNode() = default;

        //pushes back the specified node shared pointers in depth.
        template <typename... Ts>
        ScriptNode& link(const ScriptNodeSharedPointer& node, Ts... args) {
            linkedNodePointers.push_back(node);
            if constexpr (sizeof...(args) > 0) {
                return link(args...);
            }
            return *this;
        }

        virtual void clearData(void* voidPointer) {}

        //makes a copy of the data held in the given void* and returns a new void* to
        //the copy
        virtual void* copyData(void* sourcePointer) { return nullptr; }

        friend struct ScriptProgram;
    };

    //internal data should not be changed
    template <typename Internal, typename External>
    struct ScriptNodeData : ScriptNode {
        //fields
        Internal internalData{};

        //Constructs a script node with the given scriptInstruction and internal data
        ScriptNodeData(
            ScriptInstructions scriptInstruction, 
            const Internal& internalData
        )
            : ScriptNode{ scriptInstruction }
            , internalData{ internalData } {
        }

        ~ScriptNodeData() override = default;

        External* getDataPointer(void* voidPointer) {
            if (voidPointer) {
                return reinterpret_cast<External*>(voidPointer);
            }
            else {
                return nullptr;
            }
        }

        void clearData(void* voidPointer) override {
            if (voidPointer) {
                delete reinterpret_cast<External*>(voidPointer);
            }
        }

        void* copyData(void* sourcePointer) override {
            if (sourcePointer) {
                External* externalPointer{ reinterpret_cast<External*>(sourcePointer) };
                External* copyPointer{ new External{ *externalPointer} };
                return copyPointer;
            }
            else {
                return nullptr;
            }
        }
    };

    struct ScriptProgram {
        //fields
        std::shared_ptr<ScriptNode> currentNodePointer{};
        std::unordered_map<ScriptNode*, void*> externalData{};

        //Constructs a SpawnProgram with the given currentNodePointer 
        ScriptProgram(
            const std::shared_ptr<ScriptNode>& currentNodePointer
        )
            : currentNodePointer { currentNodePointer } {
        }

        //Copies a script program, including the contents of it's external data
        ScriptProgram(const ScriptProgram& toCopy)
            : currentNodePointer{ toCopy.currentNodePointer } 
        {
            for (auto [scriptNodePointer, sourcePointer] : toCopy.externalData) {
                externalData[scriptNodePointer] 
                    = scriptNodePointer->copyData(sourcePointer);
            }
        }

        //Moves a script program, taking the contents of it's external data
        ScriptProgram(ScriptProgram&& toMove) noexcept
            : currentNodePointer{ std::move(toMove.currentNodePointer) }
        {
            for (auto [scriptNodePointer, voidPointer] : toMove.externalData) {
                externalData[scriptNodePointer] = voidPointer;
            }
            toMove.externalData.clear();
        }

        //Copy assignment operator, copies external data
        ScriptProgram& operator=(const ScriptProgram& toCopy) {
            clearExternalData();
            currentNodePointer = toCopy.currentNodePointer;
            for (auto [scriptNodePointer, sourcePointer] : toCopy.externalData) {
                externalData[scriptNodePointer]
                    = scriptNodePointer->copyData(sourcePointer);
            }
            return *this;
        }

        //Move assignment operator, takes contents of external data
        ScriptProgram& operator=(ScriptProgram&& toMove) noexcept {
            clearExternalData();
            currentNodePointer = toMove.currentNodePointer;
            for (auto [scriptNodePointer, voidPointer] : toMove.externalData) {
                externalData[scriptNodePointer] = voidPointer;
            }
            toMove.externalData.clear();
            return *this;
        }

        //clears the external data
        ~ScriptProgram() {
            clearExternalData();
        }

    private:
        //helper function
        void clearExternalData() {
            for (auto [scriptNodePointer, voidPointer] : externalData) {
                scriptNodePointer->clearData(voidPointer);
            }
            externalData.clear();
        }
    };

    using ScriptProgramList = std::vector<ScriptProgram>;
}