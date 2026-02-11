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
    double t;                     //< Recall we think of a ray as a function P(t)=Origin+t*Direction.
};