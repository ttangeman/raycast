#pragma once

#include "ppmrw.h"
#include "raycast.h"

enum object_type {
    OBJ_UNKNOWN,
    OBJ_CAMERA,
    OBJ_SPHERE,
    OBJ_PLANE
};

struct object {
    enum object_type type;
    union {
        struct camera camera;
        struct sphere sphere;
        struct plane plane;
    };
};

struct scene *construct_scene(struct file_contents *csvfc);

