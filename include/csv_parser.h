#pragma once

#include "ppmrw.h"
#include "raycast.h"

enum object_type {
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

struct object *get_csv_objects(struct file_contents *csv);
