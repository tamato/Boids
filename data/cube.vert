#version 430

layout(location = 0) in vec4 Position;
layout(location = 2) in vec3 Normal;
layout(location = 0) uniform mat4 ProjectionView;
layout(location = 4) uniform mat3 NormalMat;

layout(location = 0) out vec3 outNormal;

void main() {
    gl_Position = ProjectionView * Position;
    outNormal = NormalMat * Normal;
}
