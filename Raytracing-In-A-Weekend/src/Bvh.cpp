#include "Bvh.h"

inline bool boxCompare(const std::shared_ptr<Hittable> a, const std::shared_ptr<Hittable> b, int axis)
{
    AABB boxA;
    AABB boxB;

    if (!a->BoundingBox(boxA) || !b->BoundingBox(boxB))
        std::cerr << "No bounding box in BVHNode Constructor." << std::endl;

    return boxA.minimum[axis] < boxB.minimum[axis];
}

bool boxXCompare(const std::shared_ptr<Hittable> a, const std::shared_ptr<Hittable> b) { return boxCompare(a, b, 0); }
bool boxYCompare(const std::shared_ptr<Hittable> a, const std::shared_ptr<Hittable> b) { return boxCompare(a, b, 1); }
bool boxZCompare(const std::shared_ptr<Hittable> a, const std::shared_ptr<Hittable> b) { return boxCompare(a, b, 2); }