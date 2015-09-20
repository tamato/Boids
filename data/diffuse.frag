#version 430

layout(location = 0) out vec4 FragColor;

void main() {
    vec3 x = dFdx(gl_FragCoord.xyz);
    vec3 y = dFdy(gl_FragCoord.xyz);

    x = normalize(x * vec3(.001,.001,100));
    y = normalize(y * vec3(.001,.001,100));
    vec3 norm = abs(cross(x,y));
    FragColor = vec4(norm, 1);
}
