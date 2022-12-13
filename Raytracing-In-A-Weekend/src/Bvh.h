#pragma once
#include <iostream>
#include "RTWeekend.h"

#include "Hittable.h"

inline bool boxCompare(const std::shared_ptr<Hittable> a, const std::shared_ptr<Hittable> b, int axis);

bool boxXCompare(const std::shared_ptr<Hittable> a, const std::shared_ptr<Hittable> b);
bool boxYCompare(const std::shared_ptr<Hittable> a, const std::shared_ptr<Hittable> b);
bool boxZCompare(const std::shared_ptr<Hittable> a, const std::shared_ptr<Hittable> b);

class BVHNode : public Hittable
{
public:
    BVHNode() = default;
    BVHNode(const HittableList& list) : BVHNode(list.objects, 0, list.objects.size(), -1) {}
    BVHNode(const std::vector<std::shared_ptr<Hittable>>& srcObjects, size_t start, size_t end, int maxDepth)
    {
        auto objects = srcObjects;

        int axis = randomInt(0, 2);
        auto comparator = (axis == 0) ? boxXCompare : (axis == 1) ? boxYCompare : boxZCompare;

        size_t objectSpan = end - start;

        if (objectSpan == 1)
        {
            left = right = objects[start];
        }
        else if (objectSpan == 2)
        {
            if (comparator(objects[start], objects[start + 1]))
            {
                left = objects[start];
                right = objects[start + 1];
            }
            else
            {
                left = objects[start + 1];
                right = objects[start];
            }
        }
        else
        {
            std::sort(objects.begin() + start, objects.begin() + end, comparator);
            auto mid = start + objectSpan / 2;

            if (maxDepth == 0)
            {
                std::shared_ptr<HittableList> leftList = std::make_shared<HittableList>();
                std::shared_ptr<HittableList> rightList = std::make_shared<HittableList>();
                for (size_t i = start; i < mid; i++)
                {
                    leftList->add(objects[i]);
                }
                for (size_t i = mid; i < end; i++)
                {
                    rightList->add(objects[i]);
                }
                left = leftList;
                right = rightList;
            }
            else
            {
                left = std::make_shared<BVHNode>(objects, start, mid, maxDepth-1);
                right = std::make_shared<BVHNode>(objects, mid, end, maxDepth-1);
            }
        }

        AABB boxLeft;
        AABB boxRight;

        if (!left->BoundingBox(boxLeft) || !right->BoundingBox(boxRight))
            std::cerr << "No bounding box in BVHNode Constructor." << std::endl;

        box = surroundingBox(boxLeft, boxRight);
    }

    virtual bool Hit(const Ray& r, float tMin, float tMax, HitRecord& rec) const override
    {
        if (!box.Hit(r, tMin, tMax)) return false;

        bool hitLeft = left->Hit(r, tMin, tMax, rec);
        bool hitRight = right->Hit(r, tMin, hitLeft ? rec.t : tMax, rec);

        return hitLeft || hitRight;
    }

    virtual bool BoundingBox(AABB& outputBox) const
    {
        outputBox = box;
        return true;
    }

public:
    std::shared_ptr<Hittable> left;
    std::shared_ptr<Hittable> right;
    AABB box;
};