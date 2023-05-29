#pragma once

#include "systemInclude.h"
#include "Utility/TwoFrame.h"

namespace process::game::systems {

	class PlayerMovementSystem {

    private:
        //forward declarations
        class PlayerInputData;

        //typedefs
        using TwoFramePlayerInputData = utility::TwoFrame<PlayerInputData>;
        using Vector2 = math::Vector2;

    public:
        void operator()(Scene& scene);

    private:
        //helper functions
        void parseGameCommand(GameCommands gameCommand, PlayerInputData& inputData);
        math::Vector2 calculateVelocity(PlayerInputData inputData);

        //private class
        class PlayerInputData {
        private:
            //typedefs
            using DataType = std::uint_fast8_t;

            //constants
            static constexpr DataType upMask =       0b0000'0001;
            static constexpr DataType downMask =     0b0000'0010;
            static constexpr DataType leftMask =     0b0000'0100;
            static constexpr DataType rightMask =    0b0000'1000;
            static constexpr DataType focusMask =    0b0001'0000;

            static constexpr DataType verticalMask = upMask | downMask;
            static constexpr DataType horizontalMask = leftMask | rightMask;
            static constexpr DataType allDirectionMask = verticalMask | horizontalMask;

            //fields
            DataType data{};

        public:
            //default constructor and copy constructor autogenerated

            void reset() {
                data = 0;
            }

            //getters
            bool isZero() const {
                //either we are not moving at all or we are moving in all directions
                return !getData(allDirectionMask)
                    || ((data & allDirectionMask) == allDirectionMask);
            }
            bool isUp() const {
                return getData(upMask);
            }
            bool isDown() const  {
                return getData(downMask);
            }
            bool isLeft() const  {
                return getData(leftMask);
            }
            bool isRight() const  {
                return getData(rightMask);
            }
            bool isFocused() const  {
                return getData(focusMask);
            }

            //setters
            void setUp(bool up) {
                updateData(upMask, up);
            }
            void setDown(bool down) {
                updateData(downMask, down);
            }
            void setLeft(bool left) {
                updateData(leftMask, left);
            }
            void setRight(bool right) {
                updateData(rightMask, right);
            }
            void setFocus(bool focused) {
                updateData(focusMask, focused);
            }

            //equality operators
            friend bool operator==(
                const PlayerInputData& left,
                const PlayerInputData& right
            ) {
                return left.data == right.data;
            }

            friend bool operator!=(
                const PlayerInputData& left,
                const PlayerInputData& right
                ) {
                return left.data != right.data;
            }

        private:
            //helper functions
            void updateData(DataType mask, bool set) {
                if (set) {
                    setData(mask);
                }
                else {
                    unsetData(mask);
                }
            }
            void setData(DataType mask) {
                data |= mask;
            }
            void unsetData(DataType mask) {
                data &= ~mask;
            }

            bool getData(DataType mask) const {
                return data & mask;
            }
        };
	};
}