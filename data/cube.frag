#version 430

layout(location = 0) in vec3 Normal;
layout(location = 1) in vec2 TexCoord;

layout(location = 1) uniform vec4 Diffuse;

layout(location = 0) out vec4 FragColor;

void main() {
    vec4 color = vec4(TexCoord,0,1);
    FragColor = color + (0.000001 * Diffuse);
}
