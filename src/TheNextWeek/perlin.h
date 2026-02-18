#pragma once

// Book 2: Section 5: Perlin Noise

#include "rtweekend.h"
#include <assert.h>

#define POINT_COUNT 256

struct Perlin_Info
{
    vec3 randvec[POINT_COUNT]; //< Random unit vectors
    int perm_x[POINT_COUNT];
    int perm_y[POINT_COUNT];
    int perm_z[POINT_COUNT];
};

/// @brief Returns a real in [-1, +1].
double perlin_interp(const double *c[2][2][2], double u, double v, double w)
{
    // A Hermite cubic to round off the interpolation
    double uu = u * u * (3.0 - 2.0 * u); // Still in [0, 1) given u is in [0,1).
    double vv = v * v * (3.0 - 2.0 * v);
    double ww = w * w * (3.0 - 2.0 * w);

    double accum = 0.0;

    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 2; j++)
            for (int k = 0; k < 2; k++)
            {
                vec3 weight_v = {u - i, v - j, w - k};
                accum += (i * uu + (1.0 - i) * (1.0 - uu)) *
                         (j * vv + (1.0 - j) * (1.0 - vv)) *
                         (k * ww + (1.0 - k) * (1.0 - ww)) *
                         dot(c[i][j][k], weight_v);
            }

    return accum;
}

/*
/// @brief OLD-- Part of section 5. We later replaced using this with using perlin_interp.
double trilinear_interp(double c[2][2][2], double u, double v, double w)
{
    double accum = 0.0;
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 2; j++)
            for (int k = 0; k < 2; k++)
            {
                accum += (i * u + (1 - i) * (1 - u)) *
                         (j * v + (1 - j) * (1 - v)) *
                         (k * w + (1 - k) * (1 - w)) *
                         c[i][j][k];
            }

    return accum;
}
*/

void permute(int *p, int n)
{
    for (int i = n - 1; i > 0; i--)
    {
        int target = random_int(0, i);
        int tmp = p[i];
        p[i] = p[target];
        p[target] = tmp;
    }
}

void perlin_generate_perm(int *p)
{
    for (int i = 0; i < POINT_COUNT; i++)
    {
        p[i] = i;
    }

    permute(p, POINT_COUNT);
}

/// @brief Init Perlin Info.
void perlin(struct Perlin_Info *perlin)
{
    for (int i = 0; i < POINT_COUNT; i++)
    {
        vec_rand_in_range(perlin->randvec[i], -1, 1);
        unit(perlin->randvec[i], perlin->randvec[i]);
    }

    perlin_generate_perm(perlin->perm_x);
    perlin_generate_perm(perlin->perm_y);
    perlin_generate_perm(perlin->perm_z);
}

/// @brief Return a random real in [-1, +1] given a point p.
double noise(const struct Perlin_Info *perlin, const point3 p)
{
    double u = p[0] - floor(p[0]);
    double v = p[1] - floor(p[1]);
    double w = p[2] - floor(p[2]);

    int i = (int)floor(p[0]);
    int j = (int)floor(p[1]);
    int k = (int)floor(p[2]);
    // Defining c like this instead of vec3 c[2][2][2] (stores a vec3 inside c),
    // lets us avoid a memcpy below, and saves on overall memory as we only store pointers in c
    // instead of the actual vec3 block.
    const double *c[2][2][2];

    for (int di = 0; di < 2; di++)
        for (int dj = 0; dj < 2; dj++)
            for (int dk = 0; dk < 2; dk++)
            {
                c[di][dj][dk] = perlin->randvec[perlin->perm_x[(i + di) & 255] ^
                                                perlin->perm_y[(j + dj) & 255] ^
                                                perlin->perm_z[(k + dk) & 255]];
            }

    return perlin_interp(c, u, v, w);
}

/// @brief A composite noise that has multiple summed frequencies is used.
/// This is usually called turbulence, and is a sum of repeated calls to noise.
/// @return A random real given a point p.
double turb(const struct Perlin_Info *perlin, const point3 p, int depth)
{
    double accum = 0.0;

    point3 temp_p;
    memcpy(temp_p, p, sizeof(double) * 3);

    double weight = 1.0;

    for (int i = 0; i < depth; i++)
    {
        accum += weight * noise(perlin, temp_p);
        weight *= 0.5;
        // We do temp_p *= 2
        scale(temp_p, temp_p, 2);
    }

    return fabs(accum);
}