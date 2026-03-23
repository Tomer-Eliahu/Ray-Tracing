/* Book 3 Section 2: A Simple Monte Carlo Program

Let’s start with one of the simplest Monte Carlo programs.
If you're not familiar with Monte Carlo programs, then it'll be good to pause and catch you up.
There are two kinds of randomized algorithms: Monte Carlo and Las Vegas.
Randomized algorithms can be found everywhere in computer graphics,
so getting a decent foundation isn't a bad idea.
A randomized algorithm uses some amount of randomness in its computation.
A Las Vegas random algorithm always produces the correct result,
whereas a Monte Carlo algorithm may produce a correct result—and frequently gets it wrong!
But for especially complicated problems such as ray tracing,
we may not place as huge a priority on being perfectly exact as on getting an answer in a reasonable amount of time.
Las Vegas algorithms will eventually arrive at the correct result,
but we can't make too many guarantees on how long it will take to get there.
The classic example of a Las Vegas algorithm is the quicksort sorting algorithm.
The quicksort algorithm will always complete with a fully sorted list,
but, the time it takes to complete is random.
Another good example of a Las Vegas algorithm is the code that we use to pick a random point in a unit disk:

    inline vec3 random_in_unit_disk() {
        while (true) {
            auto p = vec3(random_double(-1,1), random_double(-1,1), 0);
            if (p.length_squared() < 1)
                return p;
        }
    }
    Listing 1: [vec3.h] A Las Vegas algorithm

This code will always eventually arrive at a random point in the unit disk,
but we can't say beforehand how long it'll take.
It may take only 1 iteration, it may take 2, 3, 4, or even longer.
Whereas, a Monte Carlo program will give a statistical estimate of an answer,
and this estimate will get more and more accurate the longer you run it.
Which means that at a certain point, we can just decide that the answer is accurate *enough* and call it quits.
This basic characteristic of simple programs producing noisy but ever-better
answers is what Monte Carlo is all about, and is especially good for applications
like graphics where great accuracy is not needed.

*/

/* The canonical example of a Monte Carlo algorithm is estimating π, so let's do that. */

#include "rtweekend.h"
/// @brief See section 2 Estimating Pi in Book 3.
int main()
{

    int inside_circle = 0;
    int inside_circle_stratified = 0;
    int sqrt_N = 1000;

    for (int i = 0; i < sqrt_N; i++)
    {
        for (int j = 0; j < sqrt_N; j++)
        {
            double x = random_in_range(-1, 1);
            double y = random_in_range(-1, 1);
            if (x * x + y * y < 1)
                inside_circle++;

            x = 2 * ((i + random_zero_to_one()) / sqrt_N) - 1;
            y = 2 * ((j + random_zero_to_one()) / sqrt_N) - 1;
            if (x * x + y * y < 1)
                inside_circle_stratified++;
        }
    }

    // Note that the area of the unit circle is Pi as well so this is also an estimate of that area.
    printf("Regular Estimate of Pi = %.12f \n", (4.0 * inside_circle) / (sqrt_N * sqrt_N));
    printf("Stratified Estimate of Pi = %.12f \n", (4.0 * inside_circle_stratified) / (sqrt_N * sqrt_N));
}
