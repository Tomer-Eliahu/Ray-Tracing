#pragma once

/* Book 2 Section 3: Bounding Volume Hierarchies

The Key Idea

The key idea of creating bounding volumes for a set of primitives is to
find a volume that fully encloses (bounds) all the objects.
For example, suppose you computed a sphere that bounds 10 objects.
Any ray that misses the bounding sphere definitely misses all ten objects inside.
If the ray hits the bounding sphere, then it might hit one of the ten objects.
So the bounding code is always of the form:

    if (ray hits bounding object)
        return whether ray hits bounded objects
    else
        return false

Note that we will use these bounding volumes to group the objects in the scene into subgroups.
We are not dividing the screen or the scene space.
We want any given object to be in just one bounding volume, though bounding volumes can overlap.

We do this to improve performence as ray-object intersection is the main time-bottleneck in a ray tracer,
and the run time is linear with the number of objects.

See this section for more details.

*/

/*Axis-Aligned Bounding Boxes (AABBs)

To get that all to work we need a way to make good divisions, rather than bad ones,
and a way to intersect a ray with a bounding volume. A ray bounding volume intersection needs to be fast,
and bounding volumes need to be pretty compact. In practice for most models,
axis-aligned boxes work better than the alternatives (such as the spherical bounds mentioned above),
but this design choice is always something to keep in mind if you encounter other types of bounding models.

From now on we will call axis-aligned bounding rectangular parallelepipeds
(really, that is what they need to be called if we're being precise) axis-aligned bounding boxes, or AABBs.
(In the code, you will also come across the naming abbreviation “bbox” for “bounding box”.)
Any method you want to use to intersect a ray with an AABB is fine.
And all we need to know is whether or not we hit it; we don’t need hit points or normals
or any of the stuff we need to display the object.

Most people use the “slab” method. This is based on the observation
that an n-dimensional AABB is just the intersection of n axis-aligned intervals, often called “slabs”.

*/

#include "rtweekend.h"
#include "assert.h"

/// @brief An Axis-Aligned Bounding Box. If making an AABB in a way other than aabb_from_boxes, padding
/// is added (if needed) to ensure the resulting bbox has a non-zero volume (we do this in aabb_from_points).
/// If init an AABB directly, please call the padding function pad_to_minimums to ensure this yourself.
struct AABB
{
    struct Interval x, y, z;
};

#define AABB_EMPTY \
    (struct AABB) { .x = INTERVAL_EMPTY, .y = INTERVAL_EMPTY, .z = INTERVAL_EMPTY }

#define AABB_UNIVERSE \
    (struct AABB) { .x = INTERVAL_UNIVERSE, .y = INTERVAL_UNIVERSE, .z = INTERVAL_UNIVERSE }

/// @brief Makes ret be the bounding box made by offseting bbox by offset.
void bbox_offset(struct AABB *ret, const struct AABB *bbox, const vec3 offset)
{
    interval_add(&ret->x, &bbox->x, offset[0]);
    interval_add(&ret->y, &bbox->y, offset[1]);
    interval_add(&ret->z, &bbox->z, offset[2]);
}

/// @brief Adjust the AABB so that no side is narrower than some delta, padding if *necessary*.
void pad_to_minimums(struct AABB *ret)
{
    static const double delta = 0.0001;

    if (interval_size(&ret->x) < delta)
        interval_expand(&ret->x, &ret->x, delta);

    if (interval_size(&ret->y) < delta)
        interval_expand(&ret->y, &ret->y, delta);

    if (interval_size(&ret->z) < delta)
        interval_expand(&ret->z, &ret->z, delta);
}

/// @brief Make an AABB from 2 points.
/// That is treat the two points a and b as extrema for the bounding box, so we don't require a
/// particular minimum/maximum coordinate order.
/// @param ret A pointer to the AABB to be initialized.
/// @remark Adds padding (if needed) to make sure our bounding boxes will always
/// have a non-zero volume even for quadrilaterals (quads).
void aabb_from_points(struct AABB *ret, const point3 a, const point3 b)
{
    ret->x =
        (a[0] <= b[0]) ? (struct Interval){.min = a[0], .max = b[0]} : (struct Interval){.min = b[0], .max = a[0]};
    ret->y =
        (a[1] <= b[1]) ? (struct Interval){.min = a[1], .max = b[1]} : (struct Interval){.min = b[1], .max = a[1]};
    ret->z =
        (a[2] <= b[2]) ? (struct Interval){.min = a[2], .max = b[2]} : (struct Interval){.min = b[2], .max = a[2]};

    pad_to_minimums(ret);
}

/// @brief Make an AABB from 2 boxes (that encloses the two input boxes).
void aabb_from_boxes(struct AABB *ret, const struct AABB *box0, const struct AABB *box1)
{
    interval_enclose(&ret->x, &box0->x, &box1->x);
    interval_enclose(&ret->y, &box0->y, &box1->y);
    interval_enclose(&ret->z, &box0->z, &box1->z);
}

/// @brief Get a pointer back to the interval along the n-th axis.
/// n=1 is the y axis, n=2 is the z-axis.
const struct Interval *aabb_axis_interval(const struct AABB *bbox, int n)
{
    if (n == 1)
    {
        return &(bbox->y);
    }
    if (n == 2)
    {
        return &(bbox->z);
    }
    return &(bbox->x);
}

/// @brief detect if the ray hits the bounding box
/// @param ray_t We only consider hits that happen in this ray interval.
/// @return bool if bbox was hit by the given ray
bool aabb_hit(const struct AABB *bbox, const struct Ray *ray, struct Interval ray_t)
{

    for (int axis = 0; axis < 3; axis++)
    {
        const struct Interval *ax = aabb_axis_interval(bbox, axis);

        /* Recall from Book 2 Section 3: t0=(x0−Qx)/dx.

            Recall that if ray->direction[axis]=0 but the numerators below are not zero:
                then both t0 and t1 will equal to each other and be either both +∞ or -∞ IF
                Qx is not between x0 and x1.
                So we return false from this function in that case (which is correct).

                If Qx is in between x0 and x1, then t0 will be -∞ and t1 will be +∞.
                So in this case this function also behaves correctly.

            If the numerator is also 0 then 0.0/0.0 is NaN in C. But we leave that case for later.
        */

        // Note that the fact we are using 1.0 instead of an int 1 is important.
        // INFINITY == 1.0 / 0

        const double adinv = 1.0 / ray->direction[axis];
        double t0 = (ax->min - ray->origin[axis]) * adinv;
        double t1 = (ax->max - ray->origin[axis]) * adinv;

        if (t0 < t1)
        {
            if (t0 > ray_t.min)
                ray_t.min = t0;
            if (t1 < ray_t.max)
                ray_t.max = t1;
        }
        else
        {
            if (t1 > ray_t.min)
                ray_t.min = t1;
            if (t0 < ray_t.max)
                ray_t.max = t0;
        }

        if (ray_t.max <= ray_t.min)
            return false;
    }

    return true;
}

/// @brief Returns the index of the longest axis of the bounding box.
int bbox_longest_axis(const struct AABB *bbox)
{
    double x_size = interval_size(&bbox->x);
    double y_size = interval_size(&bbox->y);
    double z_size = interval_size(&bbox->z);

    if (x_size > y_size)
        return x_size > z_size ? 0 : 2;
    else
        return y_size > z_size ? 1 : 2;
}