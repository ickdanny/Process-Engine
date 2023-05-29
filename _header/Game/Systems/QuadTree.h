#pragma once

#include <array>
#include <vector>
#include <memory>

#include "Math/Geometry.h"
#include "Utility/TwoFrame.h"

namespace process::game::systems {

	template <typename IdType>
	class QuadTree {
	private:
        //typedefs
        using AABB = wasp::math::AABB;
        using Point2 = wasp::math::Point2;
        using Vector2 = wasp::math::Vector2;
        using TwoFramePosition = wasp::utility::TwoFrame<Point2>;

        //inner types
        struct Element {
            IdType id{};
            AABB hitbox{};
            AABB twoFrameEncompassingHitbox{};
            AABB trueHitbox{};
            TwoFramePosition twoFramePosition{};
        };

        //constants
        constexpr static std::size_t defaultMaxElements{ 8 };
        constexpr static int defaultLevel{ 4 };

        //fields
        const std::size_t maxElements{};
        const int level{};
        const AABB bounds{};
        std::array<std::shared_ptr<QuadTree<IdType>>, 4> subTreePointers{};
        std::vector<Element> elements{};

    public:
        //constructors

        //Constructs a default quad tree with the given bounds
        QuadTree(const AABB& bounds)
            : maxElements{ defaultMaxElements }
            , level{ defaultLevel }
            , bounds{ bounds } {
        }

        //Constructs a quad tree with the specified max elements, level, and bounds
        QuadTree(std::size_t maxElements, int level, const AABB& bounds)
            : maxElements{ maxElements }
            , level{ level }
            , bounds{ bounds } {
        }

        //Inserts an object with the given id, hitbox, and position into this quadtree
        //if that object falls within the bounds of this quadtree.
        void insert(
            const IdType& id,
            const AABB& hitbox,
            const TwoFramePosition& twoFramePosition
        ) {
            //create relevant AABBs
            AABB trueHitbox{ hitbox.centerAt(twoFramePosition) };
            AABB pastHitbox{ hitbox.centerAt(twoFramePosition.getPast()) };
            AABB twoFrameEncompassingHitbox{
				wasp::math::makeEncompassingAABB(trueHitbox, pastHitbox)
            };

            //check to see if this object falls into this quadtree
            if (collides(twoFrameEncompassingHitbox)) {

                //if so, construct an element for the object and insert it
                Element element{
                    id,
                    hitbox,
                    twoFrameEncompassingHitbox,
                    trueHitbox,
                    twoFramePosition
                };
                insertElement(element);
            }
        }

        //Returns the list of objects in this quadtree that collide with the object
        //specified by the given hitbox and two frame position
        std::vector<IdType> checkCollisions(
            const AABB& hitbox,
            const TwoFramePosition& twoFramePosition
        ) const {
            //make a dummy Element to use in collision checks
            AABB trueHitbox{ hitbox.centerAt(twoFramePosition) };
            AABB pastHitbox{ hitbox.centerAt(twoFramePosition.getPast()) };
            AABB twoFrameEncompassingHitbox{
				wasp::math::makeEncompassingAABB(trueHitbox, pastHitbox)
            };
            Element dummyElement{
                {},
                hitbox,
                twoFrameEncompassingHitbox,
                trueHitbox,
                twoFramePosition
            };

            std::vector<IdType> collisionList{};
            populateCollisionList(dummyElement, collisionList);
            return collisionList;
        }

        //Returns the bounds of this quadtree
        AABB getBounds() {
            return { bounds };
        }

        //Returns the bounds of this quadtree
        [[nodiscard]]
		const AABB& getBounds() const {
            return bounds;
        }

        //Returns true if this quadtree has no objects
        [[nodiscard]]
		bool isEmpty() const {
            if (hasSubTrees()) {
                for (const auto& subTreePointer : subTreePointers) {
                    if (!subTreePointer->isEmpty()) {
                        return false;
                    }
                }
            }
            return elements.empty();
        }

    private:
        //helper functions

        //Inserts the given element into this quadtree or a subtree if the element
        //is within bounds.
        void insert(
            const Element& element
        ) {
            //check to see if this object falls into this quadtree
            if (collides(element.twoFrameEncompassingHitbox)) {
                insertElement(element);
            }
        }

        //Inserts an element into this quadtree or a subtree regardless of whether the 
        //element is within bounds.
        void insertElement(const Element& element) {
            if (hasSubTrees()) {
                std::shared_ptr<QuadTree<IdType>> collidedSubTreePointer{};
                int numSubTreesCollided{ 0 };

                for (auto& subTreePointer : subTreePointers) {
                    if (subTreePointer->collides(element.twoFrameEncompassingHitbox)) {
                        collidedSubTreePointer = subTreePointer;
                        ++numSubTreesCollided;
                    }
                }

                //if only 1 subTree collided, insert into that subTree
                if (numSubTreesCollided == 1) {
                    collidedSubTreePointer->insert(element);
                }
                //otherwise insert into the top level tree (this)
                else {
                    addElement(element);
                }
            }
            else {
                addElement(element);
            }
        }

