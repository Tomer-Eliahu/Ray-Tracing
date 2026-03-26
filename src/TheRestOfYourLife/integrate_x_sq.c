
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
area(f(x),a,b)=(b-a)*average(f(x),a,b) // Note that by pushing (b-a) into the average we can think of this
// as averaging areas.
// That is area(f(x),a,b) = some average of proposed areas by sampling f(x) * something

( Later we see this something = (1/PDF used for sampling).
The (b-a) comes from the specific case of using a unifrom PDF).

*/

#include "rtweekend.h"

/*The code in its section 3.7 last before final version:
We can try to solve for the integral using the *linear* PDF, p(r)=r/2,
for which we were able to solve for the CDF and its inverse, ICD.
To do that, all we need to do is replace the functions ICD(d)=sqrt(4d) and p(x)=x/2.

So now we can handel non-uniform PDFs.
We are still calculating the area(f(x),a,b) (i.e. the integral) but are now using a weighted average approach
(to calculate the *area* directly by averaging proposed areas) by
sampling some regions of f(x) over [a,b] more frequently but adjusting the weights accordingly.

CRITICAL: The Key Idea:
    Intuitively, we are taking smallers silvers of the area "under" f(x) in some areas
    (where our PDF tells us to sample more, i.e. where our PDF is high)
    and bigger silvers of the area "under" f(x) in other areas.
    By averaging all these silvers, we get the actual area under f(x) (this is an approximation: the more we sample
    this way, the closer we get to the actual exact result).
    I wrote "under" because those silvers are not really under f(x) but rather proposed areas
    (since we average them by dividing by N these silvers must sum to more than the area under f(x)).

    I think these proposed areas are proportional to the actual real silvers (not sure though).


double icd(double d)
{
    return sqrt(4.0 * d);
}

double pdf(double x)
{
    return x / 2.0;
}

int main()
{
    int N = 1000000;
    double sum = 0.0;

    for (int i = 0; i < N; i++)
    {
        double z = random_zero_to_one();
        if (z == 0.0) // Ignore zero to avoid NaNs
            continue;

        double x = icd(z);
        sum += x * x / pdf(x); // See note from section 3.7 below on why we do this
    }

    printf("I = %.12f \n", (sum / N));
}

*/

/*Final version:
If you compared the runs from the uniform PDF and the linear PDF,
you would have probably found that the linear PDF converged faster.
If you think about it, a linear PDF is probably a better approximation
for a quadratic function than a uniform PDF, so you would expect it to converge faster.
My note: this makes sense as the linear PDF samples the areas the function area changes most
(highest variance regions), more often.

If that's the case, then we should just try to make the PDF match the integrand
by turning the PDF into a quadratic function p(r) = C*(r^2) for r in [0,2] and 0 elsewhere.
We can solve for C by knowing the integral of p(r) on the real line must be = 1.

Now for JUST ONE SAMPLE we always get the exact right answer (because z gets canceled out).
Why? because if the PDF is just the function * some constant C, and since our goal is to integrate the function,
when we successfully integrated the PDF, we acheived our goal. That is we found the relevant
area under the curve and made C = 1/area.
Then the sum is the function/pdf = function/ (C*function) = 1/C = area.

*/

double icd(double d)
{
    return 8.0 * pow(d, 1.0 / 3.0);
}

double pdf(double x)
{
    return (3.0 / 8.0) * x * x;
}

int main()
{
    int N = 1;
    double sum = 0.0;

    for (int i = 0; i < N; i++)
    {
        double z = random_zero_to_one();
        if (z == 0.0) // Ignore zero to avoid NaNs
            continue;

        double x = icd(z);
        sum += x * x / pdf(x); // See note from section 3.7 below on why we do this
    }

    printf("I = %.12f \n", (sum / N));
}

/*
You could rightly point to this example (talking about an older version of the code)
and say that the integration is actually a lot less work than the Monte Carlo.
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


From Section 3.7: Importance Sampling

We need to account for the nonuniformity of the PDF of x.
Failing to account for this nonuniformity will introduce bias in our scene.
Indeed, this bias is the source of our inaccurately bright image.
Accounting for the nonuniformity will yield accurate results.
The PDF will “steer” samples toward specific parts of the distribution,
which will cause us to converge *faster*, but at the cost of introducing bias.
To remove this bias, we need to down-weight where we sample more frequently,
and to up-weight where we sample less frequently. For our new nonuniform random number generator,
the PDF defines how much or how little we sample a specific portion.
So the weighting function should be proportional to 1/pdf.
In fact it is *exactly* 1/pdf (we can deduce this from the specific case of the uniform distribution).
This is why we divide x*x by pdf(x).

*/

/* Chapter 3 final notes

CRITICAL: Key Insight:
    area(f(x),a,b) = some average of proposed areas by sampling f(x) * (1/PDF used for sampling).
    (we saw that this is true for a few situations and appears to be true in general,
    but we did not see a proof for why it is true in general beyond some intuition we developed).
    Later I googled it, and the proof of why it works and why when p(x) is close to f(x) we get
    faster convergence are really easy actually. To see it, just google "The Monte Carlo estimator".
    The faster convergence (here really means getting close enough to the actual value in less samples)
    comes from the lower standard error (due to lower variance) of The Monte Carlo estimator when we pick good p(x).

A nonuniform PDF “steers” more samples to where the PDF is big,
and fewer samples to where the PDF is small.
By this sampling, we would expect less noise in the places where the PDF is big and more noise where the PDF is small.
If we choose a PDF that is higher in the parts of the scene that have higher noise,
and is smaller in the parts of the scene that have lower noise,
we'll be able to reduce the total noise of the scene with *fewer* samples.
This means that we will be able to converge to the correct scene **faster** than with a uniform PDF.
In effect, we are steering our samples toward the parts of the distribution that are more *important*.
This is why using a carefully chosen nonuniform PDF is usually called *importance sampling*.

Note that for *any* PDF you choose to use (even a uniform one), you will eventually converage to the right answer.
But the closer the PDF is to the actual function, the *faster* the converagence will be.
This should make sense, as that means choosing to sample the important parts of the distribution more often.

The perfect importance sampling (PDF = target function) is only possible when we already know the answer
(we got the PDF by integrating the target function analytically),
but it’s a good exercise to make sure our code works.

Let's review the main concepts that underlie Monte Carlo ray tracers:

    1.  You have an integral of f(x) over some domain [a,b] you want to know.
    2.  You pick a PDF p that is non-zero and non-negative over [a,b]. The non-zero requirement comes from 3.
    3.  You average a whole ton of f(r)/p(r) where r is a random number with PDF p.
        This gives you the answer (approx.).

    *Any* choice of PDF p will result in *always* converging to the right answer,
    but the closer that p approximates f, the faster it will converge.

*/