#pragma once

/* Book 2 Section 9: Volumes

One thing it’s nice to add to a ray tracer is smoke/fog/mist.
These are sometimes called volumes or participating media.
Another feature that is nice to add is subsurface scattering, which is sort of like dense fog inside an object.
This usually adds software architectural mayhem because volumes are a different animal than surfaces,
but a cute technique is to make a volume a random surface.
A bunch of smoke can be replaced with a surface that probabilistically
might or might not be there at every point in the volume.
This will make more sense when you see the code.

let’s start with a volume of constant density (a constant medium).

*/

#include "rtweekend.h"
#include "aabb.h"

/// @brief A Constant Medium. That is a volume (smoke/fog/mist) with constant density.
/// Will be abbreviated cm. **Must** be init using cm_init!
struct Constant_Medium
{
    /// @brief The boundary of the volume is an array of hittables as a tree of BVH nodes.
    /// It is the container of the volume.
    /// If the boundary was a Sphere, the fog would be inside the sphere.
    /// Note that this can be a single object (like a sphere) or an array (like a box which is made up of 6 quads).
    /// @remark This code assumes that once a ray exits the constant medium boundary,
    /// it will continue forever outside the boundary.
    /// Put another way, it assumes that the boundary shape is convex.
    /// So this particular implementation will work for boundaries like boxes or spheres,
    /// but will not work with toruses or shapes that contain voids.
    /// It's possible to write an implementation that handles arbitrary shapes, but we did not bother here.
    struct BVH_Node *boundary;
    double neg_inv_density;                    //< This is = (-1.0)/density.
    const struct Material_Cfg *phase_function; //< The material config for the material the volume is made from.

    /// @brief Note that a Constant Medium's bbox is boundary->bbox.
    /// We have this duplicate for simplicity (and so that we can maintain the book's header only style).
    struct AABB bbox;
};

// Needed forward declarations (to maintain the book's header only style)
struct Hittable;
struct BVH_Node;
struct Material_Cfg;
bool BVH_node_hit(const struct BVH_Node *node, const struct Ray *r, struct Interval ray_t, struct Hit_Record *rec);
struct BVH_Node *BVH_construct_tree(const struct Hittable *world, int world_length);
static inline void helper_init_cm_bbox(struct Constant_Medium *cm);

/// @brief Init a Constant Medium.
/// @param cm The constant medium (fog/volume) to init.
/// @param bounds An array (possibly of size just 1) of the Hittable objects that constitue the boundary
/// of this volume. For example, 6 quads that make up a box or 1 sphere.
/// @param bounds_size The size of the bounds array (in number of elements).
/// @param density The density of the fog.
/// @param mat The material this fog is made from.
/// @remark Must have the bbox(s) of all Hittables in bounds initialized before this call!
void cm_init(struct Constant_Medium *cm, const struct Hittable *bounds, int bounds_size,
             double density, const struct Material_Cfg *mat)
{
    cm->boundary = BVH_construct_tree(bounds, bounds_size);
    cm->neg_inv_density = (-1.0) / density;
    cm->phase_function = mat;
    helper_init_cm_bbox(cm);
}

/// @brief Detect if the ray hits the constant medium (and updates the hit record)
/// @param ray
/// @param ray_interval
/// @param rec the Hit_Record
/// @return bool if the constant medium was hit by given ray
bool cm_hit(const struct Constant_Medium *cm, const struct Ray *r, struct Interval ray_t, struct Hit_Record *rec)
{
    struct Hit_Record rec1, rec2;

    // Test that the ray gets inside the boundary
    if (!BVH_node_hit(cm->boundary, r, INTERVAL_UNIVERSE, &rec1))
    {
        return false;
    }

    // Test that the ray leaves the boundary
    if (!BVH_node_hit(cm->boundary, r,
                      (struct Interval){.min = rec1.t + 0.0001, .max = infinity}, &rec2))
    {
        return false;
    }

    if (rec1.t < ray_t.min)
        rec1.t = ray_t.min; // Set rec1.t to max(rec1.t,ray_t.min)

    if (rec2.t > ray_t.max)
        rec2.t = ray_t.max; // Set rec2.t to min(rec2.t,ray_t.max)

    if (rec1.t >= rec2.t)
        return false;

    // The ray can start (have its origin) inside the volume
    if (rec1.t < 0)
        rec1.t = 0;

    double ray_speed = len(r->direction); // Also the distance the ray travels in 1 unit of time.
    double distance_inside_boundary = (rec2.t - rec1.t) * ray_speed;
    // Source of where we get this hit_distance from:
    // https://psgraphics.blogspot.com/2013/11/scattering-in-constant-medium.html.
    // Note that what the blog calls t is really meant to be the distance d.
    // The blog assumes (but the book does not) that the direction vector of the ray is a unit vector,
    // so in that case distance inside the boundary would just be time inside the boundary.
    double hit_distance = cm->neg_inv_density * log(random_zero_to_one());

    // Ray went fully through the boundary without scattering (did not hit the fog inside)
    if (hit_distance > distance_inside_boundary)
        return false;

    // We hit the fog inside, update rec.
    rec->t = rec1.t + (hit_distance / ray_speed);
    ray_at(rec->p, r, rec->t);

    // Set the rec->normal to {1, 0, 0} which is arbitrary
    rec->normal[0] = 1;
    rec->normal[1] = 0;
    rec->normal[2] = 0;

    rec->front_face = true; // also arbitrary

    // Copy a pointer to the Material_Cfg this cm has (which we call phase_function).
    // We won't use the hit record to change the material.
    rec->mat_cfg = (struct Material_Cfg *)cm->phase_function;

    return true;
}
