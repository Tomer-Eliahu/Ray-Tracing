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

static inline double interval_size(struct Interval *interval)
{
    return interval->max - interval->min;
}

static inline bool interval_contains(struct Interval *interval, double x)
{
    return ((interval->min <= x) && (x <= interval->max));
}

/// @brief whether min < x && x < max.
static inline bool interval_surrounds(struct Interval *interval, double x)
{
    return ((interval->min < x) && (x < interval->max));
}

#define INTERVAL_EMPTY \
    (struct Interval) { .min = infinity, .max = -infinity }

#define INTERVAL_UNIVERSE \
    (struct Interval) { .min = -infinity, .max = infinity }
