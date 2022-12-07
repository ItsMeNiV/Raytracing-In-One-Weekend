#include "AABB.h"

AABB surroundingBox(AABB box0, AABB box1)
{
glm::vec3 small(fmin(box0.minimum.x, box1.minimum.x),
    fmin(box0.minimum.y, box1.minimum.y),
    fmin(box0.minimum.z, box1.minimum.z));
glm::vec3 big(fmax(box0.maximum.x, box1.maximum.x),
    fmax(box0.maximum.y, box1.maximum.y),
    fmax(box0.maximum.z, box1.maximum.z));
return AABB(small, big);
}