#pragma once

#include "3dmath.h"

struct sphere {
    pixel color;
    v3 pos;
    float rad;
};

struct plane {
    pixel color;
    v3 pos, norm;
};

struct camera {
    float width, height;
};

struct scene {
    struct sphere *spheres;
    struct plane *planes;
    struct camera *cameras;
    u32 num_spheres;
    u32 num_planes;
    u32 num_cameras;
    u32 world_width;
    u32 world_height;
    u32 res_width;
    u32 res_height;
};
