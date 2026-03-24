/* Book 3: Section 3.5:

At this point you should be familiar with how to experimentally solve for the area under a curve.
We'll take our existing code and modify it slightly to get an estimate for the 50% value.
We want to solve for the x value that gives us half of the total area under the curve.
As we go along and solve for the rolling sum over N samples,
we're also going to store each individual sample alongside its p(x) value.
After we solve for the total sum, we'll sort our samples and add them up until we
have an area that is half of the total. From 0 to 2π for example:

*/

#include "rtweekend.h"

typedef struct
{
    double x;
    double p_x;
} Sample;

int compare_by_x(const void *a, const void *b)
{
    const Sample *sa = a;
    const Sample *sb = b;

    if (sa->x < sb->x)
        return -1;
    if (sa->x > sb->x)
        return 1;
    return 0;
}

int main()
{
    const unsigned int N = 10000;
    Sample samples[N];
    double sum = 0.0;

    // Iterate through all of our samples.

    for (unsigned int i = 0; i < N; i++)
    {
        // Get the area under the curve.
        double x = random_in_range(0, 2 * pi);
        double sin_x = sin(x);
        double p_x = exp(-x / (2 * pi)) * sin_x * sin_x;
        sum += p_x;

        // Store this sample.
        Sample this_sample = {x, p_x};
        samples[i] = this_sample;
    }

    // Sort the samples by x.
    qsort(samples, N, sizeof(Sample), compare_by_x);

    // Find out the sample at which we have half of our area.
    double half_sum = sum / 2.0;
    double halfway_point = 0.0;
    double accum = 0.0;
    for (unsigned int i = 0; i < N; i++)
    {
        accum += samples[i].p_x;
        if (accum >= half_sum)
        {
            halfway_point = samples[i].x;
            break;
        }
    }

    printf("Average = %.12f \n", (sum / N));
    printf("Area under curve = %.12f \n", 2 * pi * (sum / N));
    printf("Halfway = %.12f \n", halfway_point);
}

/* Why we want to know this?

We need to figure out a way to convert a uniform random number generator
into a nonuniform random number generator, where the distribution is defined by the PDF.
We'll just *assume* that there exists a function f(d) that takes uniform input and
produces a nonuniform distribution weighted by PDF. We just need to figure out a way to solve for f(d).

For the PDF given previously in the book, where p(r)=r/2 (different than the above code here),
the probability of a random sample is higher toward 2 than it is toward 0.
There is a greater probability of getting a number between 1.8 and 2.0 than between 0.0 and 0.2.
If we put aside our mathematics hat for a second and put on our computer science hat,
maybe we can figure out a smart way of partitioning the PDF.
We know that there is a higher probability near 2 than near 0,
but what is the value that splits the probability in half?
What is the value that a random number has a 50% chance of being higher than and a 50% chance of being lower than?
The answer in this case is sqrt(2).

Thus, as a crude approximation we could create a function f(d) that takes as input double d = random_double().
If d is less than (or equal to) 0.5, it produces a uniform number in [0, sqrt(2)],
if d is greater than 0.5, it produces a uniform number in [sqrt(2), 2].

double f(double d)
{
    if (d <= 0.5)
        return std::sqrt(2.0) * random_double();
    else
        return std::sqrt(2.0) + (2 - std::sqrt(2.0)) * random_double();
}
Listing 11: A crude, first-order approximation to nonuniform PDF


We have a method of solving for the halfway point that splits a PDF in half.
If we wanted to, we could use this to create a nested binary partition of the PDF, further improving our approximation
of a nonuniform PDF:
    1. Solve for halfway point of a PDF
    2. Recurse into lower half, repeat step 1
    3. Recurse into upper half, repeat step 1

Stopping at a reasonable depth, say 6–10. As you can imagine, this could be quite computationally expensive.
The computational bottleneck for the code above is probably sorting the samples.
A naive sorting algorithm can have an algorithmic complexity of O(n^2) time,
which is tremendously expensive.
Fortunately, the sorting algorithm included in the standard library is usually much closer to O(nlog(n))
time, but this can still be quite expensive, especially for millions or billions of samples.
But this will produce decent nonuniform distributions.
This divide and conquer method of producing nonuniform distributions is used somewhat commonly in practice,
although there are much more efficient means of doing so than a simple binary partition.
If you have an arbitrary function that you wish to use as the PDF for a distribution,
you might want to research the Metropolis-Hastings Algorithm
as an efficient alternative to the simple binary partition approach presented here.

*/