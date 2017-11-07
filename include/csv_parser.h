#pragma once

#include "ppmrw.h"
#include "raycast.h"

enum object_type {
    OBJ_UNKNOWN,
    OBJ_CAMERA,
    OBJ_SPHERE,
    OBJ_PLANE,
    OBJ_LIGHT
};

struct object {
    enum object_type type;
    union {
        struct light light;
        struct camera camera;
        struct sphere sphere;
        struct plane plane;
    };
};

void construct_scene(struct file_contents *csvfc, struct scene *scene);

