#version 410 core

in vec2 UV;

out vec4 FragColor;

uniform sampler2D uHDRBuffer;
uniform float uExposure;

vec3 ACESFilm(vec3 x) {
    float a = 2.51, b = 0.03, c = 2.43, d = 0.59, e = 0.14;
    return clamp((x*(a*x+b))/(x*(c*x+d)+e), 0.0, 1.0);
}

void main() {
    vec3 hdrColor = texture(uHDRBuffer, UV).rgb * uExposure;
    vec3 mapped = ACESFilm(hdrColor);
    mapped = pow(mapped, vec3(1.0/2.2)); // gamma correct
    FragColor = vec4(mapped, 1.0);
}
