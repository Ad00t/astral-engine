#version 450 core

in vec3 UV;

out vec4 FragColor;

uniform samplerCube skybox;
uniform float uBrightness = 0.05f;

void main() {    
    vec3 color = texture(skybox, UV).rgb * uBrightness;
    FragColor = vec4(color, 1.0);
}
