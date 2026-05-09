#pragma once

/* Book 2 Section 6: Quadrilaterals

Though we'll name our new primitive a quad, it will technically be a parallelogram
(opposite sides are parallel) instead of a general quadrilateral.
For our purposes, we'll use three geometric entities to define a quad:

1. Q: the starting corner.
2. u: a vector representing the first side. Q+u gives one of the corners adjacent to Q.
3. v: a vector representing the second side. Q+v gives the other corner adjacent to Q.

The corner of the quad opposite Q is given by Q+u+v.
These values are three-dimensional, even though a quad itself is a two-dimensional object.
For example, a quad with corner at the origin
and extending two units in the Z direction and one unit in the Y
direction would have values Q=(0,0,0),u=(0,0,2),and v=(0,1,0).

*/

#include "hittable.h"
#include "hittable_list.h"
#include "rtweekend.h"
#include "material.h"
#include "aabb.h"

/// @brief A Quadrilateral. **Must** call quad_init() after init the struct with Q, u, and v values.
struct Quad
{
    point3 Q; //< The starting corner.

    /// vectors representing the sides, Q+u and Q+v give the coners adjacent to Q.
    /// The corner of the quad opposite Q is given by Q+u+v.
    vec3 u, v;

    vec3 w;

    const struct Material_Cfg *mat_cfg; //< The material config for the material the quad is made from.

    /// @brief The bounding box over the quad.
    /// @remark For a stationary quad (currently all quads are stationary),
    /// initialize it by calling quad_set_static_bounding_box.
    struct AABB bbox;

    vec3 normal; //< The normal vector to the plane that *contains* the quad.

    /// @brief D in the plane equation Ax + By + Cz = D (where (x,y,z) is any point on the plane)
    /// and (A, B, C) is the normal vector to the plane.
    double D;

    /// @brief The area spanned by this quad. This is exactly the magnitude of cross(u,v).
    double area;
};

/// @brief Initializes the bounding box for a *stationary* quad.
/// @remark Must have Q, u, and v initialized before this call!
void quad_set_static_bounding_box(struct Quad *quad)
{

    // Compute the bounding box of all four vertices.
    struct AABB bbox_diagonal1, bbox_diagonal2;
    point3 q_u, q_v, q_u_v;

    // Compute the other 3 corners of the quad.
    add(q_u, quad->Q, quad->u);
    add(q_v, quad->Q, quad->v);
    add(q_u_v, quad->v, q_u);

    aabb_from_points(&bbox_diagonal1, quad->Q, q_u_v);
    aabb_from_points(&bbox_diagonal2, q_u, q_v);

    aabb_from_boxes(&quad->bbox, &bbox_diagonal1, &bbox_diagonal2);
}

/// @brief Init a quad.
/// @remark Must have Q, u, and v initialized before this call!
void quad_init(struct Quad *quad)
{
    // Note we now this cross won't result in a zero vector as u and v in a quad are not parallel.
    cross(quad->normal, quad->u, quad->v);

    quad->area = len(quad->normal);

    scale(quad->w, quad->normal, 1.0 / dot(quad->normal, quad->normal));

    unit(quad->normal, quad->normal);

    quad->D = dot(quad->normal, quad->Q);

    quad_set_static_bounding_box(quad);
}

/// @brief Given the hit point in plane coordinates, return false if it is outside the
/// primitive, otherwise set the hit record UV (texture) coordinates and return true.
bool is_interior(double a, double b, struct Hit_Record *rec)
{
    struct Interval unit_interval = {.min = 0, .max = 1};
    // Testing that both a and b are in between 0 and 1.
    if (!interval_contains(&unit_interval, a) || !interval_contains(&unit_interval, b))
    {
        return false;
    }

    rec->u = a;
    rec->v = b;
    return true;
}

