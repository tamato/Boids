#version 430

layout(location = 0) in vec4 Position;

layout(location = 0) uniform mat4 ProjectionView;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main() {
    gl_Position = ProjectionView * Position;
}
