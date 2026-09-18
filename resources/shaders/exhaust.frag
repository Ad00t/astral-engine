#version 450 core

in vec3 FragPos;
in vec3 Normal;
in vec2 UV;
in float FlameDist;

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BrightColor;

uniform float uThrust;
uniform vec3  uThrustDir;
uniform float uTime;
uniform vec3  uCameraPos;
uniform int   uRenderPass;
uniform int   uDiamondCount;
uniform float uStreakSpeed;
uniform float uStreakSharpness;

uniform vec3  uCoreColorCold;
uniform vec3  uCoreColorHot;
uniform vec3  uMachColor;
uniform vec3  uFringeColor;
uniform float uFringeAlphaScale;
uniform float uFringeSpread;

float hash(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453123);
}

float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    return mix(mix(hash(i), hash(i + vec2(1.0, 0.0)), f.x),
               mix(hash(i + vec2(0.0, 1.0)), hash(i + vec2(1.0, 1.0)), f.x), f.y);
}

float flowStreaks() {
    float angle = UV.x * 6.28318530718;
    float n1 = noise(vec2(angle * 5.5, FlameDist * 11.0 - uTime * uStreakSpeed * 1.3));
    float n2 = noise(vec2(angle * 11.0 + n1 * 2.5, FlameDist * 22.0 - uTime * uStreakSpeed * 1.9));
    float combined = mix(n1, n2, 0.45);
    return pow(clamp(combined, 0.0, 1.0), uStreakSharpness * 1.5);
}

void main() {
    if (uThrust <= 0.0) {
        FragColor = vec4(0.0);
        BrightColor = vec4(0.0);
        return;
    }

    vec3 N = normalize(Normal);
    vec3 V = normalize(uCameraPos - FragPos);
    float face = abs(dot(N, V));
    float rim = 1.0 - face;
    float axialView = abs(dot(V, normalize(uThrustDir)));
    rim = mix(rim, 0.25, axialView * 0.75);
    
    float streak = flowStreaks();

    float phase = FlameDist * float(uDiamondCount) * 6.28318530718;
    float diamond = pow(0.5 + 0.5 * cos(phase), 11.0) * exp(-FlameDist * 3.0);

    float nozzleFade = smoothstep(0.0, 0.015, FlameDist);
    float tipDist = smoothstep(0.4, 1.0, FlameDist);
    float shred1 = noise(vec2(UV.x * 14.0, FlameDist * 18.0 - uTime * uStreakSpeed * 1.8));
    float shred2 = noise(vec2(UV.x * 28.0 + shred1 * 1.5, FlameDist * 36.0 - uTime * uStreakSpeed * 2.6));
    float dissolve = mix(shred1, shred2, 0.5);
    float fineNoise = noise(vec2(UV.x * 42.0 + shred1 * 2.0, FlameDist * 48.0 - uTime * uStreakSpeed * 2.2));
    float edgeShred = mix(dissolve, fineNoise, 0.65);
    float edgeTurbulence = smoothstep(0.02, 0.85, FlameDist);

    float baseFade = 1.0 - smoothstep(0.65, 0.98, FlameDist);
    float tailFade = baseFade * mix(1.0, smoothstep(0.1, 0.6, dissolve), tipDist);

    if (uRenderPass == 0) {
        // Pass 0: Inner Core
        float core = 1.0 - smoothstep(0.1, 0.7, rim); 
        float flameTemp = clamp(FlameDist + (streak - 0.5) * 0.8, 0.0, 1.0);
        float colorMix = smoothstep(0.10, 0.80, flameTemp); 
        vec3 baseColor = mix(uCoreColorCold, uCoreColorHot, colorMix);
        vec3 color = baseColor * mix(0.5, 1.5, streak); 
        color += uCoreColorHot * diamond * 1.5;
        float alpha = core * mix(0.25, 0.85, streak) * nozzleFade * tailFade;
        if (alpha < 0.005f) discard;
        FragColor = vec4(color, alpha);
        float bloomMask = core * mix(0.4, 1.0, streak);
        BrightColor = vec4(color * bloomMask * 0.3, alpha);
    } else if (uRenderPass == 1) {
        // Pass 1: Mach Sheath
        float sheath = mix(0.15, 1.0, smoothstep(0.0, 0.4, rim)) * (1.0 - smoothstep(0.7, 0.95, rim));
        vec3 color = uMachColor * mix(0.2, 1.2, streak);
        float alpha = sheath * mix(0.3, 0.7, streak) * nozzleFade * tailFade;
        if (rim > 0.25 && edgeShred < smoothstep(0.28, 0.90, rim) * edgeTurbulence * 1.5) discard;
        if (alpha < 0.05f) discard;
        FragColor = vec4(color, alpha);
        BrightColor = vec4(color * 0.4, alpha);
    } else {
        // Pass 2: Outer Fringe / Ambient Fringing
        float fringe = pow(rim, uFringeSpread) * (1.0 - FlameDist * 0.2);
        vec3 color = uFringeColor * (0.4 + streak * 1.4);
        float alpha = fringe * mix(0.1, 0.8, streak) * uFringeAlphaScale * nozzleFade * tailFade;
        if (rim > 0.17 && edgeShred < smoothstep(0.20, 0.80, rim) * (edgeTurbulence * 1.5 + 0.15)) discard;
        if (alpha < 0.05f) discard;
        FragColor = vec4(color, alpha);
        BrightColor = vec4(color * 0.3, alpha);
    }
}
