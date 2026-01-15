#pragma once

#include "hittable.h"
#include "vec3.h"
#include "sphere.h"

/// @brief An enum of all possible hittable objects (we can then have an array of the type [Hittable]
/// for a list of hittalbe objects).
///
/// [Hittable]: https://github.com/Tomer-Eliahu/Ray-Tracing/blob/main/src/InOneWeekend/hittable_list.h
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
