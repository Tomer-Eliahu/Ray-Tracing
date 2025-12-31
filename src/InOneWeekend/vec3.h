#pragma once

#include <math.h>
#include <stdio.h>
#include <string.h>

typedef double vec3[3];

typedef vec3 point3;
typedef vec3 color3;

/// @brief Negate a vector in place (multiply each coordinate by -1).
void negate(vec3 vec)
{
    for (int i = 0; i < 3; i++)
    {
        vec[i] *= -1;
    }
}

/// @brief Add vec2 to vec1 modifying vec1 in place.
void add(vec3 vec1, const vec3 vec2)
{
    for (int i = 0; i < 3; i++)
    {
        vec1[i] += vec2[i];
    }
}

/// @brief Subtract vec2 from vec1 modifying vec1 in place.
void subtract(vec3 vec1, const vec3 vec2)
{
    for (int i = 0; i < 3; i++)
    {
        vec1[i] -= vec2[i];
    }
}

/// @brief Scale vec by a scalar t (modifies vec in place).
void scale(vec3 vec, double t)
{
    for (int i = 0; i < 3; i++)
    {
        vec[i] *= t;
    }
}

/// @brief Multiply vec1 by vec2 element wise (vec1[i]*= vec2[i]]).
/// Modifies vec1 in place.
void multiply(vec3 vec1, const vec3 vec2)
{
    for (int i = 0; i < 3; i++)
    {
        vec1[i] *= vec2[i];
    }
}

/// @brief Returns the length of a vec3.
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

/// @brief Make vec into a unit vector (in-place).
void unit(vec3 vec)
{
    double magnitude = len(vec);
    for (int i = 0; i < 3; i++)
    {
        vec[i] /= magnitude;
    }
}

/// @brief Returns the dot product of two vec3.
static inline double dot(const vec3 u, const vec3 v)
{
    return u[0] * v[0] + u[1] * v[1] + u[2] * v[2];
}

/// @brief The cross product of two vec3 (constructs a new vector).
#define CROSS_PRODUCT(vec1, vec2)           \
    {vec1[1] * vec2[2] - vec1[2] * vec2[1], \
     vec1[2] * vec2[0] - vec1[0] * vec2[2], \
     vec1[0] * vec2[1] - vec1[1] * vec2[0]}
