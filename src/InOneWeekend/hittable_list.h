#pragma once

#include "hittable.h"
#include "vec3.h"
#include "sphere.h"

/// @brief An enum of all possible hittable objects (we can then have an array of this type for a list
/// of hittalbe objects).
enum Which_Hittable
{
    Sphere
};

union Hittable_Object
{
    struct Sphere sphere;
};

struct Hittable
{
    enum Which_Hittable which;
    union Hittable_Object object;
};
