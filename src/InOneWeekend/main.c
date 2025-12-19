#include <stdio.h>

int main()
{

    /*
        We will render images (run build\inOneWeekend.exe > image.ppm).
        We use the ppm format of writing some numbers to a file to describe the image.

        You can use https://jumpshare.com/viewer/ppm to view the image (no download needed)
        or this extension (PBM/PPM/PGM Viewer for Visual Studio Code -- what I am using).
    */

    // Image

    int image_width = 256;
    int image_height = 256;

    // Render

    printf("P3\n");                               // This means the colors will be in ASCII
    printf("%i %i\n", image_width, image_height); // how many pixels to make
    printf("255\n");                               // Max color possible

    for (int i = 0; i < image_height; i++)
    {
        for (int j = 0; j < image_width; j++)
        {
            double r = (double) j / (image_width - 1);
            double g = (double) i/ (image_height - 1);
            double b = 0.0;

            /*
                By convention, each of the red/green/blue components are represented internally
                by real-valued variables that range from 0.0 to 1.0.
                These must be scaled to integer values between 0 and 255 before we print them out.
            */

            int ir = (int) 255.999 * r;
            int ig = (int) 255.999 * g;
            int ib = (int) 255.999 * b;

            printf("%i %i %i\n", ir, ig, ib);
        }
    }

    return 0;
}