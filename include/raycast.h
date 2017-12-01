#pragma once

#include "3dmath.h"

typedef struct color3f color3f;
struct color3f {
    double r, g, b;
};

struct light {
    color3f color;
    v3 pos;
    double rad_a0;
    double rad_a1;
    double rad_a2;

    // spotlights
    v3 direction;
    double theta;
    double ang_a0;
};

struct sphere {
    color3f color;
    color3f diffuse;
    color3f specular;
    v3 pos;
    double rad;
    float reflectivity;
    float refractivity;
    float ior;
};

struct plane {
    color3f color;
    color3f diffuse;
    color3f specular;
    v3 pos, norm;
    float reflectivity;
    float refractivity;
    float ior;
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
