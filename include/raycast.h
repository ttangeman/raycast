#pragma once

#include "3dmath.h"

typedef struct color3f color3f;
struct color3f {
    double r, g, b;
};

struct sphere {
    color3f color;
    v3 pos;
    double rad;
};

struct plane {
    color3f color;
    v3 pos, norm;
};

struct camera {
    double width, height;
};

struct scene {
    struct sphere *spheres;
    struct plane *planes;
    struct camera *cameras;
    u32 num_spheres;
    u32 num_planes;
    u32 num_cameras;
};
