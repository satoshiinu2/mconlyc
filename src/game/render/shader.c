#include "shader.h"

#include <stdbool.h>

#include "core/render.h"

DEFINE_INTERPOLATE(BasicVertexOut, INTERP_FIELD(clip_pos.x) INTERP_FIELD(clip_pos.y)
                                       INTERP_FIELD(clip_pos.z) INTERP_FIELD(clip_pos.w))

static Vec4 basic_shader_vertex(const BasicVertexIn *in, const BasicUniforms *u,
                                BasicVertexOut *out) {
    float aspect = u->width / u->height;

    Vec3 rotated = vec3_rotate_y(vec3_rotate_x(in->position, u->time), u->time);
    Vec3 translated = vec3_translate_z(rotated, u->z_offset);

    Mat4 proj = mat4_perspective(u->fov, aspect, 0.1f, 100.0f);
    Vec4 pos4 = {in->position.x, in->position.y, in->position.z, 1.0f};
    out->clip_pos = mat4_mul_vec4(&proj, pos4);

    return out->clip_pos;
}

static Vec4 basic_shader_fragment(const BasicVertexOut *vo, float depth, const BasicUniforms *u) {
    // For now, just return a color based on clip position
    return (Vec4){vo->clip_pos.x, vo->clip_pos.y, vo->clip_pos.z, 1};
}

#define SHADER_NAME basic
#define V_IN BasicVertexIn
#define V_OUT BasicVertexOut
#define UNIFORMS BasicUniforms
#define VS_FN basic_shader_vertex
#define FS_FN basic_shader_fragment
#define INTERP_FN interpolate_BasicVertexOut
#define USES_CLIP 1
#include "core/shader_template.h"

DEFINE_INTERPOLATE(TerrainVertexOut,
                   INTERP_FIELD(clip_pos.x) INTERP_FIELD(clip_pos.y) INTERP_FIELD(clip_pos.z)
                       INTERP_FIELD(clip_pos.w) INTERP_FIELD(uv.x) INTERP_FIELD(uv.y))

static Vec4 terrain_shader_vertex(const TerrainVertexIn *in, const TerrainUniforms *u,
                                  TerrainVertexOut *out) {
    Vec4 pos = {in->position.x, in->position.y, in->position.z, 1.0f};
    out->clip_pos = mat4_mul_vec4(u->mvp, pos);
    out->uv = in->uv;

    return out->clip_pos;
}

static Vec4 terrain_shader_fragment(const TerrainVertexOut *vo, float depth,
                                    const TerrainUniforms *u) {
    return texture_sample_nearest(u->texture, vo->uv.x, vo->uv.y);
}

#define SHADER_NAME terrain
#define V_IN TerrainVertexIn
#define V_OUT TerrainVertexOut
#define UNIFORMS TerrainUniforms
#define VS_FN terrain_shader_vertex
#define FS_FN terrain_shader_fragment
#define INTERP_FN interpolate_TerrainVertexOut
#define USES_CLIP 1
#include "core/shader_template.h"
