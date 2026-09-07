#version 450 core

layout (location = 0) in vec3 aPos;

out vec3 UV;

uniform mat4 projection;
uniform mat4 view;

void main() {
    UV = aPos;
    vec4 pos = projection * view * vec4(aPos, 1.0);
    gl_Position = vec4(pos.xy, 0.0, pos.w);
}  