        //Returns true if the given hitbox touches the bounds of this QuadTree.
        [[nodiscard]]
		bool collides(const AABB& hitbox) const {
            return wasp::math::collides(hitbox, bounds);
        }

        //Returns true if this QuadTree has subtrees, false otherwise.
        [[nodiscard]]
		bool hasSubTrees() const {
            return static_cast<bool>(subTreePointers[0]);
        }

        //Returns true if this QuadTree is eligible to split, false otherwise.
        [[nodiscard]]
		bool canSplit() const {
            return !hasSubTrees() && level > 0;
        }

        //Adds the given element and splits if necessary
        void addElement(const Element& element) {
            elements.push_back(element);
            tryToSplit();
        }

        //Splits this quadtree if necessary
        void tryToSplit() {

            //if we can't split, bail
            if (!canSplit()) {
                return;
            }

            //if we are not at max elements, no need to split
            if (elements.size() < maxElements) {
                return;
            }

            //otherwise, calculate new bounds and create subtrees
            float xAvg{ (bounds.xLow + bounds.xHigh) / 2.0f };
            float yAvg{ (bounds.yLow + bounds.yHigh) / 2.0f };

            int nextLevel{ level - 1 };

            subTreePointers[0] = std::make_shared<QuadTree<IdType>>(
                maxElements, 
                nextLevel, 
                AABB{ bounds.xLow, xAvg, bounds.yLow, yAvg } 
            );
            subTreePointers[1] = std::make_shared<QuadTree<IdType>>(
                maxElements,
                nextLevel,
                AABB{ xAvg, bounds.xHigh, bounds.yLow, yAvg }
            );
            subTreePointers[2] = std::make_shared<QuadTree<IdType>>(
                maxElements,
                nextLevel,
                AABB{ xAvg, bounds.xHigh, yAvg, bounds.yHigh }
            );
            subTreePointers[3] = std::make_shared<QuadTree<IdType>>(
                maxElements,
                nextLevel,
                AABB{ bounds.xLow, xAvg, yAvg, bounds.yHigh }
            );

            //populate our newly created subtrees
            splitElementsIntoSubTrees();
        }

        //Takes our old element list and distributes it among the subtrees
        void splitElementsIntoSubTrees() {
            std::vector<Element> oldElements{ std::move(elements) };
            elements = {};
            for (const Element& element : oldElements) {
                insert(element);
            }
        }

        //Recursive helper function for checking subtree collisions
        void populateCollisionList(
            const Element& toCollide,
            std::vector<IdType>& collisionList
        ) const {
            //check collisions in this quadTree
            for (const Element& element : elements) {
                if (collides(element, toCollide)) {
                    collisionList.push_back(element.id);
                }
            }
            //check collisions in any subTrees
            if (hasSubTrees()) {
                for (const auto& subTreePointer : subTreePointers) {
                    if (subTreePointer->collides(toCollide.trueHitbox)) {
                        subTreePointer->populateCollisionList(
                            toCollide, 
                            collisionList
                        );
                    }
                }
            }
        }

        //helper functions for detecting collisions

        static bool collides(const Element& left, const Element& right) {
            //If the twoFrameEncompassingHitboxes collide, check to see if either
            //the true hitboxes collide or the objects collided in the sub-frame.
            return
				wasp::math::collides(
                    left.twoFrameEncompassingHitbox,
                    right.twoFrameEncompassingHitbox
                ) 
                && 
                (
					wasp::math::collides(left.trueHitbox, right.trueHitbox)
                    	|| subFrameCollides(left, right)
                );
        }

        static bool subFrameCollides(const Element& left, const Element& right) {

            float largestSpeedRatio = std::max(speedRatio(left), speedRatio(right));

            if (largestSpeedRatio > 2.0f) {
                Vector2 leftVelocity{
					wasp::math::vectorFromAToB(
                        left.twoFramePosition.getPast(), left.twoFramePosition
                    )
                };
                Vector2 rightVelocity{
					wasp::math::vectorFromAToB(
                        right.twoFramePosition.getPast(), right.twoFramePosition
                    )
                };
                int numChecks{ static_cast<int>(largestSpeedRatio) - 1 };
                float baseRatio{ 1.0f / static_cast<float>(numChecks + 1) };
                for (int i{ 1 }; i <= numChecks; ++i) {
                    float currentRatio{ baseRatio * static_cast<float>(i) };

                    Point2 leftInterpolatedPos{
                        left.twoFramePosition.getPast() + leftVelocity * currentRatio
                    };
                    Point2 rightInterpolatedPos{
                        right.twoFramePosition.getPast() + rightVelocity * currentRatio
                    };

                    AABB leftInterpolatedHitbox{
                        left.hitbox.centerAt(leftInterpolatedPos)
                    };
                    AABB rightInterpolatedHitbox{
                        right.hitbox.centerAt(rightInterpolatedPos)
                    };

                    if (
						wasp::math::collides(leftInterpolatedHitbox, rightInterpolatedHitbox)
                    ) {
                        return true;
                    }
                }
            }
            return false;
        }

        //larger = two frame larger than real
        //1 = no movement
        static float speedRatio(const Element& element) {
            return element.twoFrameEncompassingHitbox.getArea() 
                / element.trueHitbox.getArea();
        }
	};
}