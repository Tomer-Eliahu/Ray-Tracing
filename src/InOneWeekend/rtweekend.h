#pragma once

#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

// Constants

const double infinity = INFINITY;
const double pi = 3.1415926535897932385;

// Utility Functions

static inline double degrees_to_radians(double degrees)
{
    return degrees * pi / 180.0;
}

/// @brief Returns a random real in [0,1).
static inline double random_zero_to_one()
{
    return rand() / (RAND_MAX + 1.0);
}

/// @brief Returns a random real in [min,max).
static inline double random_in_range(double min, double max)
{
    return min + (max - min) * random_zero_to_one();
}

// Common Headers

#include "color.h"
#include "ray.h"
#include "vec3.h"
#include "interval.h"
