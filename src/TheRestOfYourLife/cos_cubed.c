#include "rtweekend.h"

/* Book 3 Section 7.2: Uniform Sampling a Hemisphere

We found that to generate a unifrom random unit vector on the Hemisphere using the inversion method
we have that:

    x=cos(2πr_1)⋅sqrt( r_2(2−r_2) )
    y=sin(2πr_1)⋅sqrt( r_2(2−r_2) )
    z=1−r_2

where r_1 and r_2 are uniformly distributed on [0,1].


We can test we got this right by plotting points like we did in sphere_plot.c
OR by solving for a 2D integral with a known solution using Monte-Carlo Integration
(as we know that we will converge to the correct solution if we *actually* generate samples according to the pdf
we claim we do).

So we do integration with importance sampling: p(ω) = 1/2π (we claim we generate samples uniformly on the
hemisphere), so we average f()/p() = cos^3(θ)/(1/2π)
and see if we converge to the correct result (which we solved analytically that the integral of cos^3(θ)
on the hemisphere is π/2 ).

*/

double f(double r2)
{
    // auto x = std::cos(2*pi*r1) * 2 * std::sqrt(r2*(1-r2));
    // auto y = std::sin(2*pi*r1) * 2 * std::sqrt(r2*(1-r2));
    double z = 1 - r2;
    double cos_theta = z;
    return cos_theta * cos_theta * cos_theta;
}

double pdf()
{
    return 1.0 / (2.0 * pi);
}

int main()
{
    int N = 1000000;

    double sum = 0.0;
    for (int i = 0; i < N; i++)
    {
        double r2 = random_zero_to_one();
        sum += f(r2) / pdf();
    }

    printf("PI/2 = %.12f \n", (pi / 2.0));
    printf("Estimate = %.12f \n", (sum / N));
}