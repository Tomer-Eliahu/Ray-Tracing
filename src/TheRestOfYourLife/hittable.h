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

    ///@brief The u,v surface texture coordinates of the ray-object hit point (used in texture mapping).
    /// These coordinates specify the location on 2D source image (or in some 2D parameterized space).
    /// To get this, we need a way to find the u,v coordinates of any point on the surface of a 3D object.
    /// This mapping is completely arbitrary, but generally you'd like to cover the entire surface,
    /// and be able to scale, orient and stretch the 2D image in a way that makes some sense.
    /// @remark An example of such a mapping is in section 4.4 in Book 2.
    double u, v;
};

/// @brief Sets the hit record normal vector. Note this will set rec->normal to have unit length.
/// @param ray
/// @param outward_normal Assumed to have unit length!
/// @param rec
void set_face_normal(const struct Ray *ray,
                     const vec3 outward_normal, struct Hit_Record *rec)
{
    // The dot product will be positive if the ray goes in the same direction of the outward normal.
    // This happens (for example) if the ray travels from inside the sphere out.
    rec->front_face = dot(ray->direction, outward_normal) < 0;
    // We make sure the normal always goes against the ray
    rec->front_face ? memcpy(rec->normal, outward_normal, 3 * sizeof(double)) : negate(rec->normal, (double *)outward_normal);
}