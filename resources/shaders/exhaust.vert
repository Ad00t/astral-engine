#version 450 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float uExpansionRate;
uniform float uExpansionPower;
uniform int   uDiamondCount;
uniform float uNeckStrength;

out vec3 FragPos;
out vec3 Normal;
out vec2 UV;
out float FlameDist;

void main() {
    vec3 warpedPos = aPos;
    float flameDist = clamp(-aPos.z, 0.0, 1.0);

    float radiusScale = 1.0 + pow(flameDist, uExpansionPower) * uExpansionRate;

    if (uDiamondCount > 0 && uNeckStrength > 0.0) {
        float phase = flameDist * float(uDiamondCount) * 6.28318530718;
        float waist = pow(0.5 + 0.5 * cos(phase), 11.0);
        float envelope = smoothstep(0.04, 0.22, flameDist) * (1.0 - smoothstep(0.55, 0.95, flameDist));
        radiusScale -= waist * uNeckStrength * envelope;
    }

    warpedPos.xy *= max(radiusScale, 0.05);

    vec4 worldPos = model * vec4(warpedPos, 1.0);

    FragPos = worldPos.xyz;
    Normal = mat3(transpose(inverse(model))) * aNormal;
    UV = aUV;
    FlameDist = flameDist;

    gl_Position = projection * view * worldPos;
}
