#pragma once

#include "vec3.h"
#include <stdio.h>
#include "interval.h"

static const struct Interval intensity = {.min = 0.000, .max = 0.999};

/// @brief Write out a color to the output stream.
/// @param color
void write_color(color3 color)
{
    double r = color[0];
    double g = color[1];
    double b = color[2];

    // Translate the component values in the range [0,1] to the byte range [0,255].
    int rbyte = (int)255.999 * interval_clamp(&intensity, r);
    int gbyte = (int)255.999 * interval_clamp(&intensity, g);
    int bbyte = (int)255.999 * interval_clamp(&intensity, b);

    // Write out the color components
    printf("%i %i %i\n", rbyte, gbyte, bbyte);
}
