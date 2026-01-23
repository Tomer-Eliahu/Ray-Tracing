#pragma once

#include "hittable.h"
#include "hittable_list.h"
#include "sphere.h"
#include "rtweekend.h"
#include "material.h"

struct Camera_Config
{
    double aspect_ratio;   //< Ratio of image width over height
    int image_width;       //< Rendered image width in pixel count
    int samples_per_pixel; //< Count of random samples for each pixel
    int max_depth;         //< Maximum number of ray bounces into scene
    double vfov;           //< Vertical view angle (field of view) in degrees. This is effectively our zoom in/out.
    point3 lookfrom;       //< Point camera is looking from
    point3 lookat;         //< Point camera is looking at

    /// @brief Camera-relative "up" direction ("view up").
    /// We can specify any up vector we want, as long as it's not parallel to the view direction
    /// (the diffrence between the look at and from points).
    vec3 vup;

    /// @brief Variation angle of rays through each pixel.
    /// @remarks By specifying the angle we preserve the same amount of blur as
    /// we change the focus distance and, corresponding to this angle, resize the defocus disk (the lens).
    /// Also note that defocus blur is sometimes called depth of field.
    /// See section 13 (Defocus Blur) for more details.
    double defocus_angle;
    double focus_dist; //< Distance from camera lookfrom point to plane of perfect focus
};

