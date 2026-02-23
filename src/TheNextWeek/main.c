#include "rtweekend.h"

#include "camera.h"
#include "hittable.h"
#include "hittable_list.h"
#include "sphere.h"
#include "quad.h"
#include "material.h"
#include "bvh.h"
#include "texture.h"

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
    CORNELL_BOX
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

    struct Hittable *box1 = world_add_box_rotated_translated((point3){0, 0, 0}, (point3){165, 330, 165},
                                                             &white_mat,
                                                             (vec3){265, 0, 295}, 15);

    // Copy box1 (just the final 6 quads post operations) into world
    memcpy(&world[actual_world_len], &box1[12], sizeof(struct Hittable) * 6);
    actual_world_len += 6;

    struct Hittable *box2 = world_add_box_rotated_translated((point3){0, 0, 0}, (point3){165, 165, 165},
                                                             &white_mat,
                                                             (vec3){130, 0, 65}, -18);

    // Copy box2 (just the final 6 quads post operations) into world
    memcpy(&world[actual_world_len], &box2[12], sizeof(struct Hittable) * 6);
    actual_world_len += 6;

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
    default:
        fprintf(stderr, "Invalid scene selection!\n");
        fflush(stderr);
        break;
    }
}