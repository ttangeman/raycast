#pragma once

#include "3dmath.h"

typedef struct color3f color3f;
struct color3f {
    double r, g, b;
};

struct light {
    color3f color;
    double theta;
    double rad_a0, rad_a1, rad_a2;
    double ang_a0, ang_a1, ang_a2;
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
