#pragma once

#include "hittable.h"
#include "hittable_list.h"
#include "sphere.h"
#include "rtweekend.h"

struct Camera_Config
{
    double aspect_ratio; //< Ratio of image width over height
    int image_width;     //< Rendered image width in pixel count
};

/// @brief Store derived camera information.
/// This is not meant to be accessed or modified from outside this file.
struct Camera_Info
{
    int image_height;   //< Rendered image height
    point3 center;      //< Camera center
    point3 pixel00_loc; //< Location of pixel 0, 0
    vec3 pixel_delta_u; //< Offset to pixel to the right
    vec3 pixel_delta_v; //< Offset to pixel below
};

/// @brief returns if any objects in the world are hit by the ray
/// @param world an array of hittable objects
/// @param ray
/// @param ray_interval
/// @param rec the Hit Record-- updated accordingly
/// @return
bool world_hit(const struct Hittable *world, int world_length, const struct Ray *ray,
               struct Interval ray_interval, struct Hit_Record *rec)
{

    struct Hit_Record temp_rec;
    bool hit_anything = false;
    double closest_so_far = ray_interval.max;

    for (int i = 0; i < world_length; i++)
    {
        // find out which Hittable object this is
        switch (world[i].which)
        {
        case (enum Which_Hittable)Sphere:
            if (sphere_hit(&world[i].object.sphere, ray,
                           (struct Interval){.min = ray_interval.min, .max = closest_so_far}, &temp_rec))
            {
                hit_anything = true;
                closest_so_far = rec->t;
                // set rec to temp_rec
                *rec = temp_rec;
            }
            break;

        default:
            fprintf(stderr, "Could not identify Hittable!\n");
            fflush(stderr);
            break;
        }
    }

    return hit_anything;
}

///@brief sets the color for a given scene ray
void ray_color(color3 color, const struct Ray *ray, const struct Hittable *world, int world_length)
{
    struct Hit_Record rec;

    if (world_hit(world, world_length, ray, (struct Interval){.min = 0, .max = infinity}, &rec))
    {
        scale(color, add(color, rec.normal, (color3){1, 1, 1}), 0.5);
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

/// @brief Derive Camera_Info from the camera config.
/// @param cfg
/// @param cam_info
void camera_initialize(const struct Camera_Config *cfg, struct Camera_Info *cam_info)
{

    // Calculate the image height, and ensure that it's at least 1.

    cam_info->image_height = (int)cfg->image_width / cfg->aspect_ratio;
    cam_info->image_height = (cam_info->image_height < 1) ? 1 : cam_info->image_height;

    // Set the camera center to 0,0,0;
    memset(cam_info->center, 0, 3 * sizeof(double));

    // Determine viewport dimensions.

    double focal_length = 1.0;
    double viewport_height = 2.0;

    // image_width/image_height is the *actual* aspect ratio we will have
    double viewport_width = viewport_height * ((double)cfg->image_width / cam_info->image_height);

    // Calculate the vectors across the horizontal and down the vertical viewport edges.
    vec3 viewport_u = {viewport_width, 0, 0};
    vec3 viewport_v = {0, -viewport_height, 0};

    // Calculate the horizontal and vertical delta vectors from pixel to pixel.
    // This is just viewport_u/image_width and viewport_v/image_height
    cam_info->pixel_delta_u[0] = (double)viewport_width / cfg->image_width;
    cam_info->pixel_delta_u[1] = 0;
    cam_info->pixel_delta_u[2] = 0;

    cam_info->pixel_delta_v[0] = 0;
    cam_info->pixel_delta_v[1] = (double)-viewport_height / cam_info->image_height;
    cam_info->pixel_delta_v[2] = 0;

    // Calculate the location of the upper left pixel.
    point3 temp1, temp2;
    point3 viewport_upper_left;
    subtract(temp1, cam_info->center, (vec3){0, 0, focal_length});
    subtract(temp1, temp1, scale(temp2, viewport_u, (0.5)));
    subtract(viewport_upper_left, temp1, scale(temp2, viewport_v, (0.5)));

    // pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v)
    add(cam_info->pixel00_loc, viewport_upper_left,
        scale(temp1,
              add(temp1, cam_info->pixel_delta_u, cam_info->pixel_delta_v), 0.5));
}

/// @brief Render the image
/// @param world a list of Hittable objects
void camera_render(const struct Hittable *world, const int world_length, const struct Camera_Config *cfg)
{

    struct Camera_Info cam_info;
    camera_initialize(cfg, &cam_info);
    // Render

    printf("P3\n");                                                   // This means the colors will be in ASCII
    printf("%i %i\n", cfg->image_width, cam_info.image_height); // how many pixels to make
    printf("255\n");                                                  // Max color possible

    // The book does this (j then i). So we follow that.
    for (int j = 0; j < cam_info.image_height; j++)
    {
        fprintf(stderr, "\rLines scanned: %i out of %i", j + 1, cam_info.image_height);
        fflush(stderr);

        for (int i = 0; i < cfg->image_width; i++)
        {
            point3 pixel_center, temp1, temp2;

            // calculate the pixel location
            scale(temp1, cam_info.pixel_delta_u, i);
            scale(temp2, cam_info.pixel_delta_v, j);
            add(pixel_center, cam_info.pixel00_loc, add(temp1, temp1, temp2));

            vec3 ray_dir;
            subtract(ray_dir, pixel_center, cam_info.center);

            struct Ray r;
            memcpy(r.origin, cam_info.center, 3 * sizeof(double));
            memcpy(r.direction, ray_dir, 3 * sizeof(double));

            color3 pixel_color;
            ray_color(pixel_color, &r, world, world_length);

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
}