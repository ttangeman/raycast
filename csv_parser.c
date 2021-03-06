#include "csv_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Gets the offset into the string of the next line
static void get_line_offset(struct file_contents *csv)
{
    char *memory = (char *)csv->memory;
    while (true) {
        if (memory[csv->offset++] == '\n') {
            break;
        }
    }
}

// Gets the next line dictated by a newline character.
static void get_line(struct file_contents *csv, char *str)
{
    char *memory = csv->memory;
    u32 start = csv->offset;
    get_line_offset(csv);
    u32 end = csv->offset;

    memcpy(str, &memory[start], (end - start));
    str[end] = '\0';
}

// Gets the number of objects in the file (actually just counts newlines).
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

// Handy macro for comparing a string and literal string
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

static void init_light_object(struct object *obj, char *line)
{
    color3f color;
    double theta;
    double rad_a0 = {0};
    double rad_a1 = {0};
    double rad_a2 = {0};
    double ang_a0 = {0};
    v3 direction;
    v3 pos;
    char *token;
    while((token = strsep(&line, ":")) != NULL) {
        if (strlcmp(token, "color")) {
            // the brackets are omitted from r and b
            char *rtemp = strsep(&line, ",");
            char *r = &rtemp[1];
            char *g = strsep(&line, ",");
            char *btemp = strsep(&line, ",");
            char *b = strsep(&btemp, "]");

            color.r = atof(r);
            color.g = atof(g);
            color.b = atof(b);
        } else if (strlcmp(token, "theta")) {
            char *theta_str = strsep(&line, ",");
            theta = atof(theta_str);
        } else if (strlcmp(token, "radial-a0")) {
            char *a0_str = strsep(&line, ",");
            rad_a0 = atof(a0_str);
        } else if (strlcmp(token, "radial-a1")) {
            char *a1_str = strsep(&line, ",");
            rad_a1 = atof(a1_str);
        } else if (strlcmp(token, "radial-a2")) {
            char *a2_str = strsep(&line, ",");
            rad_a2 = atof(a2_str);
        } else if (strlcmp(token, "angular-a0")) {
            char *a0_str = strsep(&line, ",");
            ang_a0 = atof(a0_str);
        } else if (strlcmp(token, "position")) {
            // the brackets are omitted from x and z
            char *xtemp = strsep(&line, ",");
            char *x = &xtemp[1];
            char *y = strsep(&line, ",");
            char *ztemp = strsep(&line, ",");
            char *z = strsep(&ztemp, "]");

            pos.x = atof(x);
            pos.y = atof(y);
            pos.z = atof(z);
        } else if (strlcmp(token, "direction")) {
            // the brackets are omitted from x and z
            char *xtemp = strsep(&line, ",");
            char *x = &xtemp[1];
            char *y = strsep(&line, ",");
            char *ztemp = strsep(&line, ",");
            char *z = strsep(&ztemp, "]");

            direction.x = atof(x);
            direction.y = atof(y);
            direction.z = atof(z);
        }
    }
    obj->type = OBJ_LIGHT;
    obj->light.color = color;
    obj->light.theta = theta;
    obj->light.rad_a0 = rad_a0;
    obj->light.rad_a1 = rad_a1;
    obj->light.rad_a2 = rad_a2;
    obj->light.ang_a0 = ang_a0;
    obj->light.direction = direction;
    obj->light.pos = pos;
}

