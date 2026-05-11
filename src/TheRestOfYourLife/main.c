#include "rtweekend.h"

#include "camera.h"
#include "hittable.h"
#include "hittable_list.h"
#include "sphere.h"
#include "quad.h"
#include "material.h"
#include "bvh.h"
#include "texture.h"
#include "constant_medium.h"

/// How many hittable objects there could possibly be in the world.
/// If we write past the end of an array with this size, the OS throws an exception for us.
#define MAX_WORLD_LENGTH 500

enum Scene
{
    BOUNCING_SPHERES,
    CHECKERED_SPHERES,
    EARTH,
    PERLIN_SPHERES,
    QUADS,
    SIMPLE_LIGHT,
    CORNELL_BOX,
    CORNELL_SMOKE,
    BOOK2_FINAL_LQ,
    BOOK2_FINAL_HQ,
};

/// Which scene to render
#define SCENE_SELECT CORNELL_BOX

#ifndef SCENE_SELECT
#error "SCENE_SELECT must be defined!"
#endif

/// Enable truly random results that vary from run to run.
#define WANT_TRUE_RANDOM

#ifdef WANT_TRUE_RANDOM
#include <time.h>
#endif

void bouncing_spheres()
{

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

    // Initialize bounding boxes for the Spheres. We do it all here just for readability.
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

    struct Camera_Config cam =
        {
            .aspect_ratio = 16.0 / 9.0,
            .image_width = 400,
            .samples_per_pixel = 100,
            .max_depth = 50,
            .background = {0.70, 0.80, 1.00},

            .vfov = 20,
            .lookfrom = {13, 2, 3},
            .lookat = {0, 0, 0},
            .vup = {0, 1, 0},

            .defocus_angle = 0.6,
            .focus_dist = 10.0,
        };

    camera_render(world_tree, &cam);
}

/// @brief Two checkered spheres, one on top of the other.
void checkered_spheres()
{

    // World

    // Textures

    struct Texture checker_texture = {.which = CHECKER};
    make_checker_from_colors(&checker_texture.object.checker, 0.32,
                             (color3){0.2, 0.3, 0.1}, (color3){0.9, 0.9, 0.9});

    // Materials

    const struct Material_Cfg material = {.which = Lambertian,
                                          .object.lambertian.tex = &checker_texture};

    const int actual_world_len = 2;
    struct Hittable world[actual_world_len];
    world[0] =
        (struct Hittable){.which = (enum Which_Hittable)Sphere,
                          .object.sphere =
                              {.center =
                                   (struct Ray){.origin = {0.0, -10.0, 0.0}, .direction = {0}},
                               .radius = 10.0,
                               .mat_cfg = &material}};

    world[1] =
        (struct Hittable){.which = (enum Which_Hittable)Sphere,
                          .object.sphere =
                              {.center =
                                   (struct Ray){.origin = {0.0, 10.0, 0.0}, .direction = {0}},
                               .radius = 10.0,
                               .mat_cfg = &material}};

    // Initialize bounding boxes for the Spheres. We do it here just for readability.
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

    struct Camera_Config cam =
        {
            .aspect_ratio = 16.0 / 9.0,
            .image_width = 400,
            .samples_per_pixel = 100,
            .max_depth = 50,
            .background = {0.70, 0.80, 1.00},

            .vfov = 20,
            .lookfrom = {13, 2, 3},
            .lookat = {0, 0, 0},
            .vup = {0, 1, 0},

            .defocus_angle = 0,
            .focus_dist = 10.0, // This is the default value.
        };

    camera_render(world_tree, &cam);
}

