#pragma once

enum object_type {
    OBJ_CAMERA,
    OBJ_SPHERE,
    OBJ_PLANE
};

struct objects {
    enum object_type type;
    union {
        struct camera camera;
        struct sphere sphere;
        struct plane plane;
    };
};

