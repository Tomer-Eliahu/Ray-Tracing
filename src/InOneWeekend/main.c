#include "rtweekend.h"

#include "camera.h"
#include "hittable.h"
#include "hittable_list.h"
#include "sphere.h"

#define WORLD_LENGTH 2 // How many hittable objects there are in the world.

int main()
{

    /*
        We will render images (run build\inOneWeekend.exe > image.ppm).
        We use the ppm format of writing some numbers to a file to describe the image.

        You can use https://jumpshare.com/viewer/ppm to view the image (no download needed)
        or this extension (PBM/PPM/PGM Viewer for Visual Studio Code -- what I am using).
    */

    // World
    struct Hittable world[WORLD_LENGTH] = {
        {.which = (enum Which_Hittable)Sphere, .object.sphere = {.center = {0, 0, -1}, .radius = 0.5}},
        {.which = (enum Which_Hittable)Sphere, .object.sphere = {.center = {0, -100.5, -1}, .radius = 100}},
    };

    struct Camera_Config cam = {.aspect_ratio = 16.0 / 9.0, .image_width = 400};

    camera_render(world, WORLD_LENGTH, &cam);

    return 0;
}