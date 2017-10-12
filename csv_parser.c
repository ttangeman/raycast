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

#define strlcmp(str, strlit) strncmp(str, strlit, sizeof(strlit))

static void parse_token(struct object *obj, char *line, char *token)
{
    if (strlcmp(token, "color")) {

    }
}

static void parse_line(struct object *obj, char *line)
{
    char *type = strsep(&line, ",");
    if (strlcmp(type, "camera")) {
        char *token = strsep(&line, ":");
        parse_token(obj, line, token);
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
