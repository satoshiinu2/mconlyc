#include "min.h"

#include <float.h>
#include <math.h>
#include <stdint.h>

Vec3 project(Vec3 a) {
    float z = a.z;
    if (fabsf(z) < FLT_EPSILON) {
        return (Vec3){0, 0, z};
    }
    return (Vec3){a.x / z, a.y / z, z};
}

Vec4 view_to_clip(Vec3 a, float focal, float width, float height) {
    float z = a.z;
    return (Vec4){
        a.x * focal + z * width / 2,
        -a.y * focal + z * height / 2,
        z,
        z,
    };
}

Vec3 rotate_x3(Vec3 a, float t) {
    float c = cos(t), s = sin(t);
    return (Vec3){a.x, a.y * c - a.z * s, a.y * s + a.z * c};
}

Vec3 rotate_y3(Vec3 a, float t) {
    float c = cos(t), s = sin(t);
    return (Vec3){a.x * c + a.z * s, a.y, -a.x * s + a.z * c};
}

Vec3 rotate_z3(Vec3 a, float t) {
    float c = cos(t), s = sin(t);
    return (Vec3){a.x * c - a.y * s, a.x * s + a.y * c, a.z};
}

Vec3 translate_x3(Vec3 a, float x) {
    return (Vec3){a.x + x, a.y, a.z};
}

Vec3 translate_y3(Vec3 a, float y) {
    return (Vec3){a.x, a.y + y, a.z};
}

Vec3 translate_z3(Vec3 a, float z) {
    return (Vec3){a.x, a.y, a.z + z};
}
