#version 430
#extension GL_EXT_gpu_shader4: enable
#extension GL_EXT_geometry_shader4: enable

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in gl_PerVertex
{
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
} gl_in[];

out gl_PerVertex
{
    vec4 gl_Position;
};

layout(location = 1) uniform float Radius;

layout(location = 0) out vec2 Corner;

void makeVert(in vec2 xy){
    gl_Position = vec4(gl_in[0].gl_Position + vec4(xy, 0, 0) * Radius);
    Corner = xy;
    EmitVertex();
}

void main() {

    /**
        Generate the quad:
        1 *---* 3    
          | / |
        2 *---* 4
    **/
    makeVert(vec2(-1, 1));
    makeVert(vec2(-1,-1));
    makeVert(vec2( 1, 1));
    makeVert(vec2( 1,-1));
    EndPrimitive();
}