/// @brief Detect if the ray hits the quad (and updates the hit record)
/// @param ray
/// @param ray_interval
/// @param rec the Hit_Record
/// @return bool if quad was hit by given ray
bool quad_hit(const struct Quad *quad, const struct Ray *ray, struct Interval ray_interval, struct Hit_Record *rec)
{
    double denom = dot(quad->normal, ray->direction);

    // No hit if the ray is parallel to the plane.
    if (fabs(denom) < 1e-8)
        return false;

    // Return false if the hit point parameter t is outside the ray interval.
    double t = (quad->D - dot(quad->normal, ray->origin)) / denom;
    if (!interval_contains(&ray_interval, t))
        return false;

    // Determine if the hit point lies within the planar shape (within the quad) using its plane coordinates.
    point3 intersection;
    ray_at(intersection, ray, t);
    vec3 planar_hitpt_vector;
    subtract(planar_hitpt_vector, intersection, (double *)quad->Q);
    // See section 6.5 for details
    vec3 temp;
    double alpha = dot(quad->w, cross(temp, planar_hitpt_vector, (double *)quad->v));
    double beta = dot(quad->w, cross(temp, (double *)quad->u, planar_hitpt_vector));

    if (!is_interior(alpha, beta, rec))
        return false;

    // Ray hits the 2D shape; set the rest of the hit record and return true.

    // Update rec->p to the point of intersection of the ray and the quad.
    memcpy(rec->p, intersection, sizeof(double) * 3);

    rec->t = t;
    // Copy a pointer to the Material_Cfg this quad has. We won't use the hit record to change the material.
    rec->mat_cfg = (struct Material_Cfg *)quad->mat_cfg;

    set_face_normal(ray, quad->normal, rec);

    return true;
}

/*
NOTE: Book 2 puts the function
    inline shared_ptr<hittable_list> box(const point3& a, const point3& b, shared_ptr<material> mat)
    which for us is world_add_box(struct Hittable *world, int start,
                                 const point3 a, const point3 b, const struct Material_Cfg *mat)
    which I think should be in hittable_list.h for us.
*/

/*
/// @remark For a moving quad, initialize it by calling quad_set_moving_bounding_box.

/// @brief Initializes the bounding box for a *moving* quad.
/// This is be the bounds over the entire range of motion, from time=0 to time=1.
/// @remark Must have _ initialized before this call!
void quad_moving_bound(struct Quad *quad)
{
}
*/

/// @brief Compute the pdf value of a sample according to the light sampling PDF in book 3 section 9.
/// @param quad The light.
/// @param origin The hit point of the ray with the non-light object.
/// @param direction A direction going from origin towards the light (the sample).
double quad_pdf_value(const struct Quad *quad, const point3 origin, const vec3 direction)
{
    struct Hit_Record rec;
    struct Ray r;
    memcpy(r.origin, origin, sizeof(double) * 3);
    memcpy(r.direction, direction, sizeof(double) * 3);

    if (!quad_hit(quad, &r, (struct Interval){.min = 0.001, .max = infinity}, &rec))
        return 0;

    // len(direction) * rec.t  is the distance
    double distance_squared = rec.t * rec.t * len_squared(direction);
    double cosine = fabs(dot(direction, rec.normal) / len(direction));

    return distance_squared / (cosine * quad->area);
}

/// @brief Generate a random sample (a direction from a non-light object towards the light)
/// accroding to the light sampling PDF in Book 3 section 9.
/// We pick a random point p on the light and return p-origin.
/// @param quad The light (or some other quad we want to sample towards).
/// @param origin Would usually be the hit point (rec.p) of the ray with the non-light object.
double *quad_random(const struct Quad *quad, const point3 origin, vec3 ret)
{
    vec3 p, temp1, temp2;
    scale(temp1, (double *)quad->u, random_zero_to_one());
    scale(temp2, (double *)quad->v, random_zero_to_one());
    add(p, (double *)quad->Q, add(temp1, temp1, temp2));

    return subtract(ret, p, (double *)origin);
}