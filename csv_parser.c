#include "csv_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void get_line_offset(struct file_contents *csv)
{
    char *memory = (char *)csv->memory;
    while (true) {
        if (memory[csv->offset++] == '\n') {
            break;
        }
    }
}

static void get_line(struct file_contents *csv, char *str)
{
    char *memory = csv->memory;
    u32 start = csv->offset;
    get_line_offset(csv);
    u32 end = csv->offset;

    memcpy(str, &memory[start], (end - start));
    str[end] = '\0';
}

static u32 get_num_objs(char *memory, size_t size)
{
    u32 n = 0;
    for (int i = 0; i < size; i++) {
        if (memory[i] == '\n') {
            n++;
        }
    }
    return n;
}

#define strlcmp(str, strlit) (strncmp(str, strlit, sizeof(strlit)) == 0)

static void init_camera_object(struct object *obj, char *line)
{
    float w, h;
    char *token = strsep(&line, ":");

    if (strlcmp(token, "width")) {
        char *width = strsep(&line, ",");
        w = atof(width);

        token = strsep(&line, ":");
        char *height = strsep(&line, ",");
        h = atof(height);
    } else if (strlcmp(token, "height")) {
        char *height = strsep(&line, ",");
        h = atof(height);

        token = strsep(&line, ":");
        char *width = strsep(&line, ",");
        w = atof(width);
    }

    obj->type = OBJ_CAMERA;
    obj->camera.width = w;
    obj->camera.height = h;
}

static void init_plane_object(struct object *obj, char *line)
{
    pixel color;
    v3 pos, norm;
    char *token;
    while((token = strsep(&line, ":")) != NULL) {
        if (strlcmp(token, "color")) {
            // the brackets are omitted form r and b
            char *rtemp = strsep(&line, ",");
            char *r = &rtemp[1];
            char *g = strsep(&line, ",");
            char *btemp = strsep(&line, ",");
            char *b = strsep(&btemp, "]");

            color.r = (u8)(atof(r) * 255);
            color.g = (u8)(atof(g) * 255);
            color.b = (u8)(atof(b) * 255);
        } else if (strlcmp(token, "position")) {
            // the brackets are omitted form x and z
            char *xtemp = strsep(&line, ",");
            char *x = &xtemp[1];
            char *y = strsep(&line, ",");
            char *ztemp = strsep(&line, ",");
            char *z = strsep(&ztemp, "]");

            pos.x = atof(x);
            pos.y = atof(y);
            pos.z = atof(z);
        } else if (strlcmp (token, "normal")) {
            char *xtemp = strsep(&line, ",");
            char *x = &xtemp[1];
            char *y = strsep(&line, ",");
            char *ztemp = strsep(&line, ",");
            char *z = strsep(&ztemp, "]");

            norm.x = atof(x);
            norm.y = atof(y);
            norm.z = atof(z);
        }
    }
    obj->type = OBJ_PLANE;
    obj->plane.color = color;
    obj->plane.pos = pos;
    obj->plane.norm = norm;
}

static void init_sphere_object(struct object *obj, char *line)
{
    pixel color;
    float radius;
    v3 pos;
    char *token;

    while((token = strsep(&line, ":")) != NULL) {
        if (strlcmp(token, "color")) {
            // the brackets are omitted form r and b
            char *rtemp = strsep(&line, ",");
            char *r = &rtemp[1];
            char *g = strsep(&line, ",");
            char *btemp = strsep(&line, ",");
            char *b = strsep(&btemp, "]");

            color.r = (u8)(atof(r) * 255);
            color.g = (u8)(atof(g) * 255);
            color.b = (u8)(atof(b) * 255);
        } else if (strlcmp(token, "position")) {
            // the brackets are omitted form x and z
            char *xtemp = strsep(&line, ",");
            char *x = &xtemp[1];
            char *y = strsep(&line, ",");
            char *ztemp = strsep(&line, ",");
            char *z = strsep(&ztemp, "]");

            pos.x = atof(x);
            pos.y = atof(y);
            pos.z = atof(z);
        } else if (strlcmp (token, "radius")) {
            char *arad = strsep(&line, ",");
            radius = atof(arad);
        }
    }

    obj->type = OBJ_SPHERE;
    obj->sphere.color = color;
    obj->sphere.pos = pos;
    obj->sphere.rad = radius;
}

static void parse_line(struct object *obj, char *line)
{
    char *type = strsep(&line, ",");
    if (strlcmp(type, "camera")) {
        init_camera_object(obj, line);
    } else if (strlcmp(type, "plane")) {
        init_plane_object(obj, line);
    } else if (strlcmp(type, "sphere")) {
        init_sphere_object(obj, line);
    }
}

static struct object *get_csv_objects(struct file_contents *csvfc, u32 nobjs)
{
    struct object *objs = malloc(sizeof(struct object) * nobjs);

    if (!objs) {
        return NULL;
    }

    // Iterate over the number of lines and parse each of them
    // Each line contains ONLY 1 object!
    for (int i = 0; i < nobjs; i++) {
        struct object *obj = &objs[i];
        char *line = malloc(sizeof(char) * 1024);

        get_line(csvfc, line);
        parse_line(obj, line);

        free(line);
    }
    return objs;
}

// Simply takes each individual object from the object array and constructs
// 3 arrays of cameras, spheres, and planes
void construct_scene(struct file_contents *csvfc, struct scene *scene)
{
    u32 nobjs = get_num_objs((char *)csvfc->memory, csvfc->size);

    struct object *objs = get_csv_objects(csvfc, nobjs);

    struct camera *cameras;
    struct plane *planes;
    struct sphere *spheres;
    u32 num_cameras = 0;
    u32 num_planes = 0;
    u32 num_spheres = 0;

    for (int i = 0; i < nobjs; i++) {
        struct object *obj = &objs[i];
        if (obj->type == OBJ_CAMERA) {
            num_cameras++;
        } else if (obj->type == OBJ_PLANE) {
            num_planes++;
        } else if (obj->type == OBJ_SPHERE) {
            num_spheres++;
        }
    }

    cameras = malloc(sizeof(struct camera) * num_cameras);
    planes = malloc(sizeof(struct plane) * num_planes);
    spheres = malloc(sizeof(struct sphere) * num_spheres);

    u32 camera_index = 0;
    u32 plane_index = 0;
    u32 sphere_index = 0;

    for (int i = 0; i < nobjs; i++) {
        struct object *obj = &objs[i];
        if (obj->type == OBJ_CAMERA) {
            struct camera *cam = &cameras[camera_index++];
            cam->width = obj->camera.width;
            cam->height = obj->camera.height;
        } else if (obj->type == OBJ_PLANE) {
            struct plane *plane = &planes[plane_index++];
            plane->color = obj->plane.color;
            plane->pos = obj->plane.pos;
            plane->norm = obj->plane.norm;
        } else if (obj->type == OBJ_SPHERE) {
            struct sphere *sphere = &spheres[sphere_index++];
            sphere->color = obj->sphere.color;
            sphere->pos = obj->sphere.pos;
            sphere->rad = obj->sphere.rad;
        }
    }

    scene->spheres = spheres;
    scene->planes = planes;
    scene->cameras = cameras;
    scene->num_spheres = num_spheres;
    scene->num_planes = num_planes;
    scene->num_cameras = num_cameras;

    free(objs);
}
