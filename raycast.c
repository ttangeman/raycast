#include "ppmrw.h"
#include "raycast.h"
#include "csv_parser.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

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
void free_scene(struct scene *scene)
{
    free(scene->spheres);
    free(scene->planes);
    free(scene->cameras);
    free(scene);
}

static v3 plane_intersection_check(struct plane *plane, v3 ro, v3 rd)
{
    float t;
    v3 result;
    v3 plane_vec;
    float dist, unit_dist;

    v3_from_points(&plane_vec, (point)ro, (point)plane->pos);
    dist = v3_dot(plane->norm, plane_vec);
    unit_dist = v3_dot(plane->norm, rd);

    t = dist / unit_dist;
    result.x = ro.x + rd.x*t;
    result.y = ro.y + rd.y*t;
    result.z = ro.z + rd.z*t;

    return result;
}

/*
 * This hhe main raycasting function.
 * It evaulates a ray through each pixel and checks for intersections
 * If there is an intersection, that pixel is colored with the color of the object hit.
 */
void project_scene_on_image(struct scene *scene, struct pixmap image)
{
    // Only 1 camera is supported ATM
    struct camera camera = scene->cameras[0];

    if (!camera) {
        free(image.pixels);
        free_scene(scene);
        die("Error: no camera was defined by CSV file!");
    }

    float focal_point = 1; // meters
    float pixel_width = camera.width / image.width;
    float pixel_height = camera.height / image.height;
    v3 center = {0, 0, -focal_point}; // center of the view plane
    v3 p = {0, 0, center.z};
    v3 ro = {0};
    v3 rd = {0};

    // The reading does column major but row major is far more cache efficient
    for (int i = 0; i < image.width; i++) {
        p.x = center.x + camera.width * 0.5 + pixel_width * (i + 0.5);
        for (int j = 0; j < image.height; j++) {
            p.y = center.y + camera.height * 0.5 + pixel_height * (j + 0.5);
            // Store normalized p in Rd
            v3_normalize(&rd, p);

            // Check for plane intersections
            for (int plane_index = 0; plane_index < scene->num_planes; plane_index++) {
                struct plane *plane = &scene->planes[plane_index];
                plane_intersection_check(plane, ro, rd);
            }
            // Check for sphere intersections
            for (int sphere_index = 0; sphere_index < scene->num_spheres; sphere_index++) {
                struct sphere *sphere = &scene->spheres[sphere_index];
                sphere_intersection_check(plane, ro, rd);
            }
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

    struct file_contents fc = get_file_contents(input);
    fclose(input);

    struct scene *scene = construct_scene(&fc);
    free(fc.memory);

    struct pixmap image = {0};
    image.width = width;
    image.height = height;
    image.pixels = malloc(sizeof(pixel) * width * height);

    project_scene_on_image(scene, image);

    free(image.pixels);
    free_scene(scene);
    return 0;
}
