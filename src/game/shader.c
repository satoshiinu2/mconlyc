#include "shader.h"

#include <stdbool.h>

#include "core/render.h"

// Define interpolation for BasicVertexOut
static void interpolate_BasicVertexOut(void *out, void *v1, void *v2, void *v3, Vec3 bary) {
    BasicVertexOut *o = (BasicVertexOut *)out;
    BasicVertexOut *a = (BasicVertexOut *)v1;
    BasicVertexOut *b = (BasicVertexOut *)v2;
    BasicVertexOut *c = (BasicVertexOut *)v3;

    // Interpolate Vec4 component-wise
    o->clip_pos.x = a->clip_pos.x * bary.x + b->clip_pos.x * bary.y + c->clip_pos.x * bary.z;
    o->clip_pos.y = a->clip_pos.y * bary.x + b->clip_pos.y * bary.y + c->clip_pos.y * bary.z;
    o->clip_pos.z = a->clip_pos.z * bary.x + b->clip_pos.z * bary.y + c->clip_pos.z * bary.z;
    o->clip_pos.w = a->clip_pos.w * bary.x + b->clip_pos.w * bary.y + c->clip_pos.w * bary.z;
}

static void basic_shader_vertex(vertex_in_t vertex_in, uniform_t uniforms,
                                vertex_out_t vertex_out) {
    BasicVertexIn *in = (BasicVertexIn *)vertex_in;
    BasicUniforms *u = (BasicUniforms *)uniforms;
    BasicVertexOut *out = (BasicVertexOut *)vertex_out;

    Vec3 rotated = rotate_y3(rotate_x3(in->position, u->time), u->time);
    Vec3 translated = translate_z3(rotated, u->z_offset);

    out->clip_pos = view_to_clip(translated, u->scale, u->width, u->height);
}

static Vec4 basic_shader_fragment(vertex_out_t vertex_out, float depth, uniform_t uniforms) {
    BasicVertexOut *vo = (BasicVertexOut *)vertex_out;

    // For now, just return a color based on clip position
    return (Vec4){vo->clip_pos.x, vo->clip_pos.y, vo->clip_pos.z, 1};
}

Shader create_basic_shader() {
    return CREATE_SHADER(Basic, basic_shader_vertex, basic_shader_fragment,
                         interpolate_BasicVertexOut, sizeof(BasicVertexIn), sizeof(BasicVertexOut),
                         true);
}
