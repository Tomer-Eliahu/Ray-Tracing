#pragma once
#include "vec3.h"

/* Book 3: Section 8: Orthonormal Bases

Basically, we want to do a change of coordinates from
an orthonormal basis where one of the axes is the surface normal to the scene-coordinates.
That will enable us to use random_cosine_direction in vec3.h in a more general way.

*/

/// @brief An Orthonormal Basis where its **z-axis** (called axis[2] or w) is the surface normal.
/// @remark Must be init with init_onb.
struct ONB
{
    /// @brief a 3x3 matrix where axis[i] is one of the basis vectors.
    /// The book calls axis[0], axis[1], axis[2] by u, v, w respectively.
    vec3 axis[3];
};

/// @brief Find an orthonormal basis where one of the axes (specifically the z-axis) is unit(n).
/// Initializes onb.
/// @param onb
/// @param n A surface normal (must not be the zero vector).
void init_onb(struct ONB *onb, const vec3 n)
{
    unit(onb->axis[2], (double *)n);
    // Make sure we pick a vector a which is not parallel to n.
    vec3 a = {0};
    a[(fabs(onb->axis[2][0]) > 0.9 ? 1 : 0)] = 1;

    vec3 temp;
    unit(onb->axis[1], cross(temp, onb->axis[2], a));
    // Note the cross product of these two perpendicular unit vectors is also a unit vector.
    cross(onb->axis[0], onb->axis[2], onb->axis[1]);
}

/// @brief Transforms v *from* ONB's uvw coordinates *into* our scene coordinates.
double *onb_transform(vec3 ret, const struct ONB *onb, const vec3 v)
{
    struct ONB *uvw = (struct ONB *)onb;

    // Transform from basis coordinates to local space.
    // ret = (v[0] * axis[0]) + (v[1] * axis[1]) + (v[2] * axis[2])
    vec3 temp1, temp2;
    add(temp2, scale(temp1, uvw->axis[1], v[1]),
        scale(temp2, uvw->axis[2], v[2]));
    add(ret, temp2, scale(ret, uvw->axis[0], v[0]));

    return ret;
}
