#version 450 core

in vec3 FragPos;
in vec3 Normal;
in vec2 UV;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;  

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform bool uIsDebug;
uniform vec4 uBaseColor;
uniform sampler2D uTextureMap;
uniform sampler2D uNightTextureMap;
uniform bool uUseTexture;
uniform bool uUseDayNightBlend;
uniform vec3 uSunPos;
uniform vec3 uAmbientLighting;
uniform vec3 uNightAmbientBoost;
uniform vec3 uEmissiveLighting;
uniform bool uUseRaytracedSphere;

const float PI = 3.14159265359;

void main() {
    vec3 workingPos = FragPos;
    vec3 workingNormal = normalize(Normal);
    vec2 workingUV = UV;

    if (uUseRaytracedSphere) {
        vec3 ro = vec3(0.0);
        vec3 rd = normalize(FragPos); 
        vec3 modelPos = vec3(model[3]); 
        float radius = length(vec3(model[0])); 
        
        vec3 oc = ro - modelPos;
        float b = dot(oc, rd);
        float c = dot(oc, oc) - radius * radius;
        float h = b * b - c;
        if (h < 0.0) discard; 

        float t = -b - sqrt(h); 
        
        workingPos = ro + rd * t;
        workingNormal = normalize(workingPos - modelPos);
        vec3 localNormal = normalize(inverse(mat3(model)) * workingNormal);
        
        workingUV = vec2(
            atan(localNormal.y, localNormal.x) / (2.0 * PI) + 0.5,
            1.0 - (asin(localNormal.z) / PI + 0.5)
        );
        
        vec4 clipPos = projection * view * vec4(workingPos, 1.0);
        gl_FragDepth = clipPos.z / clipPos.w;
    } else {
        gl_FragDepth = gl_FragCoord.z;
    }

    if (uIsDebug) {
        FragColor = vec4(0.0, 1.0, 0.0, 1.0);
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    vec3 lightDir = normalize(uSunPos - workingPos);
    float sunDot = dot(workingNormal, lightDir);

    vec4 baseColor = uBaseColor;
    float dayNightMix = 1.0;

    if (uUseTexture) {
        if (uUseDayNightBlend) {
            vec4 dayColor = texture(uTextureMap, workingUV);
            vec4 nightColor = texture(uNightTextureMap, workingUV);
            dayNightMix = smoothstep(-0.1, 0.1, sunDot);
            baseColor = mix(nightColor, dayColor, dayNightMix);
        } else {
            baseColor = texture(uTextureMap, workingUV);
        }
    }

    float diffuse = max(sunDot, 0.0);

    vec3 nightBoost = vec3(0.0);
    if (uUseDayNightBlend) {
        float nightFactor = max(-sunDot, 0.0);
        nightBoost = uNightAmbientBoost * nightFactor;
    }

    vec3 light = uEmissiveLighting + uAmbientLighting + nightBoost + vec3(diffuse);
    vec3 finalColor = light * baseColor.rgb;
    FragColor = vec4(finalColor, baseColor.a);

    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        BrightColor = vec4(FragColor.rgb, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}