void earth()
{
    // World

    // Textures

    struct Texture earth_texture = {.which = IMAGE};
    image_tex_init(&earth_texture.object.img, "earthmap.jpg");

    // Materials

    const struct Material_Cfg earth_surface = {.which = Lambertian,
                                               .object.lambertian.tex = &earth_texture};

    const int actual_world_len = 1;
    struct Hittable world[actual_world_len];
    // The globe
    world[0] = (struct Hittable){.which = Sphere,
                                 .object.sphere =
                                     {.center = {.origin = {0}, .direction = {0}},
                                      .radius = 2.0,
                                      .mat_cfg = &earth_surface}};

    // Initialize bounding boxes for the Spheres. We do it here just for readability.
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

    struct Camera_Config cam =
        {
            .aspect_ratio = 16.0 / 9.0,
            .image_width = 400,
            .samples_per_pixel = 100,
            .max_depth = 50,
            .background = {0.70, 0.80, 1.00},

            .vfov = 20,
            .lookfrom = {0, 0, 12},
            .lookat = {0, 0, 0},
            .vup = {0, 1, 0},

            .defocus_angle = 0,
            .focus_dist = 10.0, // This is the default value.
        };

    camera_render(world_tree, &cam);
}

void perlin_spheres()
{

    // World

    // Textures

    struct Texture per_text = {.which = NOISE};
    noise_tex_init(&per_text.object.noise, 4.0);

    // Materials

    const struct Material_Cfg mat = {.which = Lambertian,
                                     .object.lambertian.tex = &per_text};

    const int actual_world_len = 2;
    struct Hittable world[actual_world_len];
    world[0] = (struct Hittable){.which = Sphere,
                                 .object.sphere =
                                     {.center = {.origin = {0, -1000, 0}, .direction = {0}},
                                      .radius = 1000,
                                      .mat_cfg = &mat}};

    world[1] = (struct Hittable){.which = Sphere,
                                 .object.sphere =
                                     {.center = {.origin = {0, 2, 0}, .direction = {0}},
                                      .radius = 2.0,
                                      .mat_cfg = &mat}};

    // Initialize bounding boxes for the Spheres.
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

    struct Camera_Config cam =
        {
            .aspect_ratio = 16.0 / 9.0,
            .image_width = 400,
            .samples_per_pixel = 100,
            .max_depth = 50,
            .background = {0.70, 0.80, 1.00},

            .vfov = 20,
            .lookfrom = {13, 2, 3},
            .lookat = {0, 0, 0},
            .vup = {0, 1, 0},

            .defocus_angle = 0,
            .focus_dist = 10.0, // This is the default value.
        };

    camera_render(world_tree, &cam);
}

void quads()
{
    // World

    // Textures

    struct Texture red = {.which = SOLID_COLOR, .object.sct.albedo = {1.0, 0.2, 0.2}};
    struct Texture green = {.which = SOLID_COLOR, .object.sct.albedo = {0.2, 1.0, 0.2}};
    struct Texture blue = {.which = SOLID_COLOR, .object.sct.albedo = {0.2, 0.2, 1.0}};
    struct Texture orange = {.which = SOLID_COLOR, .object.sct.albedo = {1.0, 0.5, 0.0}};
    struct Texture teal = {.which = SOLID_COLOR, .object.sct.albedo = {0.2, 0.8, 0.8}};

    // Materials

    const struct Material_Cfg left_red = {.which = Lambertian, .object.lambertian.tex = &red};
    const struct Material_Cfg back_green = {.which = Lambertian, .object.lambertian.tex = &green};
    const struct Material_Cfg right_blue = {.which = Lambertian, .object.lambertian.tex = &blue};
    const struct Material_Cfg upper_orange = {.which = Lambertian, .object.lambertian.tex = &orange};
    const struct Material_Cfg lower_teal = {.which = Lambertian, .object.lambertian.tex = &teal};

    const int actual_world_len = 5;
    struct Hittable world[actual_world_len];

    world[0] = (struct Hittable){.which = Quad,
                                 .object.quad =
                                     {.Q = {-3, -2, 5}, .u = {0, 0, -4}, .v = {0, 4, 0}, .mat_cfg = &left_red}};

    world[1] = (struct Hittable){.which = Quad,
                                 .object.quad =
                                     {.Q = {-2, -2, 0}, .u = {4, 0, 0}, .v = {0, 4, 0}, .mat_cfg = &back_green}};

    world[2] = (struct Hittable){.which = Quad,
                                 .object.quad =
                                     {.Q = {3, -2, 1}, .u = {0, 0, 4}, .v = {0, 4, 0}, .mat_cfg = &right_blue}};

    world[3] = (struct Hittable){.which = Quad,
                                 .object.quad =
                                     {.Q = {-2, 3, 1}, .u = {4, 0, 0}, .v = {0, 0, 4}, .mat_cfg = &upper_orange}};

    world[4] = (struct Hittable){.which = Quad,
                                 .object.quad =
                                     {.Q = {-2, -3, 5}, .u = {4, 0, 0}, .v = {0, 0, -4}, .mat_cfg = &lower_teal}};

    // Initialize bounding boxes for the quads.
    for (int i = 0; i < actual_world_len; i++)
    {
        quad_init(&world[i].object.quad);
    }

    // We rely on the OS to cleanup the malloced memory
    // as we need it for the rest of the program's duration anyhow.
    struct BVH_Node *world_tree = BVH_construct_tree(world, actual_world_len);

    struct Camera_Config cam =
        {
            .aspect_ratio = 1.0,
            .image_width = 400,
            .samples_per_pixel = 100,
            .max_depth = 50,
            .background = {0.70, 0.80, 1.00},

            .vfov = 80,
            .lookfrom = {0, 0, 9},
            .lookat = {0, 0, 0},
            .vup = {0, 1, 0},

            .defocus_angle = 0,
            .focus_dist = 10.0, // This is the default value.
        };

    camera_render(world_tree, &cam);
}

