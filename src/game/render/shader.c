#include "shader.h"

#include <stdbool.h>

#include "core/render.h"

DEFINE_INTERPOLATE(BasicVertexOut, INTERP_FIELD(clip_pos.x) INTERP_FIELD(clip_pos.y)
                                       INTERP_FIELD(clip_pos.z) INTERP_FIELD(clip_pos.w))

static void basic_shader_vertex(vertex_in_t vertex_in, uniform_t uniforms,
                                vertex_out_t vertex_out) {
    BasicVertexIn *in = (BasicVertexIn *)vertex_in;
    BasicUniforms *u = (BasicUniforms *)uniforms;
    BasicVertexOut *out = (BasicVertexOut *)vertex_out;

    float aspect = u->width / u->height;

    Vec3 rotated = vec3_rotate_y(vec3_rotate_x(in->position, u->time), u->time);
    Vec3 translated = vec3_translate_z(rotated, u->z_offset);

    Mat4 proj = mat4_perspective(u->fov, aspect, 0.1f, 100.0f);
    Vec4 pos4 = {in->position.x, in->position.y, in->position.z, 1.0f};
    out->clip_pos = mat4_mul_vec4(&proj, pos4);
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

DEFINE_INTERPOLATE(TerrainVertexOut,
                   INTERP_FIELD(clip_pos.x) INTERP_FIELD(clip_pos.y) INTERP_FIELD(clip_pos.z)
                       INTERP_FIELD(clip_pos.w) INTERP_FIELD(uv.x) INTERP_FIELD(uv.y))

static void terrain_vertex(vertex_in_t vin, uniform_t uni, vertex_out_t vout) {
    TerrainVertexIn *in = vin;
    TerrainUniforms *u = uni;
    TerrainVertexOut *out = vout;

    Vec4 pos = {in->position.x, in->position.y, in->position.z, 1.0f};
    out->clip_pos = mat4_mul_vec4(u->mvp, pos);
    out->uv = in->uv;
}

static Vec4 terrain_shader_fragment(vertex_out_t vertex_out, float depth, uniform_t uniforms) {
    TerrainVertexOut *vo = vertex_out;
    TerrainUniforms *u = uniforms;

    return texture_sample_nearest(u->texture, vo->uv.x, vo->uv.y);
}

Shader create_terrain_shader() {
    return CREATE_SHADER(Terrain, terrain_vertex, terrain_shader_fragment,
                         interpolate_TerrainVertexOut, sizeof(TerrainVertexIn),
                         sizeof(TerrainVertexOut), true);
}
