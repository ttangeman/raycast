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