static void init_plane_object(struct object *obj, char *line)
{
    color3f color;
    color3f diffuse;
    color3f specular;
    v3 pos, norm;
    float reflectivity;
    float refractivity;
    float ior;

    char *token;
    while((token = strsep(&line, ":")) != NULL) {
        if (strlcmp(token, "color")) {
            // the brackets are omitted from r and b
            char *rtemp = strsep(&line, ",");
            char *r = &rtemp[1];
            char *g = strsep(&line, ",");
            char *btemp = strsep(&line, ",");
            char *b = strsep(&btemp, "]");

            color.r = atof(r);
            color.g = atof(g);
            color.b = atof(b);
        } else if (strlcmp(token, "diffuse_color")) {
            // the brackets are omitted from r and b
            char *rtemp = strsep(&line, ",");
            char *r = &rtemp[1];
            char *g = strsep(&line, ",");
            char *btemp = strsep(&line, ",");
            char *b = strsep(&btemp, "]");

            diffuse.r = atof(r);
            diffuse.g = atof(g);
            diffuse.b = atof(b);
        } else if (strlcmp(token, "specular_color")) {
            // the brackets are omitted from r and b
            char *rtemp = strsep(&line, ",");
            char *r = &rtemp[1];
            char *g = strsep(&line, ",");
            char *btemp = strsep(&line, ",");
            char *b = strsep(&btemp, "]");

            specular.r = atof(r);
            specular.g = atof(g);
            specular.b = atof(b);
        } else if (strlcmp(token, "position")) {
            // the brackets are omitted from x and z
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
        } else if (strlcmp (token, "reflectivity")) {
            char *reflect = strsep(&line, ",");
            reflectivity = atof(reflect);
        } else if (strlcmp (token, "refractivity")) {
            char *refract = strsep(&line, ",");
            refractivity = atof(refract);
        } else if (strlcmp (token, "ior")) {
            char *aior = strsep(&line, ",");
            ior = atof(aior);
        }
    }
    obj->type = OBJ_PLANE;
    obj->plane.color = color;
    obj->plane.diffuse = diffuse;
    obj->plane.specular = specular;
    obj->plane.pos = pos;
    obj->plane.norm = norm;
    obj->plane.reflectivity = reflectivity;
    obj->plane.refractivity = refractivity;
    obj->plane.ior = ior;
}

static void init_sphere_object(struct object *obj, char *line)
{
    color3f color;
    color3f diffuse;
    color3f specular;
    float radius;
    float reflectivity;
    float refractivity;
    float ior;
    v3 pos;
    char *token;

    while((token = strsep(&line, ":")) != NULL) {
        if (strlcmp(token, "color")) {
            // the brackets are omitted from r and b
            char *rtemp = strsep(&line, ",");
            char *r = &rtemp[1];
            char *g = strsep(&line, ",");
            char *btemp = strsep(&line, ",");
            char *b = strsep(&btemp, "]");

            color.r = atof(r);
            color.g = atof(g);
            color.b = atof(b);
        } else if (strlcmp(token, "diffuse_color")) {
            // the brackets are omitted from r and b
            char *rtemp = strsep(&line, ",");
            char *r = &rtemp[1];
            char *g = strsep(&line, ",");
            char *btemp = strsep(&line, ",");
            char *b = strsep(&btemp, "]");

            diffuse.r = atof(r);
            diffuse.g = atof(g);
            diffuse.b = atof(b);
        } else if (strlcmp(token, "specular_color")) {
            // the brackets are omitted from r and b
            char *rtemp = strsep(&line, ",");
            char *r = &rtemp[1];
            char *g = strsep(&line, ",");
            char *btemp = strsep(&line, ",");
            char *b = strsep(&btemp, "]");

            specular.r = atof(r);
            specular.g = atof(g);
            specular.b = atof(b);
        } else if (strlcmp(token, "position")) {
            // the brackets are omitted from x and z
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
        } else if (strlcmp (token, "reflectivity")) {
            char *reflect = strsep(&line, ",");
            reflectivity = atof(reflect);
        } else if (strlcmp (token, "refractivity")) {
            char *refract = strsep(&line, ",");
            refractivity = atof(refract);
        } else if (strlcmp (token, "ior")) {
            char *aior = strsep(&line, ",");
            ior = atof(aior);
        }
    }

    obj->type = OBJ_SPHERE;
    obj->sphere.color = color;
    obj->sphere.diffuse = diffuse;
    obj->sphere.specular = specular;
    obj->sphere.pos = pos;
    obj->sphere.rad = radius;
    obj->sphere.reflectivity = reflectivity;
    obj->sphere.refractivity = refractivity;
    obj->sphere.ior = ior;
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
    } else if (strlcmp(type, "light")) {
        init_light_object(obj, line);
    }
}

