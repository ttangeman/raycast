#pragma once

#include "3dmath.h"

typedef struct color3f color3f;
struct color3f {
    double r, g, b;
};

struct light {
    color3f color;
    double theta;
    double a0, a1, a2;
    v3 pos;
};

struct sphere {
    color3f color;
    color3f diffuse;
    color3f specular;
    v3 pos;
    double rad;
};

struct plane {
    color3f color;
    color3f diffuse;
    color3f specular;
    v3 pos, norm;
};

struct camera {
    double width, height;
};

struct scene {
    struct light *lights;
    struct sphere *spheres;
    struct plane *planes;
    struct camera *cameras;
    u32 num_lights;
    u32 num_spheres;
    u32 num_planes;
    u32 num_cameras;
};
