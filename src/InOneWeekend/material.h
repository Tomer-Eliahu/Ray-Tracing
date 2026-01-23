#pragma once

#include "vec3.h"
#include "hittable.h"

/*

Here and throughout these books we will use the term albedo (Latin for “whiteness”).
Albedo is a precise technical term in some disciplines,
but in all cases it is used to define some form of fractional reflectance.
Albedo will vary with material color and (as we will later implement for glass materials)
can also vary with incident viewing direction (the direction of the incoming ray).

*/

enum Material
{
    Lambertian,
    Metal,
    Dielectric,
};

struct Material_Cfg
{

    enum Material mat; //< The type of this material

    /// @brief Albedo is the measure of a surface's reflectivity,
    /// representing the fraction of sunlight (or other radiation) that is reflected,
    /// ranging from 0 (no reflection, black) to 1 (total reflection, white).
    /// Note that this is done across RGB (color3) as opposed to the x-y-z axes.
    color3 albedo;
    double fuzz; //< Controls how fuzzy the reflection is (only for Metal).

    /// (For dielectric)
    /// Refractive index in vacuum or air, or the ratio of the material's refractive index over
    /// the refractive index of the enclosing media
    double refraction_index;
};

/// @brief Lambertian (diffuse) material reflectance
/// @param r_in Incoming ray
/// @param attenuation The intensity of light lost
/// @param scattered The outbound ray from hitting this material
/// @return
bool lambertian_scatter(const struct Ray *r_in, const struct Hit_Record *rec,
                        color3 attenuation, struct Ray *scattered)
{

    // Unused
    (void)r_in;

    // Find scatter direction
    random_unit_vector(scattered->direction);
    add(scattered->direction, (double *)rec->normal, scattered->direction);

    // Catch degenerate scatter direction (if the random vector is almost exactly the opposite of the normal)
    if (near_zero(scattered->direction))
    {
        memcpy(scattered->direction, rec->normal, 3 * sizeof(double));
    }

    memcpy(scattered->origin, rec->p, 3 * sizeof(double));
    memcpy(attenuation, rec->mat_cfg->albedo, 3 * sizeof(double));
    return true;
}

/// @brief Metal material reflectance
/// @param r_in Incoming ray
/// @param attenuation The intensity of light lost
/// @param scattered The outbound ray from hitting this material
/// @return
bool metal_scatter(const struct Ray *r_in, const struct Hit_Record *rec,
                   color3 attenuation, struct Ray *scattered)
{
    vec3 reflected;
    reflect(reflected, (double *)r_in->direction, rec->normal);

    // In order for the fuzz sphere to make sense,
    // it needs to be consistently scaled compared to the reflection vector,
    // we thus normalize the reflected ray.
    vec3 fuzz_applied;
    random_unit_vector(fuzz_applied);
    add(reflected, unit(reflected, reflected),
        scale(fuzz_applied, fuzz_applied, rec->mat_cfg->fuzz));

    memcpy(scattered->origin, rec->p, 3 * sizeof(double));
    memcpy(scattered->direction, reflected, 3 * sizeof(double));

    memcpy(attenuation, rec->mat_cfg->albedo, 3 * sizeof(double));

    // Return true only if we scatter above the surface (adding fuzz may mean we scatter below it).
    // If we scatter below, we simply will absorb the incoming ray.
    return (dot(scattered->direction, rec->normal) > 0);
}

/// @brief Use Schlick's approximation for reflectance.
/// @param cosine
/// @param refraction_index
/// @return
/// @remarks Now real glass has reflectivity that varies with angle —
/// look at a window at a steep angle and it becomes a mirror. There is a big ugly equation for that,
/// but almost everybody uses a cheap and surprisingly accurate polynomial approximation by Christophe Schlick.
static double reflectance(double cosine, double refraction_index)
{
    double r0 = (1 - refraction_index) / (1 + refraction_index);
    r0 = r0 * r0;
    return r0 + (1 - r0) * pow((1 - cosine), 5);
}

// Clear materials such as water, glass, and diamond are dielectrics.
// When a light ray hits them, it splits into a reflected ray and a refracted (transmitted) ray.
// We’ll handle that by randomly choosing between reflection and refraction,
// only generating one scattered ray per interaction.

/// @brief Dielectric material *refraction*
/// @param r_in Incoming ray
/// @param attenuation The intensity of light lost
/// @param scattered The outbound ray from hitting this material
bool dielectric_scatter(const struct Ray *r_in, const struct Hit_Record *rec,
                        color3 attenuation, struct Ray *scattered)
{
    // Set to white
    attenuation[0] = 1.0;
    attenuation[1] = 1.0;
    attenuation[2] = 1.0;

    double ri = rec->front_face ? (1.0 / rec->mat_cfg->refraction_index) : rec->mat_cfg->refraction_index;

    vec3 unit_direction;
    unit(unit_direction, (double *)r_in->direction);

    vec3 temp;
    double cos_theta = fmin(dot(negate(temp, unit_direction), rec->normal), 1.0);
    double sin_theta = sqrt(1.0 - (cos_theta * cos_theta));

    bool cannot_refract = ri * sin_theta > 1.0;

    if (cannot_refract || reflectance(cos_theta, ri) > random_zero_to_one())
    {
        reflect(scattered->direction, unit_direction, rec->normal);
    }
    else
    {
        refract(scattered->direction, unit_direction, rec->normal, ri);
    }

    memcpy(scattered->origin, rec->p, 3 * sizeof(double));

    return true;
}