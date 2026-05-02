#pragma once

#include "vec3.h"
#include "hittable.h"
#include "texture.h"

struct Lambertian
{
    struct Texture *tex;
};

struct Metal
{
    /// @brief Albedo is the measure of a surface's reflectivity,
    /// representing the fraction of sunlight (or other radiation) that is reflected,
    /// ranging from 0 (no reflection, black) to 1 (total reflection, white).
    /// Note that this is done across RGB (color3) as opposed to the x-y-z axes.
    color3 albedo;
    double fuzz; //< Controls how fuzzy the reflection is (only for Metal).
};

struct Dielectric
{
    /// Refractive index in vacuum or air, or the ratio of the material's refractive index over
    /// the refractive index of the enclosing media
    double refraction_index;
};

/// @brief A material which emits light.
struct Diffuse_Light
{
    struct Texture *tex;
};

/// @brief A material which scatters rays that hit it in a uniform (3D) random direction.
struct Isotropic
{
    struct Texture *tex;
};

union Material
{
    struct Lambertian lambertian;
    struct Metal metal;
    struct Dielectric dielectric;
    struct Diffuse_Light light;
    struct Isotropic isotropic;
};

/*

Here and throughout these books we will use the term albedo (Latin for “whiteness”).
Albedo is a precise technical term in some disciplines,
but in all cases it is used to define some form of fractional reflectance.
Albedo will vary with material color and (as we will later implement for glass materials)
can also vary with incident viewing direction (the direction of the incoming ray).

*/

enum Which_Material
{
    Lambertian,
    Metal,
    Dielectric,
    Light,
    Isotropic
};

/// @brief An interface to all materials.
struct Material_Cfg
{
    enum Which_Material which;
    union Material object;
};

/// @brief Lambertian (diffuse) material reflectance
/// @param r_in Incoming ray
/// @param attenuation The intensity of light lost
/// @param scattered The outbound ray from hitting this material
/// @return
bool lambertian_scatter(const struct Ray *r_in, const struct Hit_Record *rec,
                        color3 attenuation, struct Ray *scattered)
{
    // Find scatter direction
    random_unit_vector(scattered->direction);
    add(scattered->direction, (double *)rec->normal, scattered->direction);

    // Catch degenerate scatter direction (if the random vector is almost exactly the opposite of the normal)
    if (near_zero(scattered->direction))
    {
        memcpy(scattered->direction, rec->normal, 3 * sizeof(double));
    }

    memcpy(scattered->origin, rec->p, 3 * sizeof(double));
    scattered->tm = r_in->tm;

    tex_value(attenuation, rec->mat_cfg->object.lambertian.tex, rec->u, rec->v, rec->p);
    return true;
}

/// @brief Book 3 Section 6.3 has us do this.
/// @remark The point of section 6.3 is that even though the pScatter and p will cancel out, because
/// we now generate the scattering rays differently (use random_on_hemisphere in incorrect_lambertian_scatter)
/// The image will be *materially* different.
bool incorrect_lambertian_scatter(const struct Ray *r_in, const struct Hit_Record *rec,
                                  color3 attenuation, struct Ray *scattered)
{
    // Section 6.3: incorrect for Lambertian.
    // We replace Lambertian diffuse with uniform hemispherical diffuse material.
    random_on_hemisphere(scattered->direction, rec->normal);

    // Catch degenerate scatter direction (if the random vector is almost exactly the opposite of the normal)
    if (near_zero(scattered->direction))
    {
        memcpy(scattered->direction, rec->normal, 3 * sizeof(double));
    }

    memcpy(scattered->origin, rec->p, 3 * sizeof(double));
    scattered->tm = r_in->tm;

    tex_value(attenuation, rec->mat_cfg->object.lambertian.tex, rec->u, rec->v, rec->p);
    return true;
}

/// @brief For the Lambertian material, this PDF is pScatter(..) = cos(θo)/π where θo
/// is the angle between the surface normal and the scattering direction.
/// @param r_in Incoming ray
/// @param rec
/// @param scattered The outbound ray from hitting this material.
/// Must not be the zero vector (you can ensure this by calling lambertian_scatter first).
/// @return
/// @remark This is exactly how we *implictly* sampled the scattering direction in lambertian_scatter (I verified this).
/// That means that we made p(ωo)=cos(θo)/π
/// (our sampling PDF which is the only thing we can set to whatever PDF we want).
/// As pScatter(..) = p(..), these terms canceled out and so we had Color_o(x,ωo,λ) ≈ average of ( A(…)⋅Color_i(…) )
/// (or in code form ray_color = attenuation * next_ray_color(..) and we average that later).
double lambertian_scattering_pdf([[maybe_unused]] const struct Ray *r_in,
                                 const struct Hit_Record *rec, const struct Ray *scattered)
{
    vec3 temp;
    double cos_theta = dot(rec->normal, unit(temp, (double *)scattered->direction));
    return cos_theta < 0 ? 0 : cos_theta / pi;
}

/// @brief Book 3 Section 6.3 has us do this.
/// @remark The point of section 6.3 is that even though the pScatter and p will cancel out, because
/// we now generate the scattering rays differently (use random_on_hemisphere in incorrect_lambertian_scatter)
/// The image will be *materially* different.
double incorrect_lambertian_scattering_pdf([[maybe_unused]] const struct Ray *r_in,
                                           [[maybe_unused]] const struct Hit_Record *rec,
                                           [[maybe_unused]] const struct Ray *scattered)
{
    // Uniform pdf on hemisphere surface.
    return 1.0 / (2 * pi);
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

    // In order for the fuzz to make sense,
    // it needs to be consistently scaled compared to the reflection vector,
    // we thus normalize the reflected ray.
    vec3 fuzz_applied;
    random_unit_vector(fuzz_applied);
    add(reflected, unit(reflected, reflected),
        scale(fuzz_applied, fuzz_applied, rec->mat_cfg->object.metal.fuzz));

    memcpy(scattered->origin, rec->p, 3 * sizeof(double));
    memcpy(scattered->direction, reflected, 3 * sizeof(double));
    scattered->tm = r_in->tm;

    memcpy(attenuation, rec->mat_cfg->object.metal.albedo, 3 * sizeof(double));

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

    double refraction_index = rec->mat_cfg->object.dielectric.refraction_index;

    double ri = rec->front_face ? (1.0 / refraction_index) : refraction_index;

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
    scattered->tm = r_in->tm;

    return true;
}

/// @brief Isotropic material reflectance
/// @param r_in Incoming ray
/// @param attenuation The intensity of light lost
/// @param scattered The outbound ray from hitting this material
bool isotropic_scatter(const struct Ray *r_in, const struct Hit_Record *rec,
                       color3 attenuation, struct Ray *scattered)
{
    memcpy(scattered->origin, rec->p, sizeof(double) * 3);
    random_unit_vector(scattered->direction);
    scattered->tm = r_in->tm;

    tex_value(attenuation, rec->mat_cfg->object.isotropic.tex, rec->u, rec->v, rec->p);
    return true;
}

/// @brief Returns the light emitted by the Diffuse_Light material.
static inline void emitted(color3 ret, const struct Diffuse_Light *light, double u, double v, const point3 p)
{
    tex_value(ret, light->tex, u, v, p);
}