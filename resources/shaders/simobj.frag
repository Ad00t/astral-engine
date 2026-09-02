#version 410 core
out vec4 FragColor;
in vec3 FragPos;
in vec3 Normal;
in vec2 UV;

// Material Uniforms
uniform vec4 uMaterialColor;        // Solid fallback / tint color (e.g., vec4(1.0) if unneeded)
uniform sampler2D uTextureMap;      // The standard 2D texture slot (day texture when day/night blend is on)
uniform bool uUseTexture;           // Toggle: true = texture, false = solid color only
uniform sampler2D uNightTextureMap; // Night-side texture, only sampled when uUseDayNightBlend is true
uniform bool uUseDayNightBlend;     // Toggle: true = blend uTextureMap/uNightTextureMap by sun angle

// Basic Lighting Uniforms 
uniform vec3 uSunPos;
uniform vec3 uAmbientLighting;

void main() {
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(uSunPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);

    vec4 baseColor = uMaterialColor;

    if (uUseTexture) {
        if (uUseDayNightBlend) {
            vec4 dayColor = texture(uTextureMap, UV);
            vec4 nightColor = texture(uNightTextureMap, UV);
            // raw dot product (not clamped to 0) gives a soft terminator band
            // instead of a hard day/night cutoff at exactly 90 degrees
            float dayNightMix = smoothstep(-0.1, 0.1, dot(norm, lightDir));
            baseColor = mix(nightColor, dayColor, dayNightMix);
        } else {
            baseColor = texture(uTextureMap, UV);
        }
    }
   
    // Basic Blin-Phong lighting
    vec3 diffuse = diff * vec3(1.0, 1.0, 1.0); // white light
    vec3 finalColor = (uAmbientLighting + diffuse) * baseColor.rgb;
    
    FragColor = vec4(finalColor, baseColor.a);
}
