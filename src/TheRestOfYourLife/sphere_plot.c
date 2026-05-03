#include "rtweekend.h"

/* Book 3 Section 7: Generating Random Directions notes

Note that now we have (we now think of the z-axis as the "up" direction as we assume it is the surface normal):
θ is in [0, π]
φ is in [0, 2π]

To generate a uniform unit vector direction on the surface of the sphere using the inversion method
(as opposed to the rejection method)
that has the marginal PDFs for θ and φ in the book that we want,

we found:
    x=cos(2πr_1)⋅2sqrt( r_2(1−r_2) )
    y=sin(2πr_1)⋅2sqrt( r_2(1−r_2) )
    z=1−2r_2

where r_1 and r_2 are uniformly distributed on [0,1].

*/

// We can plot the output for free online on https://www.desmos.com/3d
int main()
{
    for (int i = 0; i < 200; i++)
    {
        double r1 = random_zero_to_one();
        double r2 = random_zero_to_one();
        double x = cos(2 * pi * r1) * 2 * sqrt(r2 * (1 - r2));
        double y = sin(2 * pi * r1) * 2 * sqrt(r2 * (1 - r2));
        double z = 1 - 2 * r2;
        printf("(%f,%f,%f)\n", x, y, z);
    }
}