#pragma once
#include "rtweekend.h"
#include "hittable_list.h"
#include "quad.h"
#include "onb.h"

/*Book 3: Section 10: Mixture Densities */

// We are making sample PDF's. We want them to have 2 purposes:
// Return a sample according to this sample PDF
// Return the sampling_pdf value that a certain sample corresponds to.

// Forward Declarations
struct PDF;
double pdf_value(const struct PDF *gen_pdf, const vec3 direction);
double *pdf_generate(const struct PDF *gen_pdf, vec3 ret);

/// @brief Returns the sampling_pdf value for the sample direction.
/// The sampling pdf is uniform sampling over the unit sphere.
double uniform_sphere_pdf_value([[maybe_unused]] const vec3 dir)
{
    return 1.0 / (4.0 * pi);
}

/// @brief Returns a sample according to a uniform distribution on the surface of the unit sphere.
double *uniform_sphere_pdf_generate(vec3 ret)
{
    random_unit_vector(ret);
    return ret;
}

/// @brief A sampling PDF of cos density about a surface normal (or some other vector) n.
/// Must be init with init_cos_density.
struct Cos_Density
{
    struct ONB onb;
};

void init_cos_density(struct Cos_Density *cos_den, const vec3 n)
{
    init_onb(&cos_den->onb, n);
}

/// @brief Returns the sampling PDF (that has cos_density) value for the sample direction.
double cos_density_value(const struct Cos_Density *cos_den, const vec3 direction)
{
    // The pdf value is cos(θ)/π where θ is the angle
    // betwen the surface normal and the scattered direction in our scene coordinates

    vec3 temp;
    // Because these are 2 unit vectors.
    double cosine_theta = dot(unit(temp, (double *)direction), cos_den->onb.axis[2]);
    return fmax(0, cosine_theta / pi);
}

/// @brief Generate and return a sample ret according to the cos_density PDF.
double *cos_density_generate(const struct Cos_Density *cos_den, vec3 ret)
{
    vec3 rand_vec;
    random_cosine_direction(rand_vec);
    return onb_transform(ret, &cos_den->onb, rand_vec);
}

/// @brief Sample directions towards a hittable, like the light, or a glass sphere (towards something important).
struct Hittable_PDF
{
    /// @brief The hittable objects (can be more than 1 if you use a **malloced array**).
    /// Make sure each hittable is **properly initialized** and is the object you want to sample towards.
    /// That means you are probably not going to be using Hittables that are Composite_Hittable most of the time
    /// but rather are things like Quads and Spheres.
    struct Hittable *objects;
    int size; //< The number of hittable objects.
    point3 origin;
};

/*

We only need to add pdf_value() and random() to quad because we're using this to importance sample the light,
and the only light we have in our scene is a quad. if you want other light geometries,
or want to use a PDF with other objects,
you'll need to implement the above functions for the corresponding Hittables.

*/

double
hittable_pdf_value(const struct Hittable_PDF *hit_pdf, const vec3 direction)
{
    // Calculate according to the Mixture PDF value if we have multiple objects.
    double weight = 1.0 / hit_pdf->size;
    double sum = 0.0;

    for (int i = 0; i < hit_pdf->size; i++)
    {
        switch (hit_pdf->objects[i].which)
        {
        case (enum Which_Hittable)Quad:

            sum += weight * quad_pdf_value(&hit_pdf->objects[i].object.quad, hit_pdf->origin, direction);
            break;

        case (enum Which_Hittable)Sphere:

            sum += weight * sphere_pdf_value(&hit_pdf->objects[i].object.sphere, hit_pdf->origin, direction);
            break;

        default:

            fprintf(stderr, "Not a valid Which_Hittable kind in hittable_pdf_value!\n");
            fflush(stderr);
            exit(EXIT_FAILURE);
            break;
        }
    }

    return sum;
}

double *hittable_pdf_generate(const struct Hittable_PDF *hit_pdf, vec3 ret)
{
    // In the book this is return objects.random(origin);

    // If we have multiple objects generate according to their Mixture PDF
    int index = random_int(0, hit_pdf->size - 1);

    switch (hit_pdf->objects[index].which)
    {
    case (enum Which_Hittable)Quad:

        return quad_random(&hit_pdf->objects[index].object.quad, hit_pdf->origin, ret);
        break;

    case (enum Which_Hittable)Sphere:

        return sphere_random(&hit_pdf->objects[index].object.sphere, hit_pdf->origin, ret);
        break;

    default:
        ret[0] = 1;
        ret[1] = 0;
        ret[2] = 0;
        return ret;
        break;
    }
}

/// @brief Recall any weighted average (as long as the weights are positive and add up to one)
/// of PDFs is also a PDF.
/// This in particular, is Mixture_pdf = 0.5*p_1 + 0.5*p_2 for PDFs p_1 and p_2.
struct Mixture_PDF
{
    struct PDF *p[2];
};