void simple_light()
{

    // World

    // Textures

    struct Texture per_text = {.which = NOISE};
    noise_tex_init(&per_text.object.noise, 4.0);

    // Note that the light is brighter than (1,1,1). This allows it to be bright enough to light things.
    struct Texture diff_light_tex = {.which = SOLID_COLOR, .object.sct.albedo = {4, 4, 4}};

    // Materials

    const struct Material_Cfg per_mat = {.which = Lambertian,
                                         .object.lambertian.tex = &per_text};
    const struct Material_Cfg light_mat = {.which = Light, .object.light.tex = &diff_light_tex};

    const int actual_world_len = 4;
    struct Hittable world[actual_world_len];
    world[0] = (struct Hittable){.which = Sphere,
                                 .object.sphere =
                                     {.center = {.origin = {0, -1000, 0}, .direction = {0}},
                                      .radius = 1000,
                                      .mat_cfg = &per_mat}};

    world[1] = (struct Hittable){.which = Sphere,
                                 .object.sphere =
                                     {.center = {.origin = {0, 2, 0}, .direction = {0}},
                                      .radius = 2.0,
                                      .mat_cfg = &per_mat}};

    world[2] = (struct Hittable){.which = Quad,
                                 .object.quad =
                                     {.Q = {3, 1, -2}, .u = {2, 0, 0}, .v = {0, 2, 0}, .mat_cfg = &light_mat}};

    world[3] = (struct Hittable){.which = Sphere,
                                 .object.sphere =
                                     {.center = {.origin = {0, 7, 0}, .direction = {0}},
                                      .radius = 2.0,
                                      .mat_cfg = &light_mat}};

    // Init bounding boxes
    sphere_static_bound(&world[0].object.sphere);
    sphere_static_bound(&world[1].object.sphere);
    quad_init(&world[2].object.quad);
    sphere_static_bound(&world[3].object.sphere);

    // We rely on the OS to cleanup the malloced memory
    // as we need it for the rest of the program's duration anyhow.
    struct BVH_Node *world_tree = BVH_construct_tree(world, actual_world_len);

    struct Camera_Config cam =
        {
            .aspect_ratio = 16.0 / 9.0,
            .image_width = 400,
            .samples_per_pixel = 100,
            .max_depth = 50,
            .background = {0, 0, 0}, // black

            .vfov = 20,
            .lookfrom = {26, 3, 6},
            .lookat = {0, 2, 0},
            .vup = {0, 1, 0},

            .defocus_angle = 0,
            .focus_dist = 10.0, // This is the default value.
        };

    camera_render(world_tree, &cam);
}

