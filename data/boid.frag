#version 430

layout(location = 0) in vec2 Corner;
layout(location = 0) out vec4 FragColor;

void main() {
    float len = 1. - length(Corner);
    len = max(len, 0.);
    FragColor = vec4(len);
}
