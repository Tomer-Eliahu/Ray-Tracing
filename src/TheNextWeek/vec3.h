#pragma once

#include <math.h>
#include <stdio.h>
#include <string.h>

typedef double vec3[3];

typedef vec3 point3;
typedef vec3 color3;

// Note that we are careful to always return a vec3 (the array where we want to store the result),
// so that we can easily chain computations together.
// We can always just ignore the return value and it won't matter.

/// @brief Negate a vector (multiply each coordinate by -1).
/// @remark Note that ret can potentially be equal to vec (this would mean we modify vec in place).
double *negate(vec3 ret, vec3 vec)
{
    for (int i = 0; i < 3; i++)
    {
        ret[i] = -1 * vec[i];
    }

    return ret;
}

/// @brief Add vec2 to vec1.
double *add(vec3 ret, vec3 vec1, vec3 vec2)
{
    for (int i = 0; i < 3; i++)
    {
        ret[i] = vec1[i] + vec2[i];
    }
    return ret;
}

/// @brief Subtract vec2 from vec1.
double *subtract(vec3 ret, vec3 vec1, vec3 vec2)
{
    for (int i = 0; i < 3; i++)
    {
        ret[i] = vec1[i] - vec2[i];
    }

    return ret;
}

/// @brief Scale vec by a scalar t.
/// @remark Note that ret can potentially be equal to vec (this would mean we modify vec in place).
double *scale(vec3 ret, vec3 vec, double t)
{
    for (int i = 0; i < 3; i++)
    {
        ret[i] = vec[i] * t;
    }

    return ret;
}

/// @brief Multiply vec1 by vec2 element wise.
double *multiply(vec3 ret, vec3 vec1, vec3 vec2)
{
    for (int i = 0; i < 3; i++)
    {
        ret[i] = vec1[i] * vec2[i];
    }

    return ret;
}

/// @brief Returns the length (also called the magnitude) of a vec3.
static inline double len(const vec3 vec)
{
    return sqrt((vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]));
}

/// @brief Returns the square of the length of a vec3.
static inline double len_squared(const vec3 vec)
{
    return (vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
}

/// @brief Print the vec3.
static inline void print(const vec3 vec)
{
    printf("[%f, %f, %f]", vec[0], vec[1], vec[2]);
}

/// @brief Get a unit vector in vec's direction.
/// @remark Note that ret can potentially be equal to vec (this would mean we modify vec in place).
/// @remark Assumes vec is not a zero vector (so it has a magnitude different than 0).
double *unit(vec3 ret, vec3 vec)
{
    double magnitude = len(vec);
    for (int i = 0; i < 3; i++)
    {
        ret[i] = vec[i] / magnitude;
    }
    return ret;
}

/// @brief Returns the dot product of two vec3.
static inline double dot(const vec3 u, const vec3 v)
{
    return u[0] * v[0] + u[1] * v[1] + u[2] * v[2];
}

/// @brief The cross product of two vec3.
double *cross(vec3 ret, vec3 vec1, vec3 vec2)
{
    ret[0] = vec1[1] * vec2[2] - vec1[2] * vec2[1];
    ret[1] = vec1[2] * vec2[0] - vec1[0] * vec2[2];
    ret[2] = vec1[0] * vec2[1] - vec1[1] * vec2[0];
    return ret;
}

/// @brief Make a vector random in { [0,1), [0,1), [0,1) }.
static inline void vec_rand_zero_to_one(vec3 vec)
{
    vec[0] = random_zero_to_one();
    vec[1] = random_zero_to_one();
    vec[2] = random_zero_to_one();
}

/// @brief Make a vector random in { [min,max), [min,max), [min,max) }.
static inline void vec_rand_in_range(vec3 vec, double min, double max)
{
    vec[0] = random_in_range(min, max);
    vec[1] = random_in_range(min, max);
    vec[2] = random_in_range(min, max);
}

/// @brief Generate a random vector on the surface of the unit sphere.
/// @remark Rejecting a random vector outside the unit sphere (rejection sampling)
/// ensures a uniform distribution of sample directions on the surface of the unit sphere.
static inline void random_unit_vector(vec3 vec)
{
    while (true)
    {
        vec_rand_in_range(vec, -1, 1);
        double lensq = len_squared(vec);
        if (1e-160 < lensq && lensq <= 1)
        {
            // Normalize the vector
            scale(vec, vec, (1 / sqrt(lensq)));
            return;
        }
    }
}

/// @brief Generate a random vector on the hemisphere (facing the same direction as the surface normal).
/// @param normal
static inline void random_on_hemisphere(vec3 rand_vec, const vec3 normal)
{
    random_unit_vector(rand_vec);

    if (dot(rand_vec, normal) <= 0.0) // NOT in the same hemisphere as the normal
    {
        negate(rand_vec, rand_vec);
    }
}

/// @brief Generate a random vector on the unit disk.
/// @remark Rejecting a random vector outside the unit square (rejection sampling)
/// ensures a uniform distribution of sample directions on the surface of the unit disk.
static inline void random_in_unit_disk(vec3 vec)
{
    // The z coordinate stays fixed at 0.
    vec[2] = 0;

    while (true)
    {
        vec[0] = random_in_range(-1, 1);
        vec[1] = random_in_range(-1, 1);
        if (len_squared(vec) < 1)
        {
            return;
        }
    }
}

/// @brief Reflect the vector vec (possibly in place) off a surface with the given surface normal.
static inline double *reflect(vec3 ret, vec3 vec, const vec3 normal)
{
    return subtract(ret, vec, scale(ret, (double *)normal, 2 * dot(vec, normal)));
}

/// @brief Refract (different than reflect; see remark) a given vector.
/// @param ret
/// @param uv The incident (incoming) ray.
/// @param normal
/// @param etai_over_etat
/// @return The refracted vector.
/// @remark a reflected ray hits a surface and then “bounces” off in a new direction.
/// A refracted ray bends as it transitions from a material's surroundings into the material itself
/// (as with glass or water). See 11.2 for details behind this calculation.
static inline double *refract(vec3 ret, const vec3 uv, const vec3 normal, double etai_over_etat)
{
    vec3 temp;
    double cos_theta = fmin(dot(negate(temp, (double *)uv), normal), 1.0);

    vec3 r_out_perp;
    scale(r_out_perp,
          add(r_out_perp, (double *)uv,
              scale(r_out_perp, (double *)normal, cos_theta)),
          etai_over_etat);

    vec3 r_out_parallel;
    scale(r_out_parallel, (double *)normal, -sqrt(fabs(1.0 - len_squared(r_out_perp))));

    return add(ret, r_out_perp, r_out_parallel);
}

/// @brief Return true if the vector is close to zero in all dimensions.
bool near_zero(const vec3 vec)
{
    double s = 1e-8;
    return (fabs(vec[0]) < s) && (fabs(vec[1]) < s) && (fabs(vec[2]) < s);
}