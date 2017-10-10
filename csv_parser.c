#include "csv_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void get_line_offset(struct file_contents *csv)
{
    char *memory = (char *)csv->memory;
    while (memory[csv->offset++]) {
        if (memory[csv->offset] == '\n') {
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

    memcpy(str, memory, (end - start));
    str[end] = '\0';
}

static u32 get_num_objs(char *memory)
{
    u32 n, i = 0;
    while (memory[i++]) {
        if (memory[i] == '\n') {
            n++;
        }
    }
    return n;
}

struct object *get_csv_objects(struct file_contents *csvfc)
{
    u32 nobjs = get_num_objs((char *)csvfc->memory);
    struct object *objs = malloc(sizeof(struct object) * nobjs);

    if (!objs) {
        return NULL;
    }

    for (int i = 0; i < nobjs; i++) {
        struct object *obj = &objs[i];
        char *line = malloc(sizeof(char) * 1024);
        char *token;

        get_line(csvfc, line);
        printf("%s\n", line);
#if 0
        while ((token = strsep(&line, ","))) {
            printf("%s\n", token);
        }
#endif
        free(line);
    }

    return objs;
}
