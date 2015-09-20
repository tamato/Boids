#version 430

layout(location = 0) in vec4 Position;

layout(location = 0) uniform mat4 ProjectionView;

void main() {
    gl_Position = ProjectionView * Position;
}
