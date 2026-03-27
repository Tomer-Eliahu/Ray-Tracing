// Book 3: Section 4:

#include "rtweekend.h"

double f(const vec3 d)
{
    double cosine_squared = d[2] * d[2];
    return cosine_squared;
}

double pdf([[maybe_unused]] const vec3 d)
{
    return 1 / (4 * pi);
}

/// @brief Integrating f = cos^2(theta) on the surface of the unit sphere.
int main()
{
    int N = 1000000;
    double sum = 0.0;
    for (int i = 0; i < N; i++)
    {
        vec3 d;
        random_unit_vector(d);
        double f_d = f(d);
        sum += f_d / pdf(d);
    }
    printf("I = %.12f \n", (sum / N));
}