/// @brief Store derived camera information.
/// This is not meant to be accessed or modified from outside this file.
struct Camera_Info
{
    int image_height;           //< Rendered image height
    double pixel_samples_scale; // Color scale factor for a sum of pixel samples
    point3 center;              //< Camera center
    point3 pixel00_loc;         //< Location of pixel 0, 0
    vec3 pixel_delta_u;         //< Offset to pixel to the right
    vec3 pixel_delta_v;         //< Offset to pixel below
    vec3 u, v, w;               //< Camera frame basis vectors
    vec3 defocus_disk_u;        //< Defocus disk horizontal radius
    vec3 defocus_disk_v;        //< Defocus disk vertical radius
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
                closest_so_far = temp_rec.t;
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
void ray_color(color3 color, const struct Ray *ray, int depth, const struct Hittable *world, int world_length)
{
    // If we've exceeded the ray bounce limit, no more light is gathered.
    if (depth <= 0)
    {
        color[0] = 0;
        color[1] = 0;
        color[2] = 0;
        return;
    }

    struct Hit_Record rec;

    // Note that we are careful to set the min to 0.001 to get rid of shadow acne (see section 9.3).
    if (world_hit(world, world_length, ray, (struct Interval){.min = 0.001, .max = infinity}, &rec))
    {

        struct Ray scattered;
        color3 attenuation;

        switch (rec.mat_cfg->mat)
        {
        case (enum Material)Lambertian:
            if (lambertian_scatter(ray, &rec, attenuation, &scattered))
            {
                ray_color(color, &scattered, depth - 1, world, world_length);
                multiply(color, attenuation, color);
                return;
            }

            // set to black
            color[0] = 0;
            color[1] = 0;
            color[2] = 0;
            return;

            break;

        case (enum Material)Metal:

            if (metal_scatter(ray, &rec, attenuation, &scattered))
            {
                ray_color(color, &scattered, depth - 1, world, world_length);
                multiply(color, attenuation, color);
                return;
            }

            // set to black
            color[0] = 0;
            color[1] = 0;
            color[2] = 0;
            return;

            break;

        case (enum Material)Dielectric:

            if (dielectric_scatter(ray, &rec, attenuation, &scattered))
            {
                ray_color(color, &scattered, depth - 1, world, world_length);
                multiply(color, attenuation, color);
                return;
            }

            // set to black
            color[0] = 0;
            color[1] = 0;
            color[2] = 0;
            return;

            break;

        default:
            fprintf(stderr, "Could not identify Material of object hit!\n");
            fflush(stderr);
            break;
        }
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

    cam_info->pixel_samples_scale = 1.0 / cfg->samples_per_pixel;

    // Set the camera center;
    memcpy(cam_info->center, cfg->lookfrom, 3 * sizeof(double));

    // Determine viewport dimensions.
    double theta = degrees_to_radians(cfg->vfov);
    double h = tan(theta / 2); // See section 12.1 for details.
    double viewport_height = 2 * h * cfg->focus_dist;

    // image_width/image_height is the *actual* aspect ratio we will have
    double viewport_width = viewport_height * ((double)cfg->image_width / cam_info->image_height);

    // Calculate the u,v,w unit (orthonormal) basis vectors for the camera coordinate frame.
    // See section 12.2 for details.
    vec3 temp;
    unit(cam_info->w, subtract(temp, (double *)cfg->lookfrom, (double *)cfg->lookat));
    unit(cam_info->u, cross(temp, (double *)cfg->vup, cam_info->w));
    // As w and u are perpendicular and are both unit vectors, their cross product will also be a unit vector.
    cross(cam_info->v, cam_info->w, cam_info->u);

    // Calculate the vectors across the horizontal and down the vertical viewport edges.
    vec3 viewport_u, viewport_v;
    scale(viewport_u, cam_info->u, viewport_width); // Vector across viewport horizontal edge
    // Vector *down* viewport vertical edge
    scale(viewport_v, negate(viewport_v, cam_info->v), viewport_height);

    // Calculate the horizontal and vertical delta vectors from pixel to pixel.
    // This is just viewport_u/image_width and viewport_v/image_height
    scale(cam_info->pixel_delta_u, viewport_u, (1.0 / (double)cfg->image_width));
    scale(cam_info->pixel_delta_v, viewport_v, (1.0 / (double)cam_info->image_height));

    // Calculate the location of the upper left pixel.
    // viewport_upper_left = center - (focus_dist * w) - viewport_u/2 - viewport_v/2;
    point3 temp1, temp2;
    point3 viewport_upper_left;
    subtract(temp1, cam_info->center, scale(temp2, cam_info->w, cfg->focus_dist));
    subtract(temp1, temp1, scale(temp2, viewport_u, (0.5)));
    subtract(viewport_upper_left, temp1, scale(temp2, viewport_v, (0.5)));

    // pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v)
    add(cam_info->pixel00_loc, viewport_upper_left,
        scale(temp1,
              add(temp1, cam_info->pixel_delta_u, cam_info->pixel_delta_v), 0.5));

    // Calculate the camera defocus disk basis vectors.
    double defocus_radius = cfg->focus_dist * tan(degrees_to_radians(cfg->defocus_angle / 2.0));
    scale(cam_info->defocus_disk_u, cam_info->u, defocus_radius);
    scale(cam_info->defocus_disk_v, cam_info->v, defocus_radius);
}

/// @brief Sets the vector to a random point in the [-.5,-.5]-[+.5,+.5] unit square.
void sample_square(vec3 vec)
{
    vec[0] = random_zero_to_one() - 0.5;
    vec[1] = random_zero_to_one() - 0.5;
    vec[3] = 0;
}

/// @brief Sets point to a random point in the camera defocus disk.
void defocus_disk_sample(point3 point, const struct Camera_Info *cam_info)
{
    vec3 p;
    random_in_unit_disk(p);
    // point = cam_info->center + (p[0] * cam_info->defocus_disk_u) + (p[1] * cam_info->defocus_disk_v)
    vec3 temp1, temp2;
    scale(temp1, (double *)cam_info->defocus_disk_u, p[0]);
    scale(temp2, (double *)cam_info->defocus_disk_v, p[1]);
    add(temp1, temp1, temp2);
    add(point, (double *)cam_info->center, temp1);
}

/// @brief Construct a camera ray originating from the defocus disk and directed at a randomly
/// sampled point around the pixel location i, j.
void get_ray(struct Ray *ray, const struct Camera_Info *cam_info, int i, int j, double defocus_angle)
{

    // calculate the pixel sample location
    point3 offset;
    sample_square(offset);

    point3 temp1, temp2;
    point3 pixel_sample;
    scale(temp1, (double *)cam_info->pixel_delta_u, (i + offset[0]));
    scale(temp2, (double *)cam_info->pixel_delta_v, (j + offset[1]));
    add(pixel_sample, (double *)cam_info->pixel00_loc, add(temp1, temp1, temp2));

    if (defocus_angle <= 0)
    {
        // The ray origin is the camera center (no defocus blur)
        memcpy(ray->origin, cam_info->center, 3 * sizeof(double));
    }
    else
    {
        defocus_disk_sample(ray->origin, cam_info);
    }

    subtract(ray->direction, pixel_sample, ray->origin);
}

/// @brief Render the image
/// @param world a list of Hittable objects
void camera_render(const struct Hittable *world, const int world_length, const struct Camera_Config *cfg)
{

    struct Camera_Info cam_info;
    camera_initialize(cfg, &cam_info);
    // Render

    printf("P3\n");                                             // This means the colors will be in ASCII
    printf("%i %i\n", cfg->image_width, cam_info.image_height); // how many pixels to make
    printf("255\n");                                            // Max color possible

    // The book does this (j then i). So we follow that.
    for (int j = 0; j < cam_info.image_height; j++)
    {
        fprintf(stderr, "\rLines scanned: %i out of %i", j + 1, cam_info.image_height);
        fflush(stderr);

        for (int i = 0; i < cfg->image_width; i++)
        {
            color3 pixel_color = {0};
            struct Ray r;

            /*
                For a single pixel composed of multiple samples,
                we'll select samples from the area surrounding the pixel
                and average the resulting light (color) values together. We do this to accomplish antialiasing
                (smoothing out edges). Remember a pixel is really a point sample.
            */
            for (int sample = 0; sample < cfg->samples_per_pixel; sample++)
            {
                get_ray(&r, &cam_info, i, j, cfg->defocus_angle);

                color3 temp;
                ray_color(temp, &r, cfg->max_depth, world, world_length);
                add(pixel_color, pixel_color, temp);
            }

            /*
                By convention, each of the red/green/blue components are represented internally
                by real-valued variables that range from 0.0 to 1.0.
                These must be scaled to integer values between 0 and 255 before we print them out
                (this happens in the write_color function).
            */
            scale(pixel_color, pixel_color, cam_info.pixel_samples_scale);
            write_color(pixel_color);
        }
    }

    fprintf(stderr, "\nRender done!");
}