/// @brief The “Cornell Box” was introduced in 1984 to model the interaction of light between diffuse surfaces.
/// An empty Cornell Box has just 5 walls and a light.
void cornell_box()
{
    // World

    // Textures

    struct Texture red = {.which = SOLID_COLOR, .object.sct.albedo = {.65, .05, .05}};
    struct Texture white = {.which = SOLID_COLOR, .object.sct.albedo = {.73, .73, .73}};
    struct Texture green = {.which = SOLID_COLOR, .object.sct.albedo = {.12, .45, .15}};
    // Note that the light is brighter than (1,1,1). This allows it to be bright enough to light things.
    struct Texture diff_light_tex = {.which = SOLID_COLOR, .object.sct.albedo = {15, 15, 15}};

    // Materials

    const struct Material_Cfg red_mat = {.which = Lambertian, .object.lambertian.tex = &red};
    const struct Material_Cfg white_mat = {.which = Lambertian, .object.lambertian.tex = &white};
    const struct Material_Cfg green_mat = {.which = Lambertian, .object.lambertian.tex = &green};
    const struct Material_Cfg light_mat = {.which = Light, .object.light.tex = &diff_light_tex};

    int actual_world_len = 6;
    struct Hittable world[MAX_WORLD_LENGTH];

    world[0] = (struct Hittable){.which = Quad,
                                 .object.quad =
                                     {.Q = {555, 0, 0},
                                      .u = {0, 555, 0},
                                      .v = {0, 0, 555},
                                      .mat_cfg = &green_mat}};

    world[1] = (struct Hittable){.which = Quad,
                                 .object.quad =
                                     {.Q = {0, 0, 0},
                                      .u = {0, 555, 0},
                                      .v = {0, 0, 555},
                                      .mat_cfg = &red_mat}};

    world[2] = (struct Hittable){.which = Quad,
                                 .object.quad =
                                     {.Q = {343, 554, 332},
                                      .u = {-130, 0, 0},
                                      .v = {0, 0, -105},
                                      .mat_cfg = &light_mat}};

    world[3] = (struct Hittable){.which = Quad,
                                 .object.quad =
                                     {.Q = {0, 0, 0},
                                      .u = {555, 0, 0},
                                      .v = {0, 0, 555},
                                      .mat_cfg = &white_mat}};

    world[4] = (struct Hittable){.which = Quad,
                                 .object.quad =
                                     {.Q = {555, 555, 555},
                                      .u = {-555, 0, 0},
                                      .v = {0, 0, -555},
                                      .mat_cfg = &white_mat}};
    world[5] = (struct Hittable){.which = Quad,
                                 .object.quad =
                                     {.Q = {0, 0, 555},
                                      .u = {555, 0, 0},
                                      .v = {0, 555, 0},
                                      .mat_cfg = &white_mat}};

    // Initialize bounding boxes for these 6 quads.
    for (int i = 0; i < actual_world_len; i++)
    {
        quad_init(&world[i].object.quad);
    }

    // const struct Material_Cfg aluminum = {.which = Metal,
    //                                       .object.metal = {.albedo = {0.8, 0.85, 0.88}, .fuzz = 0.0}};

    struct Hittable *box1 = world_add_box_rotated_translated((point3){0, 0, 0}, (point3){165, 330, 165},
                                                             &white_mat,
                                                             (vec3){265, 0, 295}, 15);

    // Copy box1 (just the final 6 quads post operations) into world
    memcpy(&world[actual_world_len], &box1[12], sizeof(struct Hittable) * 6);
    actual_world_len += 6;

    // struct Hittable *box2 = world_add_box_rotated_translated((point3){0, 0, 0}, (point3){165, 165, 165},
    //                                                          &white_mat,
    //                                                          (vec3){130, 0, 65}, -18);

    // // Copy box2 (just the final 6 quads post operations) into world
    // memcpy(&world[actual_world_len], &box2[12], sizeof(struct Hittable) * 6);
    // actual_world_len += 6;

    // Glass Sphere
    const struct Material_Cfg glass_material =
        {.which = Dielectric, .object.dielectric.refraction_index = 1.5};

    world[actual_world_len] =
        (struct Hittable){.which = (enum Which_Hittable)Sphere,
                          .object.sphere =
                              {.center =
                                   (struct Ray){.origin = {190, 90, 190}, .direction = {0}},
                               .radius = 90,
                               .mat_cfg = &glass_material}};

    int glass_sphere_idx = actual_world_len;
    // Static Sphere
    sphere_static_bound(&world[glass_sphere_idx].object.sphere);
    actual_world_len++;

    // Light Sources (OR just important things we would like to sample towards)
    // (Note these *ARE* already in the scene and properly initialized)
    g_lights_size = 2;
    g_lights = malloc(sizeof(struct Hittable) * g_lights_size);
    g_lights[0] = world[2]; // The ceiling light
    g_lights[1] = world[glass_sphere_idx];

    // We rely on the OS to cleanup the malloced memory
    // as we need it for the rest of the program's duration anyhow.
    struct BVH_Node *world_tree = BVH_construct_tree(world, actual_world_len);

    struct Camera_Config cam =
        {
            .aspect_ratio = 1.0,
            .image_width = 600,
            .samples_per_pixel = 1000,
            .max_depth = 50,
            .background = {0, 0, 0},

            .vfov = 40,
            .lookfrom = {278, 278, -800},
            .lookat = {278, 278, 0},
            .vup = {0, 1, 0},

            .defocus_angle = 0,
            .focus_dist = 10.0, // This is the default value.
        };

    camera_render(world_tree, &cam);
}

