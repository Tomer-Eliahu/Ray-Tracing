#pragma once

#include <stdbool.h>

// Util to manage real-valued intervals.

struct Interval
{
    double min;
    double max;
};

void make_empty_interval(struct Interval *interval)
{
    interval->min = infinity;
    interval->max = -infinity;
}

static inline double interval_size(const struct Interval *interval)
{
    return interval->max - interval->min;
}

static inline bool interval_contains(const struct Interval *interval, double x)
{
    return ((interval->min <= x) && (x <= interval->max));
}

/// @brief whether min < x && x < max.
static inline bool interval_surrounds(const struct Interval *interval, double x)
{
    return ((interval->min < x) && (x < interval->max));
}

/// @brief Returns x if x is in the interval.
/// If x is not in the interval, returns the closest value to x in the interval (the min or the max).
static inline double interval_clamp(const struct Interval *interval, double x)
{
    if (x < interval->min)
    {
        return interval->min;
    }

    if (x > interval->max)
    {
        return interval->max;
    }

    return x;
}

#define INTERVAL_EMPTY \
    (struct Interval) { .min = infinity, .max = -infinity }

#define INTERVAL_UNIVERSE \
    (struct Interval) { .min = -infinity, .max = infinity }
