/*
 * CS430: Project 1
 * Author: Trey Tangeman
 * Description: ppmrw is able to read, write, and convert between files in the
 * PPM P3/P6 format.
 */

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "ppmrw.h"

/*
 * Reads a file into memory and returns a struct with a pointer
 * to the memory and the file size.
 * NOTE: user is responsible for freeing the memory
 */
struct file_contents get_file_contents(FILE *fh)
{
    void *memory;
    size_t size;

    fseek(fh, 0, SEEK_END);
    size = ftell(fh);
    rewind(fh);

    memory = malloc(size);
    fread(memory, 1, size, fh);

    struct file_contents result = {memory, size, 0};
    return result;
}

/*
 * Wrapper function for advancing until a whitespace.
 */
static void read_until_whitespace(struct file_contents *fc)
{
    char *ppm_memory = (char *)fc->memory;
    while (!isspace(ppm_memory[fc->offset++]));
}

/*
 * Wrapper function for advancing through whitespace.
 */
static void read_whitespace(struct file_contents *fc)
{
    char *ppm_memory = (char *)fc->memory;
    while (isspace(ppm_memory[fc->offset])) {
        fc->offset++;
    }
}

/*
 * This function is a wrapper around the operation of advancing through all
 * comments until we hit the next line.
 */
static void read_comments(struct file_contents *fc)
{
    char *ppm_memory = (char *)fc->memory;
    while (ppm_memory[fc->offset] == '#') {
        while (ppm_memory[(fc->offset)++] != '\n');
    }
}

/*
 * This function is a wrapper around getting an ASCII value and converting
 * it to binary. The ASCII value should be delimited by whitespace!
 *
 * IMPORTANT: this function also handles the proceeding whitespaces and comments,
 * for ease of use.
 */
static int get_binary_value(struct file_contents *fc)
{
    // Get the start of the string and the end of the string
    u32 start = fc->offset;
    read_until_whitespace(fc);
    u32 end = fc->offset;

    // Allocate a string for the characters and copy the values from
    // memory. We also need to null terminate for atoi to work.
    char ascii[end - start];
    strncpy(ascii, fc->memory + start, end - start);
    ascii[end - start] = '\0';

    // Get us to the next value
    read_whitespace(fc);
    read_comments(fc);

    return atoi(ascii);
}

/*
 * Initializes a PPM pixmap struct with the values in the header:
 * format - Either P3 or P6
 * width, height - the width and height of the pixmap
 * max_color_depth - the maximum value for the RGB colors (e.g. 255)
 *
 * Returns a status code identifying the error.
 */
int init_ppm_pixmap(struct ppm_pixmap *pm, struct file_contents *fc)
{
    char *ascii_mem = (char *)fc->memory;
    char magic[3] = {ascii_mem[0], ascii_mem[1], 0};

    if (strncmp(magic, "P3", sizeof("P3")) == 0) {
        pm->format = P3_PPM;
    } else if (strncmp(magic, "P6", sizeof("P6")) == 0) {
        pm->format = P6_PPM;
    } else {
       return INVALID_FORMAT;
    }

    u32 offset = 0;

    read_until_whitespace(fc);
    read_whitespace(fc);
    read_comments(fc);

    // Parse out the width, height, and bits per channel from the header
    // NOTE: these functions account for whitespace and comments!
    s32 width = get_binary_value(fc);
    s32 height = get_binary_value(fc);
    s32 maxval = get_binary_value(fc);

    // Error checking for negative widths and heights
    // and maxval != 255
    if (width < 0) {
        return INVALID_WIDTH;
    } else if (height < 0) {
        return INVALID_HEIGHT;
    } else if (maxval != MAX_CHANNEL_VAL) {
        return INVALID_MAXVAL;
    }

    pm->width = width;
    pm->height = height;
    pm->maxval = maxval;
    pm->pixmap = malloc(sizeof(struct pixel) * (width * height));

    struct pixel *pixels = pm->pixmap;

    if (pm->format == P3_PPM) {
        // Get the binary values of the ASCII pixels
        for (int i = 0; i < (width * height); i++) {
            pixels[i].r = get_binary_value(fc);
            pixels[i].g = get_binary_value(fc);
            pixels[i].b = get_binary_value(fc);
        }
    } else {
        // Cast the binary blob into a pixel array
        memcpy(pm->pixmap, fc->memory + offset, sizeof(struct pixel) * width * height);
    }

    return INIT_SUCCESS;
}

