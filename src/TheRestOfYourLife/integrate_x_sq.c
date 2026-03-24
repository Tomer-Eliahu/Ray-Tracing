
/*Recall in Book 3, section 3: One Dimensional Monte Carlo Integration

We derived that

E[f(x)|a≤x≤b]=(1/(b−a))*area_under_the_curve(f(x),a,b) = (1/(b−a))*(the integral from a to b of f(x))

assuming the probability of each xi in [a,b] is the same (equally likely outcomes of a random variable X).

Note that E[f(X)] is the mean of the function over that interval.

Both the integral of a function and a Monte Carlo sampling of that function can be used to solve
for the average over a specific interval.
While integration solves for the average with the
sum of infinitely many infinitesimally small slices of the interval,
a Monte Carlo algorithm will approximate the same average by
solving the sum of ever increasing random sample points within the interval.

*/

/*
Integrating x²
Let’s look at a classic integral:

I= The integral from 0 to 2 of (x^2)

from calculus we know
I = (1/3 * x^3 evaluated at 2) - (1/3 * x^3 evaluated at 0) = 8/3.

We could solve the integral using a Monte Carlo approach (by approximation).
In computer sciency notation, we might write this as:

I=area(x^2,0,2)
We could also write it as:

E[f(x)|a≤x≤b]=(1/(b−a))*area(f(x),a,b)
average(x^2,0,2)=(1/2)*area(x^2,0,2)
average(x^2,0,2)=(1/2)*I
I=2*average(x^2,0,2)

Where we use the Monte Carlo method to approximate average(x^2,0,2).

In the general case (but still assuming X is a uniform random variable over the interval)
it would be:
E[f(x)|a≤x≤b]=(1/(b−a))*area(f(x),a,b)
area(f(x),a,b)=(b-a)*average(f(x),a,b)

*/

#include "rtweekend.h"

int main()
{
    int a = 0;
    int b = 2;
    int N = 1000000;
    double sum = 0.0;

    for (int i = 0; i < N; i++)
    {
        double x = random_in_range(a, b);
        sum += x * x;
    }

    printf("I = %.12f \n", (b - a) * (sum / N));
}

/*
You could rightly point to this example and say that the integration is actually a lot less work than the Monte Carlo.
That might be true in the case where the function is f(x)=x^2,
but there exist many functions where it might be simpler to solve for the Monte Carlo than for the integration,
like f(x)= (sin(x))^5.
We could also use the Monte Carlo algorithm for functions where an analytical integration
is difficult or impossible to express in elementary terms,
like f(x)= ln(sin(x)).

In graphics, we often have functions that we can write down explicitly but that have a
complicated analytic integration, or, just as often,
we have functions that can be evaluated but that can't be written down explicitly
(I think the author means can't be written down explicitly in a nice way),
and we will frequently find ourselves with a function that can only be evaluated probabilistically.
The function ray_color from the first two books
is an example of a function that can only be determined probabilistically.
We can’t know what color can be seen from any given place in all directions
(that is given an input ray to ray_color,
there is not a simple expression for the output we can use evaluate ray_color for that ray),
but we can statistically estimate which color can be seen from one particular place,
for a single particular direction.

*/

/*
The ray_color function that we wrote in the first two books, while elegant in its simplicity,
has a fairly major problem.
Small light sources create too much noise.
This is because our uniform sampling doesn’t sample these light sources often enough.
Light sources are only sampled if a ray scatters toward them,
but this can be unlikely for a small light, or a light that is far away. If the background color is black,
then the only real sources of light in the scene are from the lights that are actually placed about the scene.
There might be two rays that intersect at nearby points on a surface,
one that is randomly reflected toward the light and one that is not.
The ray that is reflected toward the light will appear a very bright color.
The ray that is reflected to somewhere else will appear a very dark color.
The two intensities should really be somewhere in the middle.
We could lessen this problem if we steered both of these rays toward the light,
but this would cause the scene to be inaccurately bright.

For any given ray, we usually trace from the camera, through the scene, and terminate at a light.
But imagine if we traced this same ray from the light source, through the scene,
and terminated at the camera. This ray would start with a bright intensity
and would lose energy with each successive bounce around the scene.
It would ultimately arrive at the camera, having been dimmed and colored by its reflections off various surfaces.
Now, imagine if this ray was forced to bounce toward the camera as soon as it could.
It would appear inaccurately bright because it hadn't been dimmed by successive bounces.
This is analogous to sending more random samples toward the light.
It would go a long way toward solving our problem of having a bright pixel next to a dark pixel,
but it would then just make all of our pixels bright.

We can remove this inaccuracy by downweighting those samples to adjust for the over-sampling.
How do we do this adjustment? Well, we'll first need to understand the concept of a probability density function.

From later (section 3.5):

If we have a PDF for the function that we care about,
then we have the probability that the function will return a value within an arbitrary interval.
We can use this to determine where we should sample.
Remember that this started as a quest to determine
the best way to sample a scene so that we wouldn't get very bright pixels next to very dark pixels.
If we have a PDF for the scene, then we can probabilistically steer our samples toward the light
without making the image inaccurately bright.
We already said that if we steer our samples toward the light then we will make the image inaccurately bright.
We need to figure out how to steer our samples without introducing this inaccuracy,
this will be explained a little bit later, but for now we'll focus on generating samples if we have a PDF.
How do we generate a random number with a PDF?
For that we will need some more machinery. Don’t worry — this doesn’t go on forever!


My note regarding section 3.6: this is the key idea

The idea is that if we can find f(d) for all values then we have
our random number generator with density p(r)!
This is because we can say for infinitesimally small delta
that the values between f(x + delta) and f(x) are close to uniformally distributed and since the interval
is so small picking f(x) is fine (a good enough approximation).

If we cannot find f(d) for all values by pure math, we can do the binary partition approach in estimate_halfway.c
that approximates f(d) to some level we desire. The idea of using f(d) directly if we can find f(d) for all values
is basically taking the limit of this binary partition approach.

Finding f(d) by pure math means solving for the CDF (by integrating the PDF or approximating it by other means)
and then finding the inverse of the CDF on the relevant interval (f(d) is that inverse of the CDF).
The book will sometimes call the inverse of the CDF by ICD.

*/