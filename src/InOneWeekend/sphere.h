#pragma once

#include "hittable.h"
#include "hittable_list.h"
#include "vec3.h"
#include "ray.h"

struct Sphere
{
    point3 center;
    double radius; //< Must be 0<=
};

/// @brief Sets the hit record normal vector.
/// @param ray
/// @param outward_normal Assumed to have unit length!
/// @param rec
void sphere_set_face_normal(const struct Ray *ray,
                            const vec3 outward_normal, struct Hit_Record *rec)
{

    // The dot product will be positive if the ray goes in the same direction of the outward normal.
    // This happens if the ray travels from inside the sphere out.
    rec->front_face = dot(ray->direction, outward_normal) < 0;
    // We make sure the normal always goes against the ray
    rec->front_face ? memcpy(rec->normal, outward_normal, 3 * sizeof(double)) : negate(rec->normal, (double *)outward_normal);
}

/// @brief dedect if the ray hits the sphere
/// @param ray
/// @param ray_interval
/// @param rec the Hit_Record
/// @return bool if sphere was hit by given ray
bool sphere_hit(const struct Sphere *sphere, const struct Ray *ray, struct Interval ray_interval, struct Hit_Record *rec)
{

    vec3 diff;
    subtract(diff, (double *)sphere->center, (double *)ray->origin);

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
    // This is because the radius is exactly the magnitude this vector (rec.p - center).
    vec3 outward_normal;
    scale(outward_normal,
          subtract(rec->normal, rec->p, (double *)sphere->center), (1 / sphere->radius));

    sphere_set_face_normal(ray, outward_normal, rec);

    return true;
}
