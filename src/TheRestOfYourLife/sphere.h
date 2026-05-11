#pragma once

#include "rtweekend.h"
#include "hittable.h"
#include "hittable_list.h"
#include "aabb.h"
#include "onb.h"

struct Material_Cfg;

struct Sphere
{
    /// @brief Having the center be a ray allows us to have a moving sphere in addition to a static sphere.
    /// The origin of the ray is where the sphere center point was at time t = 0.
    /// The sphere moves in this ray's direction. This means that at time t = 1 the ray will be at
    /// the vector = center.origin + center.direction.
    /// @remark If we want a static sphere, just have the center.direction be the zero vector.
    /// @remark Note that the time parameter of the center ray itself (center.tm) is not used.
    struct Ray center;

    double radius;                      //< Must be 0<=
    const struct Material_Cfg *mat_cfg; //< The material config for the material the sphere is made from.

    /// @brief The bounding box over the sphere.
    /// @remark For a stationary sphere, initialize it by calling sphere_static_bound.
    /// @remark For a moving sphere, initialize it by calling sphere_moving_bound.
    struct AABB bbox;
};

/// @brief Initializes the bounding box for a *stationary* sphere.
/// @remark Must have radius and center.origin initialized before this call!
void sphere_static_bound(struct Sphere *sphere)
{
    vec3 rvec = {sphere->radius, sphere->radius, sphere->radius};
    point3 sub_res, add_res;
    subtract(sub_res, sphere->center.origin, rvec);
    add(add_res, sphere->center.origin, rvec);
    aabb_from_points(&sphere->bbox, sub_res, add_res);
}

/// @brief Initializes the bounding box for a *moving* sphere.
/// This is be the bounds over the entire range of motion, from time=0 to time=1.
/// @remark Must have radius and center (both origin and direction) initialized before this call!
void sphere_moving_bound(struct Sphere *sphere)
{
    // To do this, we can take the box of the sphere at time=0,
    // and the box of the sphere at time=1, and compute the enclosing box around those two boxes.
    // Note this assumes all of our motion is linear (that is the sphere does not backtrack).
    vec3 rvec = {sphere->radius, sphere->radius, sphere->radius};
    struct AABB box0, box1;
    point3 center_at0, center_at1;
    ray_at(center_at0, &sphere->center, 0);
    ray_at(center_at1, &sphere->center, 1);
    point3 temp0, temp1;

    // box at t=0
    aabb_from_points(&box0,
                     subtract(temp0, center_at0, rvec),
                     add(temp1, center_at0, rvec));
    // box at t=1
    aabb_from_points(&box1,
                     subtract(temp0, center_at1, rvec),
                     add(temp1, center_at1, rvec));

    aabb_from_boxes(&sphere->bbox, &box0, &box1);
}

/// @brief Compute uv texture coordinates.
/// @param p a given point on the sphere of radius one, centered at the origin.
/// @param u returned value [0,1] of angle around the Y axis from X=-1(from -X to +Z to +X to -Z back to -X).
/// @param v returned value [0,1] of angle from Y=-1 to Y=+1.
/// @example <1 0 0> yields <0.50 0.50>
/// @example <-1  0  0> yields <0.00 0.50>
/// @example <0 1 0> yields <0.50 1.00>
/// @example < 0 -1  0> yields <0.50 0.00>
/// @example <0 0 1> yields <0.25 0.50>
/// @example < 0  0 -1> yields <0.75 0.50>
/// @remark atan2(0,0) is assumed to be defined as 0 by the implementation.
void get_sphere_uv(const point3 p, double *u, double *v)
{
    double theta = acos(-p[1]);           // acos(-y)
    double phi = atan2(-p[2], p[0]) + pi; // atan2(-z, x) + pi

    *u = phi / (2 * pi);
    *v = theta / pi;
}

