#version 410 core

in vec3 FragPos;
in vec3 Normal;
in vec2 UV;

uniform vec3 objectColor;

out vec4 FragColor;

void main() {
    FragColor = vec4(objectColor, 1.0);
}
