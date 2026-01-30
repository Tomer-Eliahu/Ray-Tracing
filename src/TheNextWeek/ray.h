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
    double tm; //< The exact time of the ray existing, in absolute time.
};

/* We added Motion Blur (see Section 2 of TheNextWeek book)

Our model is the following: We will render only a single frame,
implicitly assuming a start at time = 0 and ending at time = 1.
Our first task is to modify the camera to launch rays with random times in [0,1],
and our second task will be the creation of an animated sphere.

To see why what we are doing here still preserves the other effects we have (anti-aliasing + defocus blur),
think of the rays we are averaging. Motion blur is simply averaging the same image over time (so think
of us producing a complete image at time 0, one at time 0.1, one at time 0.2,
and averaging all those complete images together).
It is clear that for the static parts of the image what we do now (our changes for this section)
is the exact same thing we did previously (since time has no impact).
And as for the dynamic parts: we are averaging the same rays we would
have if we were averaging complete images (we just average less of them than in that way).

*/

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