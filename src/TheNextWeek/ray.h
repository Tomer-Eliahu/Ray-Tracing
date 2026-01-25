#pragma once

/*
    The one thing that all ray tracers have is a ray abstraction
    and a computation of what color is seen along a ray.
    Let’s think of a ray as a function P(t)=A+tB.
    Here P is a 3D position along a line in 3D.
    A is the ray origin and B is the ray direction.
    The ray parameter t is a real number (double in the code).

*/

#include "vec3.h"

/**
 *  @struct Ray
 *  @brief A ray consists of an 3D origin point and a direction.
 *  @param direction this is not necessarily a unit vector!
 */
struct Ray
{
    point3 origin;
    vec3 direction;
};

/// @brief Computes dest: the point the ray will be at t (P(t)= Origin+t* Direction).
double *ray_at(vec3 dest, const struct Ray *ray, double t)
{
    for (int i = 0; i < 3; i++)
    {
        dest[i] = ray->origin[i] + ray->direction[i] * t;
    }

    // for ergonomics
    return dest;
}

/*

At its core,
a ray tracer sends rays through pixels and computes the color seen in the direction of those rays.
The involved steps are:

    1.Calculate the ray from the “eye” (the camera center) through the pixel,
    2.Determine which objects the ray intersects, and
    3.Compute a color for the closest intersection point.

Lets do a simple camera for getting the code up and running.
The camera will be placed some distance orthogonally behind a digital viewport
(you can think of this viewport as a window through which we view the image; like standing
at the window in your house, you view a 3D world through a 2D object).

For simplicity we'll start with the camera center at (0,0,0).
We'll also have the y-axis go up, the x-axis to the right, and the negative z-axis
pointing in the viewing direction.
(This is commonly referred to as right-handed coordinates.)

See section 4.2 for more details.

*/