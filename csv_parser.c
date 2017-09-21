#include "csv_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

static void get_token(struct file_contents *csv, char *buffer)
{
    u32 bufoff = 0;
    char *memory = (char *)csv->memory;

    while (memory[csv->offset] != ',') {
        buffer[bufoff++] = memory[csv->offset++];
    }
    csv->offset++;
    buffer[bufoff] = '\0';
}

static u32 get_num_objs(struct file_contents *csv)
{
    u32 nobjs = 0;
    for (int i = 0; i < csv->size; i++) {
        char c = ((char *)csv->memory)[i];
        if ( c == '\n') {
            nobjs++;
        }
    }
    return nobjs;
}

struct object *get_csv_objects(struct file_contents *csv)
{
    struct object *objects;
    u32 nobjs = get_num_objs(csv);

    if (!nobjs) {
        return NULL;
    }

    objects = malloc(sizeof(struct object) * nobjs);

    char token[32];
    get_token(csv, token);
    printf("%s\n", token);
    printf("%c\n", ((char *)csv->memory)[csv->offset]);

    return objects;
}
