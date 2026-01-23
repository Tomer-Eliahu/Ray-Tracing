#include "rtweekend.h"

#include "camera.h"
#include "hittable.h"
#include "hittable_list.h"
#include "sphere.h"
#include "material.h"

#define WORLD_LENGTH 5 // How many hittable objects there are in the world.

int main()
{

    /*
        We will render images (run build\inOneWeekend.exe > image.ppm).
        We use the ppm format of writing some numbers to a file to describe the image.

        You can use https://jumpshare.com/viewer/ppm to view the image (no download needed)
        or this extension (PBM/PPM/PGM Viewer for Visual Studio Code -- what I am using).
    */

    // World

    // Materials
    const struct Material_Cfg material_ground = {.mat = Lambertian, .albedo = {0.8, 0.8, 0.0}};
    const struct Material_Cfg material_center = {.mat = Lambertian, .albedo = {0.1, 0.2, 0.5}};
    const struct Material_Cfg material_left = {.mat = Dielectric, .refraction_index = 1.50};
    // Air bubble in Glass
    const struct Material_Cfg material_bubble = {.mat = Dielectric, .refraction_index = (1.00 / 1.50)};
    const struct Material_Cfg material_right = {.mat = Metal, .albedo = {0.8, 0.6, 0.2}, .fuzz = 1.0};

    struct Hittable world[WORLD_LENGTH] = {
        {.which = (enum Which_Hittable)Sphere,
         .object.sphere = {.center = {0.0, -100.5, -1.0}, .radius = 100, .mat_cfg = &material_ground}},
        {.which = (enum Which_Hittable)Sphere,
         .object.sphere = {.center = {0.0, 0.0, -1.2}, .radius = 0.5, .mat_cfg = &material_center}},
        {.which = (enum Which_Hittable)Sphere,
         .object.sphere = {.center = {-1.0, 0.0, -1.0}, .radius = 0.5, .mat_cfg = &material_left}},
        {.which = (enum Which_Hittable)Sphere,
         .object.sphere = {.center = {-1.0, 0.0, -1.0}, .radius = 0.4, .mat_cfg = &material_bubble}},
        {.which = (enum Which_Hittable)Sphere,
         .object.sphere = {.center = {1.0, 0.0, -1.0}, .radius = 0.5, .mat_cfg = &material_right}},
    };

    struct Camera_Config cam =
        {
            .aspect_ratio = 16.0 / 9.0,
            .image_width = 400,
            .samples_per_pixel = 100,
            .max_depth = 50,
            .vfov = 20,
            .lookfrom = {-2, 2, 1},
            .lookat = {0, 0, -1},
            .vup = {0, 1, 0},
            .defocus_angle = 10.0,
            .focus_dist = 3.4,
        };

    camera_render(world, WORLD_LENGTH, &cam);

    return 0;
}