#pragma once

#include <math.h>

/*
 * General math
 * ===================
 */

static inline double clamp(double value, double min, double max)
{
    if (value < min) {
        value = min;
    } else if (value > max) {
        value = max;
    }
    return value;
}

static inline double clamp01(double value)
{
    return clamp(value, 0, 1);
}

/*
 * 3D Vector math
 * ====================
 */
typedef struct v3 v3;
typedef struct v3 point;

struct v3 {
    double x, y, z;
};

static inline double v3_distance(v3 from, v3 to)
{
    double x = to.x - from.x;
    double y = to.y - from.y;
    double z = to.z - from.z;

    return sqrt(x*x + y*y + z*z);
}

static inline double v3_magnitude(v3 vec)
{
    return sqrt(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z);
}

static inline void v3_normalize(v3 *result, v3 vec)
{
    double mag = v3_magnitude(vec);
    result->x = vec.x / mag;
    result->y = vec.y / mag;
    result->z = vec.z / mag;
}

static inline void v3_add(v3 *result, v3 a, v3 b)
{
    result->x = a.x + b.x;
    result->y = a.y + b.y;
    result->z = a.z + b.z;
}

static inline void v3_sub(v3 *result, v3 a, v3 b)
{
    result->x = a.x - b.x;
    result->y = a.y - b.y;
    result->z = a.z - b.z;
}

static inline void v3_from_points(v3 *result, point a, point b)
{
    v3_sub(result, b, a);
}

static inline void v3_scale(v3 *result, v3 vec, double scale)
{
    result->x = vec.x * scale;
    result->y = vec.y * scale;
    result->z = vec.z * scale;
}

static inline double v3_dot(v3 a, v3 b)
{
    return (a.x*b.x + a.y*b.y + a.z*b.z);
}

static inline void v3_cross(v3 *result, v3 a, v3 b)
{
    result->x = a.y*b.z - a.z*b.y;
    result->y = a.z*b.x - a.x*b.z;
    result->z = a.x*b.y - a.y*b.x;
}
