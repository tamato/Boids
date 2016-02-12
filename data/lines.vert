#version 430

layout(location = 0) in vec4 Position;
layout(location = 3) in vec4 Prev;
layout(location = 4) in vec4 Next;

layout(location = 0) uniform mat4 ProjectionView;

layout(location = 0) out vec3 outNormal;

#define AspectRatio 1.0;

void main() {
    // position in clip space
    vec4 current_projected = ProjectionView * Position;

    // into NDC [-1,1] or screen space
    vec2 current_screen = current_projected.xy / current_projected.w;

    // correct for aspect ratio
    current_screen.x *= AspectRatio;

    // find NDC points for the prev and next
    vec4 prev_projected = ProjectionView * vec4(Prev.xyz, 1);
    vec2 prev_screen = prev_projected.xy / prev_projected.w;
    prev_screen.x *= AspectRatio;

    vec4 next_projected = ProjectionView * vec4(Next.xyz, 1);
    vec2 next_screen = next_projected.xy / next_projected.w;
    next_screen.x *= AspectRatio;

    vec2 dir = next_screen - prev_screen; // the line we are going to find our normal from
    dir = normalize(dir);
    vec2 normal = vec2(-dir.y, dir.x);
    normal *= Prev.w; // prev and next .w hold the radius of the line
    normal /= AspectRatio;

    gl_Position = current_projected + vec4(normal, 0, 1);

    outNormal = vec3(1, 0, 0);
}
