#pragma once

#include "vec3.h"
#include <stdio.h>

/// @brief Write out a color to the output stream.
/// @param color
void write_color(color3 color)
{
    double r = color[0];
    double g = color[1];
    double b = color[2];

    // Translate the component values in the range [0,1] to the byte range [0,255].
    int rbyte = (int)255.999 * r;
    int gbyte = (int)255.999 * g;
    int bbyte = (int)255.999 * b;

    // Write out the color components
    printf("%i %i %i\n", rbyte, gbyte, bbyte);
}
