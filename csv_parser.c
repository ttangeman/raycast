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
            char *r = &strsep(&line, ",")[1];
            char *g = strsep(&line, ",");
            char *btemp = strsep(&line, ",");
            char *b = strsep(&btemp, "]");

            color.r = atof(r);
            color.g = atof(g);
            color.b = atof(b);
        } else if (strlcmp(token, "position")) {
            // the brackets are omitted form x and z
            char *x = &strsep(&line, ",")[1];
            char *y = strsep(&line, ",");
            char *ztemp = strsep(&line, ",");
            char *z = strsep(&ztemp, "]");

            pos.x = atof(x);
            pos.y = atof(y);
            pos.z = atof(z);
        } else if (strlcmp (token, "normal")) {
            char *x = &strsep(&line, ",")[1];
            char *y = strsep(&line, ",");
            char *ztemp = strsep(&line, ",");
            char *z = strsep(&ztemp, "]");

            norm.x = atof(x);
            norm.y = atof(y);
            norm.z = atof(z);
        }
    }

}

static void parse_line(struct object *obj, char *line)
{
    char *type = strsep(&line, ",");
    if (strlcmp(type, "camera")) {
        init_camera_object(obj, line);
    } else if (strlcmp(type, "plane")) {
        init_plane_object(obj, line);
    }
}

struct object *get_csv_objects(struct file_contents *csvfc)
{
    u32 nobjs = get_num_objs((char *)csvfc->memory, csvfc->size);
    struct object *objs = malloc(sizeof(struct object) * nobjs);

    if (!objs) {
        return NULL;
    }

    for (int i = 0; i < nobjs; i++) {
        struct object *obj = &objs[i];
        char *line = malloc(sizeof(char) * 1024);

        get_line(csvfc, line);
        parse_line(obj, line);

        free(line);
    }

    return objs;
}
