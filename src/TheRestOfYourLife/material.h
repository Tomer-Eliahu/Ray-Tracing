#pragma once

#include "vec3.h"
#include "hittable.h"
#include "hittable_list.h"
#include "texture.h"
#include "pdf.h"

/// @brief Note pdf_ptr uses malloc and must later be **freed** by calling free_pdf_ptr.
struct Scatter_Record
{
    color3 attenuation;
    struct PDF *pdf_ptr; //< Uses malloc and **must be freed after use** by calling free_pdf_ptr.

    /// @brief True for specular rays (glass and metal) or sometimes true for a a material
    /// — like varnished wood — that is partially ideal specular (the polish) and partially diffuse (the wood).
    /// Note that for such materials we are basically implictly doing the mixture density approach
    /// (one of the PDFs is essentially 0 in all but one direction, so we can change the diffuse PDF to
    /// be zero in that direction with no noticeable difference in practice). But for the math
    /// to remain correct we need to multiply the specular PDF and the diffuse PDF
    /// by s or (1-s) where s is the prob of a specular ray (according to the formula of mixture density).
    bool skip_pdf;
    struct Ray skip_pdf_ray; //< The specular ray.
};

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
/// @param srec The scatter record which records:
/// @param attenuation The intensity of light lost
bool lambertian_scatter([[maybe_unused]] const struct Ray *r_in, const struct Hit_Record *rec,
                        struct Scatter_Record *srec)
{
    tex_value(srec->attenuation, rec->mat_cfg->object.lambertian.tex, rec->u, rec->v, rec->p);

    // Cosine sampling
    srec->pdf_ptr = new_pdf_ptr();
    *(srec->pdf_ptr) = (struct PDF){.which = Cos_Density};
    init_cos_density(&srec->pdf_ptr->pdf.cos_den, rec->normal);
    srec->skip_pdf = false;
    return true;
}

/*Old version of lambertian_scatter (pre book 3 section 8.3)
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
*/

/// @brief Book 3 Section 6.3 has us do this (use both this and incorrect_lambertian_scattering_pdf).
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
    vec3 temp1, temp2;
    double cos_theta = dot(unit(temp1, (double *)rec->normal),
                           unit(temp2, (double *)scattered->direction));
    return cos_theta < 0 ? 0 : cos_theta / pi;
}

/// @brief Book 3 Section 6.3 has us do this (use both this and incorrect_lambertian_scatter).
double incorrect_lambertian_scattering_pdf([[maybe_unused]] const struct Ray *r_in,
                                           [[maybe_unused]] const struct Hit_Record *rec,
                                           [[maybe_unused]] const struct Ray *scattered)
{
    // Uniform pdf on hemisphere surface.
    return 1.0 / (2 * pi);
}

/// @brief Metal material reflectance
/// @param r_in Incoming ray
/// @param srec The scatter record which records:
/// @param attenuation The intensity of light lost
/// @param skip_pdf_ray The outbound ray from hitting this material
bool metal_scatter(const struct Ray *r_in, const struct Hit_Record *rec, struct Scatter_Record *srec)
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

    // Note that if the fuzziness is nonzero, this surface isn’t really ideally specular,
    // but the implicit sampling works just like it did before.
    // We're effectively skipping all of our PDF work for the materials that we're treating specularly.

    memcpy(srec->attenuation, rec->mat_cfg->object.metal.albedo, 3 * sizeof(double));

    srec->pdf_ptr = nullptr;
    srec->skip_pdf = true;

    memcpy(srec->skip_pdf_ray.origin, rec->p, 3 * sizeof(double));
    memcpy(srec->skip_pdf_ray.direction, reflected, 3 * sizeof(double));
    srec->skip_pdf_ray.tm = r_in->tm;

    // Return true only if we scatter above the surface (adding fuzz may mean we scatter below it).
    // If we scatter below, we simply will absorb the incoming ray.
    return (dot(srec->skip_pdf_ray.direction, rec->normal) > 0);
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
/// @param srec The scatter record which records:
/// @param attenuation The intensity of light lost
/// @param skip_pdf_ray The outbound ray from hitting this material
bool dielectric_scatter(const struct Ray *r_in, const struct Hit_Record *rec, struct Scatter_Record *srec)
{
    // Set to white
    srec->attenuation[0] = 1.0;
    srec->attenuation[1] = 1.0;
    srec->attenuation[2] = 1.0;

    srec->pdf_ptr = nullptr;
    srec->skip_pdf = true;

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
        reflect(srec->skip_pdf_ray.direction, unit_direction, rec->normal);
    }
    else
    {
        refract(srec->skip_pdf_ray.direction, unit_direction, rec->normal, ri);
    }

    memcpy(srec->skip_pdf_ray.origin, rec->p, 3 * sizeof(double));
    srec->skip_pdf_ray.tm = r_in->tm;

    return true;
}

/// @brief Isotropic material reflectance
/// @param r_in Incoming ray
/// @param attenuation The intensity of light lost
/// @remark Remember an Isotropic material is a material which scatters rays
/// that hit it in a uniform (3D) random direction.
bool isotropic_scatter([[maybe_unused]] const struct Ray *r_in, const struct Hit_Record *rec,
                       struct Scatter_Record *srec)
{
    tex_value(srec->attenuation, rec->mat_cfg->object.isotropic.tex, rec->u, rec->v, rec->p);
    srec->pdf_ptr = new_pdf_ptr();
    *(srec->pdf_ptr) = (struct PDF){.which = Uniform_Sphere_PDF};
    srec->skip_pdf = false;
    return true;
}

/// @brief pScatter(..) = 1.0 / (4.0 * pi).
/// Remember an Isotropic material is
/// a material which scatters rays that hit it in a uniform (3D) random direction.
double isotropic_scattering_pdf([[maybe_unused]] const struct Ray *r_in,
                                [[maybe_unused]] const struct Hit_Record *rec,
                                [[maybe_unused]] const struct Ray *scattered)
{
    return 1.0 / (4.0 * pi);
}

/// @brief Returns the light emitted by the Diffuse_Light material.
static inline void emitted(color3 ret, const struct Hit_Record *rec,
                           [[maybe_unused]] const struct Ray *r_in, double u, double v, const point3 p)
{
    // The light is two-sided and we want to have the light just emit down.
    if (!rec->front_face)
    {
        ret[0] = 0;
        ret[1] = 0;
        ret[2] = 0;
        return;
    }

    const struct Diffuse_Light *light = &(rec->mat_cfg->object.light);
    tex_value(ret, light->tex, u, v, p);
}