#pragma once

#include <math.h>

typedef struct v3 v3;
typedef struct v3 point;

struct v3 {
    float x, y, z;
};

static inline float v3_magnitude(v3 vec)
{
    return sqrt(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z);
}

static inline void v3_normalize(v3 *result, v3 vec)
{
    float mag = v3_magnitude(vec);
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

static inline void v3_scale(v3 *result, v3 vec, float scale)
{
    result->x = vec.x * scale;
    result->y = vec.y * scale;
    result->z = vec.z * scale;
}

static inline float v3_dot(v3 a, v3 b)
{
    return (a.x*b.x + a.y*b.y + a.z*b.z);
}

static inline void v3_cross(v3 *result, v3 a, v3 b)
{
    result->x = a.y*b.z - a.z*b.y;
    result->y = a.z*b.x - a.x*b.z;
    result->z = a.x*b.y - a.y*b.x;
}
