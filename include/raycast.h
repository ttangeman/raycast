#pragma once

#include "3dmath.h"

struct sphere {
    struct pixel color;
    v3 pos;
    u32 rad;
};

struct plane {
    struct pixel color;
    v3 pos, norm;
};

struct camera {
    float width, height;
};
