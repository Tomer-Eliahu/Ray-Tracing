#pragma once

/*Book 2 Section 4: Texture mapping

Texture mapping in computer graphics is the process of applying a material effect to an object in the scene.
The “texture” part is the effect, and the “mapping” part is in the mathematical sense
of mapping one space onto another. This effect could be any material property:
color, shininess, bump geometry (called Bump Mapping),
or even material existence (to create cut-out regions of the surface).

The most common type of texture mapping maps an image onto the surface of an object,
defining the color at each point on the object’s surface.
In practice, we implement the process in reverse:
given some point on the object, we’ll look up the color defined by the texture map.

In order to perform the texture lookup, we need a texture coordinate.
This coordinate can be defined in many ways, and we'll develop this idea as we progress.
For now, we'll pass in two dimensional texture coordinates.
By convention, texture coordinates are named u and v.
For a constant texture, every (u,v) pair yields a constant color,
so we can actually ignore the coordinates completely.
However, other texture types will need these coordinates, so we keep these in the method interface.

The primary method of our texture classes is the color value(...) method,
which returns the texture color given the input coordinates.
In addition to taking the point's texture coordinates u and v,
we also provide the position of the point in question, for reasons that will become apparent later.

*/

#include "rtweekend.h"

// Forward declaration.
struct Texture;
void tex_value(color3 ret, const struct Texture *tex, double u, double v, const point3 p);

/// @brief A solid color texture. Will be abbreviated sct.
struct Solid_Color_Tex
{
    /// @brief Albedo is the measure of a surface's reflectivity,
    /// representing the fraction of sunlight (or other radiation) that is reflected,
    /// ranging from 0 (no reflection, black) to 1 (total reflection, white).
    /// Note that this is done across RGB (color3) as opposed to the x-y-z axes.
    color3 albedo;
};

/// @brief The color that will be returned from value(double u, double v, point3 p) is always just the albedo value.
static inline void sct_value(color3 ret, const struct Solid_Color_Tex *tex,
                             [[maybe_unused]] double u, [[maybe_unused]] double v, [[maybe_unused]] const point3 p)
{
    memcpy(ret, tex->albedo, 3 * sizeof(double));
}

/*Section 4.2. Solid Textures: A Checker Texture

A solid (or spatial) texture depends only on the position of each point in 3D space.
You can think of a solid texture as if it's coloring all of the points in space itself,
instead of coloring a given object in that space.
For this reason, the object can move through the colors of the texture as it changes position,
though usually you would to fix the relationship between the object and the solid texture.

To explore spatial textures, we'll implement a spatial checker_texture class,
which implements a three-dimensional checker pattern.
Since a spatial texture function is driven by a given position in space,
the texture value() function ignores the u and v parameters, and uses only the p parameter.

To accomplish the checkered pattern, we'll first compute the floor of each component of the input point.
We could truncate the coordinates, but that would pull values toward zero,
which would give us the same color on both sides of zero.
The floor function will always shift values to the integer value on the left (toward negative infinity).
Given these three integer results (⌊x⌋,⌊y⌋,⌊z⌋
) we take their sum and compute the result modulo two, which gives us either 0 or 1.
Zero maps to the even color, and one to the odd color.

Finally, we add a scaling factor to the texture,
to allow us to control the size of the checker pattern in the scene.

*/

/// @brief A checker texture. Please use the make_checker_from_tex or make_checker_from_colors
/// functions to initilize this struct.
struct Checker_Tex
{
    /// @brief 1/scale where scale is a scaling factor of the texture.
    /// this allows us to control the size of the checker pattern in the scene.
    /// The bigger scale is, the bigger the squares of the checker are.
    double inv_scale;
    struct Texture *even;
    struct Texture *odd;
};

/// @brief A solid (or spatial) texture like checker depends only on the position of the point in 3D space.
void checker_value(color3 ret, const struct Checker_Tex *tex, double u, double v, const point3 p)
{
    int xInteger = (int)floor(tex->inv_scale * p[0]);
    int yInteger = (int)floor(tex->inv_scale * p[1]);
    int zInteger = (int)floor(tex->inv_scale * p[2]);

    bool isEven = (xInteger + yInteger + zInteger) % 2 == 0;

    isEven ? tex_value(ret, tex->even, u, v, p) : tex_value(ret, tex->odd, u, v, p);
}

// We need a generic interfacte to all textures

enum Which_Tex
{
    SOLID_COLOR,
    CHECKER
};

union Tex_Object
{
    struct Solid_Color_Tex sct; //< Solid Color Texture.
    struct Checker_Tex checker;
};

/// @brief An interface to all textures
struct Texture
{
    enum Which_Tex which;
    union Tex_Object object;
};

/// @brief Returns the texture color for the given texture given the input coordinates.
/// In addition to taking the point's texture coordinates u and v,
/// we also provide the position of the point in question, for reasons that will become apparent later.
/// @param ret The color value to return.
void tex_value(color3 ret, const struct Texture *tex, double u, double v, const point3 p)
{
    switch (tex->which)
    {
    case (enum Which_Tex)SOLID_COLOR:
        sct_value(ret, &tex->object.sct, u, v, p);
        break;

    case (enum Which_Tex)CHECKER:
        checker_value(ret, &tex->object.checker, u, v, p);
        break;

    default:
        fprintf(stderr, "Could not identify Texture!\n");
        fflush(stderr);
        break;
    }
}

void static inline make_checker_from_tex(struct Checker_Tex *ret, double scale,
                                         struct Texture *even, struct Texture *odd)
{
    ret->inv_scale = 1.0 / scale;
    ret->even = even;
    ret->odd = odd;
}

/// @brief Shortcut to make a checker texture from 2 colors. **Creates 2 heap allocated Solid_Color_Tex**.
/// @param ret
/// @param scale
/// @param color1
/// @param color2
void make_checker_from_colors(struct Checker_Tex *ret, double scale,
                              const color3 color1, const color3 color2)
{
    ret->inv_scale = 1.0 / scale;

    ret->even = malloc(sizeof(struct Texture));
    ret->odd = malloc(sizeof(struct Texture));

    if ((ret->even == NULL) || (ret->odd == NULL))
    {
        fprintf(stderr, "Could not malloc Textures in make_checker_from_colors!\n");
        fflush(stderr);
        exit(EXIT_FAILURE);
    }

    ret->even->which = SOLID_COLOR;
    memcpy(ret->even->object.sct.albedo, color1, 3 * sizeof(double));

    ret->odd->which = SOLID_COLOR;
    memcpy(ret->odd->object.sct.albedo, color2, 3 * sizeof(double));
}
