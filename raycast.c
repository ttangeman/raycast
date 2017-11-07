#include "ppmrw.h"
#include "raycast.h"
#include "csv_parser.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

struct intersect_data {
    double t;
    v3 point;
    v3 normal;
    color3f diffuse;
    color3f specular;
};

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
    free(scene->lights);
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

static inline double angular_attenuation(struct light *light, v3 intersection_point)
{
    if (light->theta) {
        color3f result = {0};
        v3 light_vec = {0};

        v3_sub(&light_vec, light->pos, intersection_point);
        v3_normalize(&light_vec, light_vec);

        double cosalpha = v3_dot(light->direction, light_vec);

        double alpha = acos(cosalpha);
        double theta = convert_to_rad(light->theta);

        if (alpha > theta) {
            return 0;
        } else {
            return(pow(cosalpha, light->ang_a0));
        }
    } else {
        return 1.0;
    }
}

#define SHININESS 20

static inline color3f specular_reflection(struct light *light, struct intersect_data intersect, v3 rd)
{
    color3f result = {0};
    v3 light_vec = {0};
    v3 view_vec = {0};
    v3 reflected_vec = {0};

    v3_sub(&light_vec, light->pos, intersect.point);
    v3_sub(&view_vec, intersect.point, rd);
    v3_reflection(&reflected_vec, light_vec, intersect.normal);
    v3_normalize(&light_vec, light_vec);
    v3_normalize(&view_vec, view_vec);
    v3_normalize(&reflected_vec, reflected_vec);

    double view_angle = v3_dot(view_vec, reflected_vec);
    double light_angle = v3_dot(light_vec, intersect.normal);

    if (view_angle > 0 && light_angle > 0) {
        double shininess_factor = pow(view_angle, SHININESS);
        result.r = intersect.specular.r * light->color.r * shininess_factor;
        result.g = intersect.specular.g * light->color.g * shininess_factor;
        result.b = intersect.specular.b * light->color.b * shininess_factor;
    }
    return result;
}

static inline double radial_attenuation(struct light *light, v3 intersection_point)
{
    double dist = v3_distance(light->pos, intersection_point);

    // if light is a spotlight
    if (light->theta) {
        return 1.0;
    }

    if (dist == INFINITY) {
        return 1.0;
    } else {
        double result = light->rad_a2*(dist*dist) + light->rad_a1*dist + light->rad_a0;
        return (1.0 / result);
    }
}

static inline color3f diffuse_reflection(struct light *light, struct intersect_data intersect)
{
    color3f result = {0};
    v3 light_vec = {0};

    v3_sub(&light_vec, light->pos, intersect.point);
    v3_normalize(&light_vec, light_vec);
    double costheta = v3_dot(intersect.normal, light_vec);

    if (costheta > 0) {
        result.r = (intersect.diffuse.r * light->color.r) * costheta;
        result.g = (intersect.diffuse.g * light->color.g) * costheta;
        result.b = (intersect.diffuse.b * light->color.b) * costheta;
        return result;
    } else {
        return result;
    }
}

static inline v3 get_intersection_point(v3 ro, v3 rd, double t)
{
    v3 result = {0};
    result.x = ro.x + rd.x*t;
    result.y = ro.y + rd.y*t;
    result.z = ro.z + rd.z*t;
    return result;
}

static inline v3 get_sphere_normal(v3 intersection_point, v3 sphere_center)
{
    v3 result = {0};
    v3_sub(&result, intersection_point, sphere_center);
    v3_normalize(&result, result);
    return result;
}

static inline v3 apply_epsilon(struct intersect_data intersect)
{
    v3 result = {0};
    v3 epsilon = {0};
    v3_scale(&epsilon, intersect.normal, 0.1);
    v3_add(&result, intersect.point, epsilon);
    return result;
}

static struct intersect_data ray_intersect(struct scene *scene, v3 ro, v3 rd)
{
    struct intersect_data result = {0};
    double t;
    // Check for plane intersections
    for (int plane_index = 0; plane_index < scene->num_planes; plane_index++) {
        struct plane *plane = &scene->planes[plane_index];
        t = plane_intersection_check(plane, ro, rd);
        if (t > 0) {
            result.t = t;
            result.point = get_intersection_point(ro, rd, t);
            result.normal = plane->norm;
            result.diffuse = plane->diffuse;
            result.specular = plane->specular;
        }
    }
    // Check for sphere intersections
    for (int sphere_index = 0; sphere_index < scene->num_spheres; sphere_index++) {
        struct sphere *sphere = &scene->spheres[sphere_index];
        t = sphere_intersection_check(sphere, ro, rd);
        if (t > 0) {
            result.t = t;
            result.point = get_intersection_point(ro, rd, t);
            result.normal = get_sphere_normal(result.point, sphere->pos);
            result.diffuse = sphere->diffuse;
            result.specular = sphere->specular;
        }
    }
    return result;
}

static color3f raycast(struct scene *scene, v3 ro, v3 rd)
{
    struct intersect_data intersection = ray_intersect(scene, ro, rd);

    struct intersect_data light_intersect = {0};
    v3 adjusted_intersect = {0};
    v3 light_ray = {0};

    color3f final_color = {0};
    color3f ambient = {.1, .1, .1};
    color3f diffuse_color = {0};
    color3f specular_color = {0};
    double rad_factor = 1;
    double ang_factor = 1;

    for (int light_index = 0; light_index < scene->num_lights; light_index++) {
        struct light *light = &scene->lights[light_index];

        v3_sub(&light_ray, light->pos, intersection.point);
        v3_normalize(&light_ray, light_ray);
        adjusted_intersect = apply_epsilon(intersection);
        light_intersect = ray_intersect(scene, adjusted_intersect, light_ray);

        if (light_intersect.t == 0) {
            rad_factor = radial_attenuation(light, intersection.point);
            ang_factor = angular_attenuation(light, intersection.point);
            diffuse_color = diffuse_reflection(light, intersection);
            specular_color = specular_reflection(light, intersection, rd);

            final_color.r += ang_factor * rad_factor * (diffuse_color.r + specular_color.r) + ambient.r;
            final_color.g += ang_factor * rad_factor * (diffuse_color.g + specular_color.g) + ambient.g;
            final_color.b += ang_factor * rad_factor * (diffuse_color.b + specular_color.b) + ambient.b;
        }
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
    double focal_point = -1;

    v3 center = {0, 0, focal_point};
    v3 ro = {0};
    v3 rd = {0};
    v3 p = {0};

    color3f color;

    for (int i = 0; i < image.height; i++) {
        for (int j = 0; j < image.width; j++) {
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
