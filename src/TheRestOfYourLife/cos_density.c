#include "rtweekend.h"

double f(const vec3 d)
{
    double cos_theta = d[2];
    return cos_theta * cos_theta * cos_theta;
}

// The pdf is cos(θ)/π
double pdf(const vec3 d)
{
    return d[2] / pi;
}

// Integration with cosine density function (Book 3 Section 7.3) of cos^3(θ) over the unit hemisphere.
// We should get the same answer as cos_cubed.c . We are simply doing Monte-Carlo Integration of the same
// target function with a different sampling PDF.
// This is a check we are *actually* doing our sampling according to this PDF that we claim to.
int main()
{
    int N = 1000000;

    double sum = 0.0;
    for (int i = 0; i < N; i++)
    {
        vec3 d;
        random_cosine_direction(d);
        sum += f(d) / pdf(d);
    }

    printf("PI/2 = %.12f \n", (pi / 2.0));
    printf("Estimate = %.12f \n", (sum / N));
}