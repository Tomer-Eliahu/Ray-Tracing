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
