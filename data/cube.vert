#version 430

layout(location = 0) in vec4 Position;
layout(location = 2) in vec4 Normal;
layout(location = 8) in vec2 TexCoord;

layout(location = 10) uniform mat4 Model;
layout(location = 14) uniform mat4 View;
layout(location = 18) uniform mat4 Projection;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec2 outTexCoord;

void main() {
    gl_Position = Projection * View * Model * Position;
    outNormal = (View * Model * Normal).xyz;
    outTexCoord = TexCoord;
}