/// @brief Detect if the ray hits the sphere (and updates the hit record)
/// @param ray
/// @param ray_interval
/// @param rec the Hit_Record
/// @return bool if sphere was hit by given ray
bool sphere_hit(const struct Sphere *sphere, const struct Ray *ray, struct Interval ray_interval, struct Hit_Record *rec)
{
    // Find out where the sphere center is at this ray's time.
    point3 current_center;
    ray_at(current_center, &sphere->center, ray->tm);

    vec3 diff;
    subtract(diff, current_center, (double *)ray->origin);

    double a = len_squared(ray->direction);
    double h = dot(ray->direction, diff); // Note this is not b.
    double c = len_squared(diff) - (sphere->radius * sphere->radius);
    double discriminant = h * h - a * c;

    if (discriminant < 0)
    {
        return false;
    }

    double sqrtd = sqrt(discriminant);

    // Find the nearest root that lies in the acceptable range.
    double root = (h - sqrtd) / a;
    if (!interval_surrounds(&ray_interval, root))
    {
        root = (h + sqrtd) / a;
        if (!interval_surrounds(&ray_interval, root))
        {
            return false;
        }
    }

    rec->t = root;
    ray_at(rec->p, ray, rec->t);

    // Note that for the normal for a sphere: we can make it into a unit vector by dividing by the sphere radius.
    // This is because the radius is exactly the magnitude of this vector (rec.p - center).
    vec3 outward_normal;
    scale(outward_normal,
          subtract(rec->normal, rec->p, current_center), (1 / sphere->radius));

    set_face_normal(ray, outward_normal, rec);

    get_sphere_uv(outward_normal, &rec->u, &rec->v);

    // Copy a pointer to the Material_Cfg this Sphere has. We won't use the hit record to change the material.
    rec->mat_cfg = (struct Material_Cfg *)sphere->mat_cfg;

    return true;
}

/// @brief Compute the pdf value of a sample according to the sphere sampling PDF in book 3 section 12.3.
/// Used to importance sample a glass sphere or an otherwise important sphere
/// (similar to how we like to light sample quad lights or other quads that we think are important).
/// @remark **This method only works for stationary spheres**.
/// @param sphere The glass sphere.
/// @param origin The hit point of the ray with the non-light (here non glass-sphere) object.
/// @param direction A direction going from origin towards the glass sphere (the sample).
/// @remark Recall we are uniformly sampling the "visible cap" of the sphere from some point outside the sphere.
double sphere_pdf_value(const struct Sphere *sphere, const point3 origin, const vec3 direction)
{
    struct Hit_Record rec;
    struct Ray r;
    memcpy(r.origin, origin, sizeof(double) * 3);
    memcpy(r.direction, direction, sizeof(double) * 3);

    if (!sphere_hit(sphere, &r, (struct Interval){.min = 0.001, .max = infinity}, &rec))
        return 0;

    vec3 temp;
    // (center.at(0) - origin).length_squared();
    double dist_squared = len_squared(
        subtract(temp, ray_at(temp, &sphere->center, 0.0), (double *)origin));

    double cos_theta_max = sqrt(1 - (sphere->radius * sphere->radius / dist_squared));
    double solid_angle = 2 * pi * (1 - cos_theta_max);

    return 1.0 / solid_angle;
}

/// @brief Generates the sample values for the sphere sampling PDF in Book 3: Section 12.3.
/// That is from two uniform random r1,r2 on [0,1] finds a uniform point on the "visible cap" of the sphere.
/// The "visible cap" is what some point outside the sphere, call it P, sees of the sphere.
/// @param radius
/// @param distance_squared the distance between P and the sphere center C squared.
/// @return
static double *random_to_sphere(double radius, double distance_squared, vec3 ret)
{

    double r1 = random_zero_to_one();
    double r2 = random_zero_to_one();
    double z = 1 + r2 * (sqrt(1 - (radius * radius / distance_squared)) - 1);

    double phi = 2 * pi * r1;
    double x = cos(phi) * sqrt(1 - z * z);
    double y = sin(phi) * sqrt(1 - z * z);

    ret[0] = x;
    ret[1] = y;
    ret[2] = z;

    return ret;
}

/// @brief Generate a random sample (a direction from a non-glass sphere object towards the glass sphere) -
/// accroding to the sphere sampling PDF in Book 3 section 12.3.
/// Used to importance sample a glass sphere or an otherwise important sphere
/// (similar to how we like to light sample quad lights or other quads that we think are important).
/// @remark Recall we are uniformly sampling the "visible cap" of the sphere from some point outside the sphere.
/// And we return said point - origin.
/// @remark **This method only works for stationary spheres**.
/// @param sphere The glass sphere (or some other sphere we want to sample towards).
/// @param origin Would usually be the hit point (rec.p) of the ray with the non-glass sphere object.
double *sphere_random(const struct Sphere *sphere, const point3 origin, vec3 ret)
{

    vec3 direction;
    subtract(direction, ray_at(direction, &sphere->center, 0.0), (double *)origin);

    double distance_squared = len_squared(direction);
    struct ONB uvw;
    init_onb(&uvw, direction);

    /*
    What is happening here is this:
    the directions of the rays hitting the visible cap of the glass sphere (from some point P outside the sphere)
    basically correspond to a cone and said cone also corresponds to a solid angle from the ray's origin (P).
    And we previously figured out how to sample solid angles.
    So we sample a solid angle where direction is the normal, and transform that back into our scene
    coordinates.
    */

    return onb_transform(ret, &uvw, random_to_sphere(sphere->radius, distance_squared, ret));
}