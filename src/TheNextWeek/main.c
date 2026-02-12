#include "rtweekend.h"

#include "camera.h"
#include "hittable.h"
#include "hittable_list.h"
#include "sphere.h"
#include "material.h"
#include "bvh.h"
#include "texture.h"

/// How many hittable objects there could possibly be in the world.
/// If we write past the end of an array with this size, the OS throws an exception for us.
#define MAX_WORLD_LENGTH 500

/// Enable truly random results that vary from run to run.
#define WANT_TRUE_RANDOM

#ifdef WANT_TRUE_RANDOM
#include <time.h>
#endif

int main()
{

#ifdef WANT_TRUE_RANDOM
    // Seed the random number generator (which rand() uses) with the current time.
    srand((unsigned int)time(NULL));
#endif

    /*
        We will render images (run build\theNextWeek.exe > image.ppm).
        We use the ppm format of writing some numbers to a file to describe the image.

        You can use https://jumpshare.com/viewer/ppm to view the image (no download needed)
        or this extension (PBM/PPM/PGM Viewer for Visual Studio Code -- what I am using).
    */

    // World

    // Textures

    struct Texture ground_texture = {.which = CHECKER};
    make_checker_from_colors(&ground_texture.object.checker, 0.32,
                             (color3){0.2, 0.3, 0.1}, (color3){0.9, 0.9, 0.9});

    // Materials

    const struct Material_Cfg ground_material = {.which = Lambertian,
                                                 .object.lambertian.tex = &ground_texture};

    const struct Material_Cfg glass_material = {.which = Dielectric, .object.dielectric.refraction_index = 1.5};

    struct Hittable world[MAX_WORLD_LENGTH];
    world[0] =
        (struct Hittable){.which = (enum Which_Hittable)Sphere,
                          .object.sphere =
                              {.center =
                                   (struct Ray){.origin = {0.0, -1000.0, 0.0}, .direction = {0}},
                               .radius = 1000.0,
                               .mat_cfg = &ground_material}};

    int actual_world_len = 1; // How many Hittable objects we actually have in the scene
    struct Material_Cfg materials[MAX_WORLD_LENGTH];
    int additonal_materials = 0;

    for (int a = -11; a < 11; a++)
    {
        for (int b = -11; b < 11; b++)
        {
            point3 center = {a + 0.9 * random_zero_to_one(), 0.2, b + 0.9 * random_zero_to_one()};
            vec3 temp;
            if (len(subtract(temp, center, (point3){4, 0.2, 0})) > 0.9)
            {
                double choose_mat = random_zero_to_one();

                if (choose_mat < 0.8)
                {
                    // diffuse (Lambertian)
                    vec3 temp1, temp2;
                    vec_rand_zero_to_one(temp1);
                    vec_rand_zero_to_one(temp2);

                    struct Material_Cfg new_mat = {.which = Lambertian, 
                        .object.lambertian.tex = malloc(sizeof(struct Texture))};
                    new_mat.object.lambertian.tex->which = SOLID_COLOR;
                    multiply(new_mat.object.lambertian.tex->object.sct.albedo, temp1, temp2);
                    materials[additonal_materials] = new_mat;

                    // Each sphere moves from its center C at time t=0 to C+(0,something_non_negative,0) at time t=1
                    world[actual_world_len] =
                        (struct Hittable){.which = (enum Which_Hittable)Sphere,
                                          .object.sphere = {
                                              .center.direction = {0, random_in_range(0, 0.5), 0},
                                              .radius = 0.2,
                                              .mat_cfg = &materials[additonal_materials]}};
                    additonal_materials++;
                }
                else if (choose_mat < 0.95)
                {
                    // metal
                    struct Material_Cfg new_mat =
                        {.which = Metal, .object.metal.fuzz = random_in_range(0, 0.5)};
                    vec_rand_in_range(new_mat.object.metal.albedo, 0.5, 1);
                    materials[additonal_materials] = new_mat;

                    world[actual_world_len] =
                        (struct Hittable){.which = (enum Which_Hittable)Sphere,
                                          .object.sphere = {
                                              .center.direction = {0},
                                              .radius = 0.2,
                                              .mat_cfg = &materials[additonal_materials]}};
                    additonal_materials++;
                }
                else
                {
                    // glass
                    world[actual_world_len] =
                        (struct Hittable){.which = (enum Which_Hittable)Sphere,
                                          .object.sphere = {
                                              .center.direction = {0},
                                              .radius = 0.2,
                                              .mat_cfg = &glass_material}};
                }

                memcpy(world[actual_world_len].object.sphere.center.origin, center, 3 * sizeof(double));
                actual_world_len++;
            }
        }
    }

    world[actual_world_len++] =
        (struct Hittable){.which = (enum Which_Hittable)Sphere,
                          .object.sphere =
                              {.center =
                                   (struct Ray){.origin = {0.0, 1.0, 0.0}, .direction = {0}},
                               .radius = 1.0,
                               .mat_cfg = &glass_material}};

    // The book calls glass_material material1.
    struct Texture material2_texture = {.which = SOLID_COLOR, .object.sct.albedo = {0.4, 0.2, 0.1}};
    const struct Material_Cfg material2 = {.which = Lambertian, .object.lambertian.tex = &material2_texture};

    world[actual_world_len++] =
        (struct Hittable){.which = (enum Which_Hittable)Sphere,
                          .object.sphere =
                              {.center =
                                   (struct Ray){.origin = {-4.0, 1.0, 0.0}, .direction = {0}},
                               .radius = 1.0,
                               .mat_cfg = &material2}};

    const struct Material_Cfg material3 = {.which = Metal,
                                           .object.metal = {.albedo = {0.7, 0.6, 0.5}, .fuzz = 0.0}};

    world[actual_world_len++] =
        (struct Hittable){.which = (enum Which_Hittable)Sphere,
                          .object.sphere =
                              {.center =
                                   (struct Ray){.origin = {4, 1, 0}, .direction = {0}},
                               .radius = 1.0,
                               .mat_cfg = &material3}};

    struct Camera_Config cam =
        {
            .aspect_ratio = 16.0 / 9.0,
            .image_width = 400,
            .samples_per_pixel = 100,
            .max_depth = 50,

            .vfov = 20,
            .lookfrom = {13, 2, 3},
            .lookat = {0, 0, 0},
            .vup = {0, 1, 0},

            .defocus_angle = 0.6,
            .focus_dist = 10.0,
        };

    // Initilize bounding boxes for the Spheres. We do it all here just for readability.
    for (int i = 0; i < actual_world_len; i++)
    {
        if (is_zero(world[i].object.sphere.center.direction))
        {
            // Static Sphere
            sphere_static_bound(&world[i].object.sphere);
        }
        else
        {
            // Moving Sphere
            sphere_moving_bound(&world[i].object.sphere);
        }
    }

    // We rely on the OS to cleanup the malloced memory
    // as we need it for the rest of the program's duration anyhow.
    struct BVH_Node *world_tree = BVH_construct_tree(world, actual_world_len);

    camera_render(world_tree, &cam);

    return 0;
}