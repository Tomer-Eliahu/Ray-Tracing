#pragma once

#include "vec3.h"
#include <stdio.h>
#include "interval.h"

static const struct Interval intensity = {.min = 0.000, .max = 0.999};

/*

The reason for this is that almost all computer programs assume that an image is “gamma corrected”
before being written into an image file.
This means that the 0 to 1 values have some transform applied before being stored as a byte.
Images with data that are written without being transformed are said to be in linear space,
whereas images that are transformed are said to be in gamma space.
It is likely that the image viewer you are using is expecting an image in gamma space,
but we are giving it an image in linear space.
This will make our images appear inaccurately dark.

There are many good reasons for why images should be stored in gamma space,
but for our purposes we just need to be aware of it.
We are going to transform our data into gamma space
so that our image viewer can more accurately display our image.

*/

static inline double linear_to_gamma(double linear_component)
{
    return (linear_component > 0) ? sqrt(linear_component) : 0;
}

/// @brief Write out a color to the output stream.
/// @param color
void write_color(color3 color)
{
    double r = color[0];
    double g = color[1];
    double b = color[2];

    // Apply a linear to gamma transform for gamma 2
    r = linear_to_gamma(r);
    g = linear_to_gamma(g);
    b = linear_to_gamma(b);

    // Translate the component values in the range [0,1] to the byte range [0,255].
    int rbyte = (int)255.999 * interval_clamp(&intensity, r);
    int gbyte = (int)255.999 * interval_clamp(&intensity, g);
    int bbyte = (int)255.999 * interval_clamp(&intensity, b);

    // Write out the color components
    printf("%i %i %i\n", rbyte, gbyte, bbyte);
}
