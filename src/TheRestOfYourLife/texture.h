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
#include "perlin.h"
#include "rtw_stb_image.h"

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

/// @brief An image texture. **Must** be initilized by calling image_tex_init(image_filename)!
struct Image_Tex
{
    struct Image_Info info;
};

void image_tex_init(struct Image_Tex *tex, const char *image_filename)
{
    // Setting the value according to the book.
    tex->info.bytes_per_pixel = 3;
    rtw_image(&tex->info, image_filename);
}

void image_value(color3 ret, const struct Image_Tex *tex, double u, double v, [[maybe_unused]] const point3 p)
{
    const struct Image_Info *image_info = &tex->info;
    // If we have no texture data, then return solid blue as a debugging aid.
    if (image_get_height(image_info) <= 0)
    {
        ret[0] = 0;
        ret[1] = 0;
        ret[2] = 1;
        return;
    }

    // Clamp input texture coordinates to [0,1] x [1,0]
    const struct Interval zero_one = {.min = 0, .max = 1};
    u = interval_clamp(&zero_one, u);

    // Flip V to image coordinates. In (u,v): v increases going up (y up system).
    // But in image coordinates the height increases as you go down from the top left of the image (y down system).
    v = 1.0 - interval_clamp(&zero_one, v);

    int i = (int)(u * image_get_width(image_info));
    int j = (int)(v * image_get_height(image_info));
    const unsigned char *pixel = pixel_data(image_info, i, j);

    double color_scale = 1.0 / 255.0;
    ret[0] = color_scale * pixel[0];
    ret[1] = color_scale * pixel[1];
    ret[2] = color_scale * pixel[2];
}

// A Perlin Noise Texture (Book 2 Section 5). **Must** be initilized by calling noise_tex_init()!
struct Noise_Tex
{
    struct Perlin_Info noise;
    double scale; //< Scale the input point to make the texture vary more quickly
};

/// @brief Init a Perlin noise texture.
/// @param scale By what to scale the input point
/// (the bigger the scale, the more quickly the texture will vary). This is also called the frequency of the noise.
static inline void noise_tex_init(struct Noise_Tex *tex, double scale)
{
    tex->scale = scale;
    perlin(&tex->noise);
}

/// @brief Returns a random grey color.
static inline void noise_value(color3 ret, const struct Noise_Tex *tex,
                               [[maybe_unused]] double u, [[maybe_unused]] double v, const point3 p)
{
    double scale_factor = 1.0 + sin(tex->scale * p[2] + 10.0 * turb(&tex->noise, p, 7));
    scale(ret, (color3){0.5, 0.5, 0.5}, scale_factor);
}

/*OLD noise_value (using the noise function directly):
point3 scaled_point;
scale(scaled_point, (double *)p, tex->scale);


    The output of the Perlin interpolation function (which is the output of the noise function)
    can return negative values.
    These negative values will later be passed to our linear_to_gamma() color function,
    which expects only positive inputs. To mitigate this, we'll map the [−1,+1]
    range of values to [0,1]

scale(ret, (color3){1, 1, 1}, 0.5 * (1.0 + noise(&tex->noise, scaled_point)));
*/

// We need a generic interfacte to all textures

enum Which_Tex
{
    SOLID_COLOR,
    CHECKER,
    IMAGE,
    NOISE
};

union Tex_Object
{
    struct Solid_Color_Tex sct; //< Solid Color Texture.
    struct Checker_Tex checker;
    struct Image_Tex img;
    struct Noise_Tex noise;
};

/// @brief An interface to all textures. Please consult the underlying type (e.g. struct Image_Tex) docs
/// for insturctions on how to initilize that type.
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

    case (enum Which_Tex)IMAGE:
        image_value(ret, &tex->object.img, u, v, p);
        break;

    case (enum Which_Tex)NOISE:
        noise_value(ret, &tex->object.noise, u, v, p);
        break;

    default:
        fprintf(stderr, "Could not identify Texture!\n");
        fflush(stderr);
        exit(EXIT_FAILURE);
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
