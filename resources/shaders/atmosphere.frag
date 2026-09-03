#version 410 core

in vec2 UV;

out vec4 FragColor;

uniform vec3 uPlanetCenterRel; // planet center relative to camera — the ONLY position uniform
uniform sampler2D uSceneDepth;
uniform mat4 uInvProj;
uniform mat4 uInvView;
uniform float uPlanetRadius;
uniform float uAtmosRadius;
uniform vec3 uSunDir;
uniform vec3 uRayleighCoeff;
uniform float uMieCoeff;
uniform float uMieG;
uniform float uRayleighScaleHeight;
uniform float uMieScaleHeight;
uniform int uNumSamples;
uniform int uNumLightSamples;

bool raySphere(vec3 ro, vec3 rd, vec3 center, float radius, out float t0, out float t1) {
    vec3 oc = ro - center;
    float b = dot(oc, rd);
    float c = dot(oc, oc) - radius*radius;
    float disc = b*b - c;
    if (disc < 0.0) return false;
    float s = sqrt(disc);
    t0 = -b - s; t1 = -b + s;
    return true;
}

void main() {
    vec2 ndc = UV * 2.0 - 1.0;
    vec4 viewPos = uInvProj * vec4(ndc, -1.0, 1.0);
    viewPos /= viewPos.w;
    vec3 rd = normalize((uInvView * vec4(viewPos.xyz, 0.0)).xyz);
    vec3 ro = vec3(0.0); // camera is the local origin, always

    float tAtmos0, tAtmos1;
    if (!raySphere(ro, rd, uPlanetCenterRel, uAtmosRadius, tAtmos0, tAtmos1) || tAtmos1 < 0.0) {
        discard;
    }
    tAtmos0 = max(tAtmos0, 0.0);

    float tPlanet0, tPlanet1;
    bool hitPlanet = raySphere(ro, rd, uPlanetCenterRel, uPlanetRadius, tPlanet0, tPlanet1);
    float rayEnd = (hitPlanet && tPlanet0 > 0.0) ? tPlanet0 : tAtmos1;

    // scene depth reprojected into camera-relative space via rotation only — no absolute positions
    float sceneDepthNDC = texture(uSceneDepth, UV).r * 2.0 - 1.0;
    if (sceneDepthNDC < 1.0 - 1e-5) {
        vec4 sceneClip = vec4(ndc, sceneDepthNDC, 1.0);
        vec4 sceneViewPos = uInvProj * sceneClip;
        sceneViewPos /= sceneViewPos.w;
        vec3 sceneRel = mat3(uInvView) * sceneViewPos.xyz;
        float sceneDist = length(sceneRel);
        rayEnd = min(rayEnd, sceneDist);
    }

    float segLen = (rayEnd - tAtmos0) / float(uNumSamples);
    if (segLen <= 0.0) discard;

    vec3 rayleighSum = vec3(0.0);
    vec3 mieSum = vec3(0.0);
    float opticalDepthR = 0.0, opticalDepthM = 0.0;

    float t = tAtmos0;
    for (int i = 0; i < uNumSamples; i++) {
        vec3 samplePos = ro + rd * (t + segLen * 0.5);
        float height = length(samplePos - uPlanetCenterRel) - uPlanetRadius;

        float hr = exp(-height / uRayleighScaleHeight) * segLen;
        float hm = exp(-height / uMieScaleHeight) * segLen;
        opticalDepthR += hr;
        opticalDepthM += hm;

        float lt0, lt1;
        if (!raySphere(samplePos, uSunDir, uPlanetCenterRel, uAtmosRadius, lt0, lt1)) {
            continue;
        }

        lt0 = max(lt0, 0.0);
        float lightSegLen = (lt1 - lt0) / float(uNumLightSamples);
        float lightOpticalDepthR = 0.0, lightOpticalDepthM = 0.0;
        bool inShadow = false;
        float lt = lt0;
        for (int j = 0; j < uNumLightSamples; j++) {
            vec3 lightSamplePos = samplePos + uSunDir * (lt + lightSegLen * 0.5);
            float lHeight = length(lightSamplePos - uPlanetCenterRel) - uPlanetRadius;
            if (lHeight < 0.0) { inShadow = true; break; }
            lightOpticalDepthR += exp(-lHeight / uRayleighScaleHeight) * lightSegLen;
            lightOpticalDepthM += exp(-lHeight / uMieScaleHeight) * lightSegLen;
            lt += lightSegLen;
        }

        if (!inShadow) {
            vec3 tau = uRayleighCoeff * (opticalDepthR + lightOpticalDepthR)
                     + uMieCoeff * 1.1 * (opticalDepthM + lightOpticalDepthM);
            vec3 attenuation = exp(-tau);
            rayleighSum += attenuation * hr;
            mieSum += attenuation * hm;
        }
        t += segLen;
    }

    float mu = dot(rd, uSunDir);
    float phaseR = 3.0 / (16.0 * 3.14159265) * (1.0 + mu * mu);
    float g2 = uMieG * uMieG;
    float phaseM = 3.0 / (8.0 * 3.14159265) * ((1.0 - g2) * (1.0 + mu * mu))
                 / ((2.0 + g2) * pow(1.0 + g2 - 2.0*uMieG*mu, 1.5));

    vec3 color = (rayleighSum * uRayleighCoeff * phaseR + mieSum * uMieCoeff * phaseM) * 10.0;

    FragColor = vec4(color, 1.0);
}