/// @brief If we replace the two blocks with smoke and fog (dark and light particles),
/// and make the light bigger (and dimmer so it doesn’t blow out the scene) for faster convergence
void cornell_smoke()
{
    // World

    // Textures

    struct Texture red = {.which = SOLID_COLOR, .object.sct.albedo = {.65, .05, .05}};
    struct Texture white = {.which = SOLID_COLOR, .object.sct.albedo = {.73, .73, .73}};
    struct Texture green = {.which = SOLID_COLOR, .object.sct.albedo = {.12, .45, .15}};
    // Note that the light is brighter than (1,1,1). This allows it to be bright enough to light things.
    struct Texture diff_light_tex = {.which = SOLID_COLOR, .object.sct.albedo = {7, 7, 7}};

    struct Texture smoke = {.which = SOLID_COLOR, .object.sct.albedo = {0, 0, 0}};
    struct Texture fog = {.which = SOLID_COLOR, .object.sct.albedo = {1, 1, 1}};

    // Materials

    const struct Material_Cfg red_mat = {.which = Lambertian, .object.lambertian.tex = &red};
    const struct Material_Cfg white_mat = {.which = Lambertian, .object.lambertian.tex = &white};
    const struct Material_Cfg green_mat = {.which = Lambertian, .object.lambertian.tex = &green};
    const struct Material_Cfg light_mat = {.which = Light, .object.light.tex = &diff_light_tex};

    const struct Material_Cfg smoke_mat = {.which = Isotropic, .object.isotropic.tex = &smoke};
    const struct Material_Cfg fog_mat = {.which = Isotropic, .object.isotropic.tex = &fog};

    const int actual_world_len = 8;
    struct Hittable world[actual_world_len];

    world[0] = (struct Hittable){.which = Quad,
                                 .object.quad =
                                     {.Q = {555, 0, 0},
                                      .u = {0, 555, 0},
                                      .v = {0, 0, 555},
                                      .mat_cfg = &green_mat}};

    world[1] = (struct Hittable){.which = Quad,
                                 .object.quad =
                                     {.Q = {0, 0, 0},
                                      .u = {0, 555, 0},
                                      .v = {0, 0, 555},
                                      .mat_cfg = &red_mat}};

    world[2] = (struct Hittable){.which = Quad,
                                 .object.quad =
                                     {.Q = {113, 554, 127},
                                      .u = {330, 0, 0},
                                      .v = {0, 0, 305},
                                      .mat_cfg = &light_mat}};

    world[3] = (struct Hittable){.which = Quad,
                                 .object.quad =
                                     {.Q = {0, 555, 0},
                                      .u = {555, 0, 0},
                                      .v = {0, 0, 555},
                                      .mat_cfg = &white_mat}};

    world[4] = (struct Hittable){.which = Quad,
                                 .object.quad =
                                     {.Q = {0, 0, 0},
                                      .u = {555, 0, 0},
                                      .v = {0, 0, 555},
                                      .mat_cfg = &white_mat}};
    world[5] = (struct Hittable){.which = Quad,
                                 .object.quad =
                                     {.Q = {0, 0, 555},
                                      .u = {555, 0, 0},
                                      .v = {0, 555, 0},
                                      .mat_cfg = &white_mat}};

    // Initialize bounding boxes for these 6 quads.
    for (int i = 0; i < 6; i++)
    {
        quad_init(&world[i].object.quad);
    }

    struct Hittable *box1 = world_add_box_rotated_translated((point3){0, 0, 0}, (point3){165, 330, 165},
                                                             &white_mat,
                                                             (vec3){265, 0, 295}, 15);
    // Make box1 into a smoke (dark particles) volume and have just that *1* object in the world.
    world[6] = (struct Hittable){.which = Constant_Medium};
    // Remember box1 is made up of just the final 6 quads (post rotation and translation)
    cm_init(&world[6].object.cm, &box1[12], 6, 0.01, &smoke_mat);

    struct Hittable *box2 = world_add_box_rotated_translated((point3){0, 0, 0}, (point3){165, 165, 165},
                                                             &white_mat,
                                                             (vec3){130, 0, 65}, -18);

    // Make box2 into a fog (light particles) volume and have just that *1* object in the world.
    world[7] = (struct Hittable){.which = Constant_Medium};
    // Remember box2 is made up of just the final 6 quads (post rotation and translation)
    cm_init(&world[7].object.cm, &box2[12], 6, 0.01, &fog_mat);

    // We rely on the OS to cleanup the malloced memory
    // as we need it for the rest of the program's duration anyhow.
    struct BVH_Node *world_tree = BVH_construct_tree(world, actual_world_len);

    struct Camera_Config cam =
        {
            .aspect_ratio = 1.0,
            .image_width = 600,
            .samples_per_pixel = 200,
            .max_depth = 50,
            .background = {0, 0, 0},

            .vfov = 40,
            .lookfrom = {278, 278, -800},
            .lookat = {278, 278, 0},
            .vup = {0, 1, 0},

            .defocus_angle = 0,
            .focus_dist = 10.0, // This is the default value.
        };

    camera_render(world_tree, &cam);
}