// Eats all whitespace from a given string (not including newlines)
static void remove_all_spaces(char *src, char *dest)
{
    while (*src) {
        *dest = *src++;
        if (*dest != ' ') {
            dest++;
        }
    }
    *dest = 0;
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
        char temp[1024];
        char line[1024];

        get_line(csvfc, temp);
        remove_all_spaces(temp, line);
        if (line[0] != '#') {
            parse_line(obj, line);
        }
    }
    return objs;
}

static inline void error_check_objects(struct object *objects, int num_objs)
{
    for (int i = 0; i < num_objs; i++) {
        struct object *obj = &objects[i];
        switch (obj->type) {
        case OBJ_CAMERA:
            if (obj->camera.width == 0) {
                fprintf(stderr, "Error: camera initialized without a width!\n");
                free(objects);
                exit(EXIT_FAILURE);
            } else if (obj->camera.height == 0) {
                fprintf(stderr, "Error: camera initialized without a height!\n");
                free(objects);
                exit(EXIT_FAILURE);
            }
            break;
       case OBJ_LIGHT:
            if (obj->light.theta) {
                v3 dir = obj->light.direction;
                if ((int)dir.x == 0 && (int)!dir.y == 0 && (int)!dir.z == 0) {
                    fprintf(stderr, "Error: a spotlight was specified without a direction!\n");
                    free(objects);
                    exit(EXIT_FAILURE);
                }
            }
            break;
       case OBJ_UNKNOWN:
            fprintf(stderr, "Error: an object was specified without a type!\n");
            free(objects);
            exit(EXIT_FAILURE);
            break;
        }
    }
}

// Simply takes each individual object from the object array and constructs
// 3 arrays of cameras, spheres, and planes
void construct_scene(struct file_contents *csvfc, struct scene *scene)
{
    u32 nobjs = get_num_objs((char *)csvfc->memory, csvfc->size);

    struct object *objs = get_csv_objects(csvfc, nobjs);
    error_check_objects(objs, nobjs);

    struct light *lights;
    struct camera *cameras;
    struct plane *planes;
    struct sphere *spheres;
    u32 num_lights = 0;
    u32 num_cameras = 0;
    u32 num_planes = 0;
    u32 num_spheres = 0;

    for (int i = 0; i < nobjs; i++) {
        struct object *obj = &objs[i];
        if (obj->type == OBJ_LIGHT) {
            num_lights++;
        } else if (obj->type == OBJ_CAMERA) {
            num_cameras++;
        } else if (obj->type == OBJ_PLANE) {
            num_planes++;
        } else if (obj->type == OBJ_SPHERE) {
            num_spheres++;
        }
    }

    lights = malloc(sizeof(struct light) * num_lights);
    cameras = malloc(sizeof(struct camera) * num_cameras);
    planes = malloc(sizeof(struct plane) * num_planes);
    spheres = malloc(sizeof(struct sphere) * num_spheres);

    u32 light_index = 0;
    u32 camera_index = 0;
    u32 plane_index = 0;
    u32 sphere_index = 0;

    for (int i = 0; i < nobjs; i++) {
        struct object *obj = &objs[i];
        if (obj->type == OBJ_LIGHT) {
            struct light *light = &lights[light_index++];
            memcpy(light, &obj->light, sizeof(struct light));
        } else if (obj->type == OBJ_CAMERA) {
            struct camera *camera = &cameras[camera_index++];
            memcpy(camera, &obj->camera, sizeof(struct camera));
        } else if (obj->type == OBJ_PLANE) {
            struct plane *plane = &planes[plane_index++];
            memcpy(plane, &obj->plane, sizeof(struct plane));
        } else if (obj->type == OBJ_SPHERE) {
            struct sphere *sphere = &spheres[sphere_index++];
            memcpy(sphere, &obj->sphere, sizeof(struct sphere));
        }
    }

    scene->lights = lights;
    scene->spheres = spheres;
    scene->planes = planes;
    scene->cameras = cameras;
    scene->num_lights = num_lights;
    scene->num_spheres = num_spheres;
    scene->num_planes = num_planes;
    scene->num_cameras = num_cameras;

    free(objs);
}
