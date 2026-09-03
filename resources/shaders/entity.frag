#version 410 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 UV;

uniform vec4 uBaseColor;
uniform sampler2D uTextureMap;
uniform sampler2D uNightTextureMap;
uniform bool uUseTexture;
uniform bool uUseDayNightBlend;
uniform vec3 uSunPos;
uniform vec3 uAmbientLighting;
uniform vec3 uNightAmbientBoost;
uniform vec3 uEmissiveLighting;

void main() {
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(uSunPos - FragPos);
    float sunDot = dot(norm, lightDir);

    vec4 baseColor = uBaseColor;
    float dayNightMix = 1.0;

    if (uUseTexture) {
        if (uUseDayNightBlend) {
            vec4 dayColor = texture(uTextureMap, UV);
            vec4 nightColor = texture(uNightTextureMap, UV);
            dayNightMix = smoothstep(-0.1, 0.1, sunDot);
            baseColor = mix(nightColor, dayColor, dayNightMix);
        } else {
            baseColor = texture(uTextureMap, UV);
        }
    }

    float diffuse = max(sunDot, 0.0);

    // only apply the artificial night-side boost for objects that actually
    // opted into day/night texturing (earth) -- plain objects (moon, sun)
    // just get normal Lambertian shading that goes properly dark at night.
    vec3 nightBoost = vec3(0.0);
    if (uUseDayNightBlend) {
        float nightFactor = max(-sunDot, 0.0);
        nightBoost = uNightAmbientBoost * nightFactor;
    }

    vec3 light = uEmissiveLighting + uAmbientLighting + nightBoost + vec3(diffuse);
    vec3 finalColor = light * baseColor.rgb;
    FragColor = vec4(finalColor, baseColor.a);
}
