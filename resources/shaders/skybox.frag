#version 410 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

void main() {    
    vec3 color = texture(skybox, TexCoords).rgb;
    color = max(color - vec3(0.05), vec3(0.0));
    FragColor = vec4(color, 1.0);
}