/*
 * This function handles the error messages for the error_code
 * from the init_ppm_pixmap function.
 */
static void handle_init_error_code(int error_code)
{
    switch (error_code) {
        case INVALID_FORMAT:
            fprintf(stderr, "Error: input file has an invalid format specified in header.\n");
        break;
        case INVALID_WIDTH:
            fprintf(stderr, "Error: input file has an invalid width specified in header.\n");
        break;
        case INVALID_HEIGHT:
            fprintf(stderr, "Error: input file has an invalid height specified in header.\n");
        break;
        case INVALID_MAXVAL:
            fprintf(stderr, "Error: input file has an invalid bits per channel specified in header.\n");
        break;
    }
}

/*
 * Writes the PPM header. NOTE: It takes format as a parameter,
 * so it can be extensible to formats other than P3/P6
 */
void write_ppm_header(struct ppm_pixmap pm, FILE *fh, u32 fmt)
{
    fprintf(fh, "P%d\n%d %d\n%d\n", fmt, pm.width, pm.height, pm.maxval);
}

/*
 * Writes a P3 PPM pixmap into a file
 */
void write_p3_pixmap(struct ppm_pixmap pm, FILE *fh)
{
    u32 num_pixels = pm.width * pm.height;
    for (int i = 0; i < num_pixels; i++) {
        fprintf(fh, "%d %d %d ", pm.pixmap[i].r, pm.pixmap[i].g, pm.pixmap[i].b);
    }
}

/*
 * Writes a P6 PPM pixmap into a file
 */
void write_p6_pixmap(struct ppm_pixmap pm, FILE *fh)
{
    fwrite(pm.pixmap, 1, sizeof(struct pixel) * pm.width * pm.height, fh);
}

/*
 * ---- Entry point for the program ----
 * Usage and error checking is done in the main function, before
 * it begins processing the file.
 */
#ifdef PPMRW_MAIN
int main(int argc, char **argv)
{
    if (argc != 4) {
        fprintf(stderr, "Error: invalid number of arguments\n");
        fprintf(stderr, "Usage: %s [3|6] [input] [output]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    s32 format = atoi(argv[1]);
    char *in_fname = argv[2];
    char *out_fname = argv[3];

    if (format != P3_PPM && format != P6_PPM) {
        fprintf(stderr, "Error: invalid format \"%d\" specified!\n", format);
        fprintf(stderr, "Valid formats: 3|6\n");
        exit(EXIT_FAILURE);
    }

    FILE *input = fopen(in_fname, "r");

    if (!input) {
        fprintf(stderr, "Error: unable to open input file %s!\n", in_fname);
        exit(EXIT_FAILURE);
    }

    struct file_contents fc = get_file_contents(input);
    fclose(input);

    struct ppm_pixmap pm = {0};
    s32 status_code = init_ppm_pixmap(&pm, &fc);

    if (status_code != INIT_SUCCESS) {
        handle_init_error_code(status_code);
        exit(EXIT_FAILURE);
    } else if (pm.format == format) {
        printf("Nothing to be changed. File is already in P%d format.\n", format);

        FILE *output = fopen(out_fname, "w");
        fwrite(fc.memory, 1, fc.size, output);
        fclose(output);
    } else {
        printf("Changed file from P%d to P%d.\n", pm.format, format);
        FILE *output = fopen(out_fname, "w");

        write_ppm_header(pm, output, format);

        if (pm.format == P6_PPM) {
            write_p3_pixmap(pm, output);
        } else {
            write_p6_pixmap(pm, output);
        }

        fclose(output);
    }

    free(fc.memory);
    free(pm.pixmap);

    return 0;
}
#endif
