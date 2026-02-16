#pragma once

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include "external/stb_image.h"
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

/// @brief Store image information.
struct Image_Info
{
    int bytes_per_pixel;  //< Must be set to 3 before use.
    float *fdata;         //< Linear floating point pixel data
    unsigned char *bdata; //< Linear 8-bit pixel data
    int image_width;      //< Loaded image width
    int image_height;     //< Loaded image height
    int bytes_per_scanline;
};

/// @brief Return the value clamped to the range [low, high).
int image_clamp(int x, int low, int high)
{
    if (x < low)
        return low;
    if (x < high)
        return x;
    return high - 1;
}

unsigned char float_to_byte(float value)
{
    if (value <= 0.0)
        return 0;
    if (1.0 <= value)
        return 255;
    return (unsigned char)(256.0 * value);
}

/// @brief Convert the linear floating point pixel data to bytes,
/// storing the resulting byte data in the `bdata` member.
void convert_to_bytes(struct Image_Info *image_info)
{
    int total_bytes = image_info->image_width * image_info->image_height * image_info->bytes_per_pixel;

    free(image_info->bdata); // free old data.
    image_info->bdata = malloc(sizeof(unsigned char) * total_bytes);
    if (image_info->bdata == nullptr)
    {
        fprintf(stderr, "Malloc failed in convert_to_bytes!\n");
        fflush(stderr);
        exit(EXIT_FAILURE);
    }

    // Iterate through all pixel components, converting from [0.0, 1.0] float values to
    // unsigned [0, 255] byte values.
    unsigned char *bptr = image_info->bdata;
    float *fptr = image_info->fdata;
    for (int i = 0; i < total_bytes; i++, fptr++, bptr++)
        *bptr = float_to_byte(*fptr);
}

/// @brief Loads the linear (gamma=1) image data from the given file name.
/// Returns true if the load succeeded. The resulting data buffer contains the *three* [0.0, 1.0]
/// floating-point values for the first pixel (red, then green, then blue). Pixels are
/// contiguous, going left to right for the width of the image, followed by the next row
/// below, for the full height of the image.
bool load(struct Image_Info *image_info, const char *image_filename)
{

    int n = image_info->bytes_per_pixel; // Dummy out parameter (irrelvant for us): original components per pixel
    STBI_FREE(image_info->fdata);        // free old data (if any)

    image_info->fdata = stbi_loadf(image_filename, &image_info->image_width,
                                   &image_info->image_height, &n, image_info->bytes_per_pixel);

    if (image_info->fdata == nullptr)
    {
        return false;
    }

    image_info->bytes_per_scanline = image_info->image_width * image_info->bytes_per_pixel;
    convert_to_bytes(image_info);
    return true;
}

/// @brief Loads image data from the specified file and updates the Image_Info accordingly.
/// If the image was not loaded successfully,
/// image_get_width() and image_get_height() will return 0.
void rtw_image(struct Image_Info *image_info, const char *image_filename)
{

    const char *prefix = "src/external/images/";

    int total_len = strlen(prefix) + strlen(image_filename) + 1; // + 1 for null terminator
    char full_filename[total_len];
    // Make the full_filename
    strcpy(full_filename, prefix);
    strcat(full_filename, image_filename);

    if (load(image_info, full_filename))
        return;

    printf("ERROR: Could not load image file %s . \n", image_filename);
}

static inline int image_get_width(const struct Image_Info *image_info)
{
    return (image_info->fdata == nullptr) ? 0 : image_info->image_width;
}
static inline int image_get_height(const struct Image_Info *image_info)
{
    return (image_info->fdata == nullptr) ? 0 : image_info->image_height;
}

/// @brief Return the address of the three RGB bytes of the pixel at x,y.
/// Remember x is the width position, and y is the height position.
/// If there is no image data, returns magenta.
const unsigned char *pixel_data(const struct Image_Info *image_info, int x, int y)
{
    static unsigned char magenta[] = {255, 0, 255};
    if (image_info->bdata == nullptr)
    {
        return magenta;
    }

    x = image_clamp(x, 0, image_info->image_width);
    y = image_clamp(y, 0, image_info->image_height);

    return image_info->bdata + y * image_info->bytes_per_scanline + x * image_info->bytes_per_pixel;
}