void book2_final_scene(int image_width, int samples_per_pixel, int max_depth)
{

    // World

    // Textures

    struct Texture ground_tex = {.which = SOLID_COLOR, .object.sct.albedo = {0.48, 0.83, 0.53}};
    struct Texture brown_tex = {.which = SOLID_COLOR, .object.sct.albedo = {0.7, 0.3, 0.1}};
    // Note that the light is brighter than (1,1,1). This allows it to be bright enough to light things.
    struct Texture diff_light_tex = {.which = SOLID_COLOR, .object.sct.albedo = {7, 7, 7}};

    struct Texture blue_volume = {.which = SOLID_COLOR, .object.sct.albedo = {0.2, 0.4, 0.9}};
    struct Texture mist = {.which = SOLID_COLOR, .object.sct.albedo = {1, 1, 1}};

    struct Texture earth_texture = {.which = IMAGE};
    image_tex_init(&earth_texture.object.img, "earthmap.jpg");

    struct Texture per_text = {.which = NOISE};
    noise_tex_init(&per_text.object.noise, 0.2);

    struct Texture white = {.which = SOLID_COLOR, .object.sct.albedo = {.73, .73, .73}};

    // Materials

    const struct Material_Cfg ground_mat = {.which = Lambertian, .object.lambertian.tex = &ground_tex};
    const struct Material_Cfg mov_sphere_mat = {.which = Lambertian, .object.lambertian.tex = &brown_tex};
    const struct Material_Cfg glass = {.which = Dielectric, .object.dielectric.refraction_index = 1.5};
    const struct Material_Cfg metal = {.which = Metal, .object.metal = {.albedo = {0.8, 0.8, 0.9}, .fuzz = 1.0}};
    const struct Material_Cfg light_mat = {.which = Light, .object.light.tex = &diff_light_tex};
    const struct Material_Cfg volume_mat = {.which = Isotropic, .object.isotropic.tex = &blue_volume};
    const struct Material_Cfg mist_mat = {.which = Isotropic, .object.isotropic.tex = &mist};
    const struct Material_Cfg earth_surface = {.which = Lambertian, .object.lambertian.tex = &earth_texture};
    const struct Material_Cfg perlin_mat = {.which = Lambertian, .object.lambertian.tex = &per_text};
    const struct Material_Cfg white_mat = {.which = Lambertian, .object.lambertian.tex = &white};

    const int boxes_per_side = 20;
    struct Hittable world[MAX_WORLD_LENGTH];

    // Create the ground
    for (int i = 0; i < boxes_per_side; i++)
    {
        for (int j = 0; j < boxes_per_side; j++)
        {
            double w = 100.0;
            double x0 = -1000.0 + i * w;
            double z0 = -1000.0 + j * w;
            double y0 = 0.0;
            double x1 = x0 + w;
            double y1 = random_in_range(1, 101);
            double z1 = z0 + w;
            struct Hittable *cur_box = world_add_box((point3){x0, y0, z0}, (point3){x1, y1, z1}, &ground_mat);
            world[(20 * i) + j] = (struct Hittable){.which = Composite_Hittable,
                                                    .object.comp =
                                                        {.root =
                                                             BVH_construct_tree(cur_box, 6)}};
        }
    }

    int actual_world_len = boxes_per_side * boxes_per_side;

    // Light
    world[actual_world_len] =
        (struct Hittable){.which = Quad,
                          .object.quad =
                              {.Q = {123, 554, 147},
                               .u = {300, 0, 0},
                               .v = {0, 0, 265},
                               .mat_cfg = &light_mat}};
    quad_init(&world[actual_world_len].object.quad);
    actual_world_len++;

    // Brown moving sphere
    world[actual_world_len] =
        (struct Hittable){.which = (enum Which_Hittable)Sphere,
                          .object.sphere =
                              {.center =
                                   (struct Ray){.origin = {400, 400, 200}, .direction = {30, 0, 0}},
                               .radius = 50.0,
                               .mat_cfg = &mov_sphere_mat}};
    sphere_moving_bound(&world[actual_world_len].object.sphere);
    actual_world_len++;

    // Glass Sphere
    world[actual_world_len] =
        (struct Hittable){.which = (enum Which_Hittable)Sphere,
                          .object.sphere =
                              {.center =
                                   (struct Ray){.origin = {260, 150, 45}, .direction = {0, 0, 0}},
                               .radius = 50.0,
                               .mat_cfg = &glass}};
    sphere_static_bound(&world[actual_world_len].object.sphere);
    actual_world_len++;

    // Metal Sphere
    world[actual_world_len] =
        (struct Hittable){.which = (enum Which_Hittable)Sphere,
                          .object.sphere =
                              {.center =
                                   (struct Ray){.origin = {0, 150, 145}, .direction = {0, 0, 0}},
                               .radius = 50.0,
                               .mat_cfg = &metal}};
    sphere_static_bound(&world[actual_world_len].object.sphere);
    actual_world_len++;

    // Add Glass Sphere with blue volume inside it (we add both the Sphere and the volume to the scene space).
    // This makes for a blue subsurface reflection sphere.
    world[actual_world_len] = (struct Hittable){.which = (enum Which_Hittable)Sphere,
                                                .object.sphere =
                                                    {.center =
                                                         (struct Ray){.origin = {360, 150, 145},
                                                                      .direction = {0, 0, 0}},
                                                     .radius = 70.0,
                                                     .mat_cfg = &glass}};
    sphere_static_bound(&world[actual_world_len].object.sphere);
    actual_world_len++;

    world[actual_world_len] = (struct Hittable){.which = Constant_Medium};
    cm_init(&world[actual_world_len].object.cm,
            &world[actual_world_len - 1], 1, 0.2, &volume_mat);
    actual_world_len++;

    // Big thin white mist covering everything
    // Note we do NOT add this boundary to the scene space.
    struct Hittable boundary[1];
    boundary[0] = (struct Hittable){.which = (enum Which_Hittable)Sphere,
                                    .object.sphere =
                                        {.center =
                                             (struct Ray){.origin = {0, 0, 0}, .direction = {0, 0, 0}},
                                         .radius = 5000.0,
                                         .mat_cfg = &glass}};
    sphere_static_bound(&boundary[0].object.sphere);

    world[actual_world_len] = (struct Hittable){.which = Constant_Medium};
    cm_init(&world[actual_world_len].object.cm, boundary, 1, 0.0001, &mist_mat);
    actual_world_len++;

    // Earth
    world[actual_world_len] =
        (struct Hittable){.which = (enum Which_Hittable)Sphere,
                          .object.sphere =
                              {.center =
                                   (struct Ray){.origin = {400, 200, 400}, .direction = {0, 0, 0}},
                               .radius = 100.0,
                               .mat_cfg = &earth_surface}};
    sphere_static_bound(&world[actual_world_len].object.sphere);
    actual_world_len++;

    // Perlin Noise Sphere
    world[actual_world_len] =
        (struct Hittable){.which = (enum Which_Hittable)Sphere,
                          .object.sphere =
                              {.center =
                                   (struct Ray){.origin = {220, 280, 300}, .direction = {0, 0, 0}},
                               .radius = 80.0,
                               .mat_cfg = &perlin_mat}};
    sphere_static_bound(&world[actual_world_len].object.sphere);
    actual_world_len++;

    // Box made of small white spheres (note this results in only adding 1 entry in our world array)
    const int ns = 1000;
    struct Hittable boxes2[ns];

    for (int j = 0; j < ns; j++)
    {
        boxes2[j] = (struct Hittable){.which = (enum Which_Hittable)Sphere,
                                      .object.sphere =
                                          {.center =
                                               (struct Ray){.direction = {0, 0, 0}},
                                           .radius = 10.0,
                                           .mat_cfg = &white_mat}};
        vec_rand_in_range(boxes2[j].object.sphere.center.origin, 0, 165);
        sphere_static_bound(&boxes2[j].object.sphere);
    }

    struct Hittable composite_box = {.which = Composite_Hittable,
                                     .object.comp =
                                         {.root = BVH_construct_tree(boxes2, ns)}};

    struct Hittable rot = {.which = Rotate_y, .object.rotate_y.rotate_object = &composite_box};
    rotate_init(&rot.object.rotate_y, 15);

    world[actual_world_len] =
        (struct Hittable){.which = (enum Which_Hittable)Translate,
                          .object.translate = {.tran_object = &rot, .offset = {-100, 270, 395}}};
    tran_init(&world[actual_world_len].object.translate);
    actual_world_len++;

    // We rely on the OS to cleanup the malloced memory
    // as we need it for the rest of the program's duration anyhow.
    struct BVH_Node *world_tree = BVH_construct_tree(world, actual_world_len);

    struct Camera_Config cam =
        {
            .aspect_ratio = 1.0,
            .image_width = image_width,
            .samples_per_pixel = samples_per_pixel,
            .max_depth = max_depth,
            .background = {0, 0, 0},

            .vfov = 40,
            .lookfrom = {478, 278, -600},
            .lookat = {278, 278, 0},
            .vup = {0, 1, 0},

            .defocus_angle = 0,
            .focus_dist = 10.0, // This is the default value.
        };

    camera_render(world_tree, &cam);
}

int main()
{

#ifdef WANT_TRUE_RANDOM
    // Seed the random number generator (which rand() uses) with the current time.
    srand((unsigned int)time(NULL));
#endif

    switch (SCENE_SELECT)
    {
    case (enum Scene)BOUNCING_SPHERES:
        bouncing_spheres();
        break;
    case (enum Scene)CHECKERED_SPHERES:
        checkered_spheres();
        break;
    case (enum Scene)EARTH:
        earth();
        break;
    case (enum Scene)PERLIN_SPHERES:
        perlin_spheres();
        break;
    case (enum Scene)QUADS:
        quads();
        break;
    case (enum Scene)SIMPLE_LIGHT:
        simple_light();
        break;
    case (enum Scene)CORNELL_BOX:
        cornell_box();
        break;
    case (enum Scene)CORNELL_SMOKE:
        cornell_smoke();
        break;
    case (enum Scene)BOOK2_FINAL_LQ:
        book2_final_scene(400, 250, 4);
        break;
    case (enum Scene)BOOK2_FINAL_HQ:
        book2_final_scene(800, 10000, 40);
        break;
    default:
        fprintf(stderr, "Invalid scene selection!\n");
        fflush(stderr);
        break;
    }
}