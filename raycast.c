#include "ppmrw.h"
#include "raycast.h"
#include "csv_parser.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

// Useful message and quit function
static void die(const char *reason, ...)
{
    va_list args;
    va_start(args, reason);
    vfprintf(stderr, reason, args);
    fprintf(stderr, "\n");
    va_end(args);
    exit(EXIT_FAILURE);
}

// Clean-up
static inline void free_scene(struct scene *scene)
{
    free(scene->spheres);
    free(scene->planes);
    free(scene->cameras);
    free(scene);
}

// Checks the spheres for intersection
static double sphere_intersection_check(struct sphere *sphere, v3 ro, v3 rd)
{
    // This vector represents the vector from the origin to the sphere
    v3 sphere_vec;
    v3_sub(&sphere_vec, ro, sphere->pos);

    double b = 2 * (rd.x * sphere_vec.x + rd.y * sphere_vec.y + rd.z * sphere_vec.z);
    double c = sphere_vec.x*sphere_vec.x + sphere_vec.y*sphere_vec.y +
               sphere_vec.z*sphere_vec.z - sphere->rad*sphere->rad;

    double disc = b*b - 4*c;

    if (disc < 0) {
        return -1;
    }

    double sqrt_disc = sqrt(disc);
    // neither t0 or t1 can be < 0
    // also t0 will always be the closest point because it does the "-"
    double t0 = (-b - sqrt_disc) / 2;
    double t1 = (-b + sqrt_disc) / 2;

    if (t0 < 0) {
        if(t1 < 0) {
            return -1;
        } else {
            return t1;
        }
    } else {
        return t0;
    }
}

// Checks planes for intersection
static double plane_intersection_check(struct plane *plane, v3 ro, v3 rd)
{
    v3 norm = plane->norm;
    v3 pos = plane->pos;
    // This vector represents the vector from the origin to the plane
    v3 plane_vec;

    v3_sub(&plane_vec, pos, ro);
    double vo = v3_dot(plane_vec, norm);
    double vd = v3_dot(norm, rd);

    if (vd > 0) {
        return -1;
    }

    // do I need to negate here? doesn't seem like it...
    double t = vo / vd;

    if (t < 0) {
        return -1;
    }

    return t;
}

static color3f ray_intersect(struct scene *scene, v3 ro, v3 rd)
{
    double t;
    color3f closest_color = {0};
    // Check for plane intersections
    for (int plane_index = 0; plane_index < scene->num_planes; plane_index++) {
        struct plane *plane = &scene->planes[plane_index];
        t = plane_intersection_check(plane, ro, rd);
        if (t > 0) {
            closest_color = plane->color;
        }
    }
    // Check for sphere intersections
    for (int sphere_index = 0; sphere_index < scene->num_spheres; sphere_index++) {
        struct sphere *sphere = &scene->spheres[sphere_index];
        t = sphere_intersection_check(sphere, ro, rd);
        if (t > 0) {
            closest_color = sphere->color;
        }
    }
    return closest_color;
}

static color3f raycast(struct scene *scene, v3 ro, v3 rd)
{
    color3f final_color = ray_intersect(scene, ro, rd);

    for (int light_index = 0; light_index < scene->num_lights; light_index++) {
        struct light *light = &scene->lights[light_index];
    }

    return final_color;
}

// Popualtes a pixmap with the pixel colors it found via intersecton tests
void render_scene(struct scene *scene, struct pixmap image)
{
    if (!scene->cameras) {
        free(image.pixels);
        free_scene(scene);
        die("Error: no camera was defined by the CSV file!");
    }
    // Only 1 camera is supported ATM
    struct camera camera = scene->cameras[0];
    double pixel_width = camera.width / image.width;
    double pixel_height = camera.height / image.height;
    double focal_point = 1;

    v3 center = {0, 0, focal_point};
    v3 ro = {0};
    v3 rd = {0};
    v3 p = {0};

    color3f color;

    for (int i = 0; i < image.width; i++) {
        for (int j = 0; j < image.height; j++) {
            p.x = center.x - camera.width*0.5 + pixel_width * (j + 0.5);
            // Make the +Y axis be "up" by negating it
            p.y = -(center.y - camera.height*0.5 + pixel_height * (i + 0.5));
            p.z = center.z;
            v3_normalize(&rd, p);

            color = raycast(scene, ro, rd);

            image.pixels[i * image.width + j].r = 255 * clamp01(color.r);
            image.pixels[i * image.width + j].g = 255 * clamp01(color.g);
            image.pixels[i * image.width + j].b = 255 * clamp01(color.b);
        }
    }
}

int main(int argc, char **argv)
{
    if (argc != 5) {
        die("Usage:\t%s [width] [height] [input] [output]", argv[0]);
    }

    FILE *input, *output;
    s32 width = atoi(argv[1]);
    s32 height = atoi(argv[2]);
    char *infn = argv[3];
    char *outfn = argv[4];

    if (width < 0 || height < 0) {
        die("Error: invalid dimensions for output image (%d %d)!", width, height);
    }

    input = fopen(infn, "r");
    if (!input) {
        die("Error: failed to open input file (%s)!", infn);
    }

    // Reads the entire file into memory
    struct file_contents fc = get_file_contents(input);
    fclose(input);

    // Get a scene with arrays of spheres and planes to render
    struct scene *scene = malloc(sizeof(struct scene));
    construct_scene(&fc, scene);
    free(fc.memory);

    struct pixmap image = {0};
    image.width = width;
    image.height = height;
    image.pixels = malloc(sizeof(pixel) * width * height);

    // This gets us a pixmap populated with all the colored pixels
    render_scene(scene, image);

    // I should create a function for this in ppmrw...
    struct ppm_pixmap pm = {0};
    pm.format = P6_PPM;
    pm.width = image.width;
    pm.height = image.height;
    pm.maxval = 255;
    pm.pixmap = image.pixels;

    // Write the P6 PPM pixmap
    output = fopen(outfn, "w");
    write_ppm_header(pm, output, pm.format);
    write_p6_pixmap(pm, output);

    // Clean up
    fclose(output);
    free(image.pixels);
    free_scene(scene);
    return 0;
}