/// @brief Returns the sampling mixture_pdf value for the sample direction.
double mixture_pdf_value(const struct Mixture_PDF *mix_pdf, const vec3 direction)
{
    return 0.5 * pdf_value(mix_pdf->p[0], direction) + 0.5 * pdf_value(mix_pdf->p[1], direction);
}

/// @brief Generate and return a sample ret according to the mixture PDF.
/// @remark Instead of generating 2 rays and averaging them, we could just flip a coin
/// and half the time return a ray accroding to p_1 and half the time according to p_2.
double *mixture_pdf_generate(const struct Mixture_PDF *mix_pdf, vec3 ret)
{
    if (random_zero_to_one() < 0.5)
        return pdf_generate(mix_pdf->p[0], ret);
    else
        return pdf_generate(mix_pdf->p[1], ret);
}

enum Which_PDF
{
    Uniform_Sphere_PDF,
    Cos_Density,
    Hittable_PDF,
    Mixture_PDF,
};

/// @remark Note the sphere_pdf has no underlying struct (not needed).
union Underlying_PDF
{
    struct Cos_Density cos_den;
    struct Hittable_PDF hittable_pdf;
    struct Mixture_PDF mixture_pdf;
};

/// @brief An interface to all PDFs.
struct PDF
{
    enum Which_PDF which;
    union Underlying_PDF pdf;
};

/// @brief Returns the sampling pdf value for the sample direction.
double pdf_value(const struct PDF *gen_pdf, const vec3 direction)
{
    switch (gen_pdf->which)
    {
    case (enum Which_PDF)Uniform_Sphere_PDF:

        return uniform_sphere_pdf_value(direction);
        break;

    case (enum Which_PDF)Cos_Density:

        return cos_density_value(&gen_pdf->pdf.cos_den, direction);
        break;

    case (enum Which_PDF)Hittable_PDF:

        return hittable_pdf_value(&gen_pdf->pdf.hittable_pdf, direction);
        break;

    case (enum Which_PDF)Mixture_PDF:

        return mixture_pdf_value(&gen_pdf->pdf.mixture_pdf, direction);
        break;

    default:
        fprintf(stderr, "Could not identify PDF kind in pdf_value!\n");
        fflush(stderr);
        exit(EXIT_FAILURE);
        break;
    }
}

/// @brief Generate and return a sample ret according to the PDF.
double *pdf_generate(const struct PDF *gen_pdf, vec3 ret)
{

    switch (gen_pdf->which)
    {
    case (enum Which_PDF)Uniform_Sphere_PDF:

        return uniform_sphere_pdf_generate(ret);
        break;

    case (enum Which_PDF)Cos_Density:

        return cos_density_generate(&gen_pdf->pdf.cos_den, ret);
        break;

    case (enum Which_PDF)Hittable_PDF:

        return hittable_pdf_generate(&gen_pdf->pdf.hittable_pdf, ret);
        break;

    case (enum Which_PDF)Mixture_PDF:

        return mixture_pdf_generate(&gen_pdf->pdf.mixture_pdf, ret);
        break;

    default:
        fprintf(stderr, "Could not identify PDF kind in pdf_generate!\n");
        fflush(stderr);
        exit(EXIT_FAILURE);
        break;
    }
}

/// @brief Malloc a new pointer to a PDF and returns said ptr.
/// The caller must later **free** this ptr which is done by calling free_pdf_ptr.
struct PDF *new_pdf_ptr()
{
    struct PDF *p = malloc(sizeof(struct PDF));
    if (p == NULL)
    {
        fprintf(stderr, "Could not malloc struct PDF in new_pdf_ptr!\n");
        fflush(stderr);
        exit(EXIT_FAILURE);
    }
    return p;
}

/// @brief Takes a malloced ptr to a struct PDF and frees it.
/// Note that as p can point to other struct PDF's in it (if it is a Mixture_PDF),
/// this function recurses as needed and assumes all inner PDF's are also malloced.
/// If inner PDF's are not malloced, just call free(p) instead of this function.
void free_pdf_ptr(struct PDF *p)
{
    if (p == NULL)
        return;

    switch (p->which)
    {
    case (enum Which_PDF)Uniform_Sphere_PDF:
        // has no associated struct.
        free(p);
        return;
        break;

    case (enum Which_PDF)Cos_Density:
        // struct Cos_Density is fully stack allocated.
        free(p);
        return;
        break;

    case (enum Which_PDF)Hittable_PDF:
        // as objects in struct Hittable_PDF point to a global variable, this is enough.
        free(p);
        return;
        break;

    case (enum Which_PDF)Mixture_PDF:
        free_pdf_ptr(p->pdf.mixture_pdf.p[0]);
        free_pdf_ptr(p->pdf.mixture_pdf.p[1]);
        free(p);
        return;
        break;

    default:
        fprintf(stderr, "Could not identify PDF kind in free_pdf_ptr!\n");
        fflush(stderr);
        exit(EXIT_FAILURE);
        break;
    }
}