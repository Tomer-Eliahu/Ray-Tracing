#include <stdio.h>

#include "color.h"
#include "vec3.h"
#include "ray.h"

///@brief Find out if the given ray hits this sphere
///@param center the center of the sphere
int hit_sphere(const point3 center, double radius, const struct Ray *ray)
{

    // As the book explains, this boils down to finding out how many solutions a quadratic equation has.
    vec3 diff;
    subtract(diff, (double *)center, (double *)ray->origin);

    double a = dot(ray->direction, ray->direction);
    double b = -2.0 * dot(ray->direction, diff);
    double c = dot(diff, diff) - radius * radius;
    double discriminant = b * b - 4 * a * c;

    return (discriminant >= 0);
}

///@brief sets the color for a given scene ray
void ray_color(color3 color, const struct Ray *ray)
{
    if (hit_sphere((point3){0, 0, -1}, 0.5, ray))
    {
        color[0] = 1; // R
        color[1] = 0; // G
        color[2] = 0; // B
        return;
    }

    vec3 unit_dir;

    // We know that this use won't actually modify ray->direction
    unit(unit_dir, (double *)ray->direction);

    double a = 0.5 * (unit_dir[1] + 1.0);
    // white is (1.0, 1.0, 1.0) and blue is (0.5, 0.7, 1.0);
    // We want a linear interpolation where the bottom is white and the top is blue.
    color[0] = (1.0 - a) * 1 + a * 0.5;
    color[1] = (1.0 - a) * 1 + a * 0.7;
    color[2] = (1.0 - a) * 1 + a * 1;
}

int main()
{

    /*
        We will render images (run build\inOneWeekend.exe > image.ppm).
        We use the ppm format of writing some numbers to a file to describe the image.

        You can use https://jumpshare.com/viewer/ppm to view the image (no download needed)
        or this extension (PBM/PPM/PGM Viewer for Visual Studio Code -- what I am using).
    */

    // Image

    double aspect_ratio = 16.0 / 9.0; // width/height
    int image_width = 400;

    // Calculate the image height, and ensure that it's at least 1.

    int image_height = (int)image_width / aspect_ratio;
    image_height = (image_height < 1) ? 1 : image_height;

    // Camera

    double focal_length = 1.0;
    double viewport_height = 2.0;

    // image_width/image_height is the *actual* aspect ratio we will have
    double viewport_width = viewport_height * ((double)image_width / image_height);
    point3 camera_center = {0};

    // Calculate the vectors across the horizontal and down the vertical viewport edges.
    vec3 viewport_u = {viewport_width, 0, 0};
    vec3 viewport_v = {0, -viewport_height, 0};

    // Calculate the horizontal and vertical delta vectors from pixel to pixel.
    // This is just viewport_u/image_width and viewport_v/image_height
    vec3 pixel_delta_u = {(double)viewport_width / image_width, 0, 0};
    vec3 pixel_delta_v = {0, (double)-viewport_height / image_height, 0};

    // Calculate the location of the upper left pixel.
    point3 temp1, temp2;
    point3 viewport_upper_left, pixel00_loc;
    subtract(temp1, camera_center, (vec3){0, 0, focal_length});
    subtract(temp1, temp1, scale(temp2, viewport_u, (0.5)));
    subtract(viewport_upper_left, temp1, scale(temp2, viewport_v, (0.5)));

    // pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v)
    add(pixel00_loc, viewport_upper_left,
        scale(temp1, add(temp1, pixel_delta_u, pixel_delta_v), 0.5));

    // Render

    printf("P3\n");                               // This means the colors will be in ASCII
    printf("%i %i\n", image_width, image_height); // how many pixels to make
    printf("255\n");                              // Max color possible

    // The book does this (j then i). So we follow that.
    for (int j = 0; j < image_height; j++)
    {
        fprintf(stderr, "\rLines scanned: %i out of %i", j + 1, image_height);
        fflush(stderr);

        for (int i = 0; i < image_width; i++)
        {
            point3 pixel_center, temp1, temp2;

            // calculate the pixel location
            scale(temp1, pixel_delta_u, i);
            scale(temp2, pixel_delta_v, j);
            add(pixel_center, pixel00_loc, add(temp1, temp1, temp2));

            vec3 ray_dir;
            subtract(ray_dir, pixel_center, camera_center);

            struct Ray r;
            memcpy(r.origin, camera_center, 3 * sizeof(double));
            memcpy(r.direction, ray_dir, 3 * sizeof(double));

            color3 pixel_color;
            ray_color(pixel_color, &r);

            /*
                By convention, each of the red/green/blue components are represented internally
                by real-valued variables that range from 0.0 to 1.0.
                These must be scaled to integer values between 0 and 255 before we print them out
                (this happens in the write_color function).
            */

            write_color(pixel_color);
        }
    }

    fprintf(stderr, "\nRender done!");

    return 0;
}