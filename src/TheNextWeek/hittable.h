#pragma once
#include "vec3.h"
#include <stdbool.h>

// Forward declare Material_Cfg.
struct Material_Cfg;

struct Hit_Record
{
    point3 p; //< The point of hitting this object
    vec3 normal;
    struct Material_Cfg *mat_cfg; //< The material config for the object we hit.
    bool front_face;              //< If the ray hits the front_face of the object or the back_face.
    double t;
};

// int hit_example(const struct Ray* ray, struct Interval ray_interval, struct Hit_Record* rec) {
//     return 0;
// }

// /// @brief Sets the front_face field of the Hit Record
// /// @param ray
// /// @param outward_normal
// void set_face_normal_example(const struct Ray* ray, const vec3 outward_normal)
// {
//     return;
// }