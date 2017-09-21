#pragma once

/*
 * Typedefs/Enums
 * ==============
 */
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;

typedef int8_t      s8;
typedef int16_t     s16;
typedef int32_t     s32;
typedef int64_t     s64;

#define MAX_CHANNEL_VAL   255

enum ppm_format {
    P1_PPM = 1,
    P2_PPM = 2,
    P3_PPM = 3,
    P4_PPM = 4,
    P5_PPM = 5,
    P6_PPM = 6,
    P7_PPM = 7
};

// These values are used for the init_ppm_pixmap function
enum status_code {
    INIT_SUCCESS,
    INVALID_FORMAT,
    INVALID_WIDTH,
    INVALID_HEIGHT,
    INVALID_MAXVAL
};

/*
 * Struct definitions
 * ==================
 */
struct pixel {
    u8 r, g, b;
};

struct pixmap {
    u32 width, height;
    struct pixel *pixmap;
};

struct ppm_pixmap {
    enum ppm_format format;     // P3 or P6 for pixmaps
    u32 width, height;
    u32 maxval;       // 0 to bits_per_channel for RGB
    struct pixel *pixmap;       // NOTE: allocated during init, so needs to be freed
};

struct file_contents {
    void *memory;
    size_t size;
    u32 offset;
};

/*
 * Function declarations
 * ====================
 */
struct file_contents get_file_contents(FILE *fh);
int init_ppm_pixmap(struct ppm_pixmap *pm, struct file_contents fc);
void write_ppm_header(struct ppm_pixmap pm, FILE *fh, u32 fmt);
void write_p3_pixmap(struct ppm_pixmap pm, FILE *fh);
void write_p6_pixmap(struct ppm_pixmap pm, FILE *fh);

