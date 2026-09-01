#version 410 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 UV;

// Material Uniforms
uniform vec4 uMaterialColor;        // Solid fallback / tint color (e.g., vec4(1.0) if unneeded)
uniform sampler2D uTextureMap;      // The standard 2D texture slot
uniform bool uUseTexture;           // Toggle: true = texture, false = solid color only

// Basic Lighting Uniforms (optional but good for custom 3D models)
uniform vec3 uLightPos;
uniform vec3 uAmbientLighting;

void main() {
    // 1. Determine Base Color
    vec4 baseColor = uMaterialColor;
    
    if (uUseTexture) {
        baseColor = texture(uTextureMap, UV);
    }
    
    // 2. Apply Basic Blinn-Phong Lighting (Works across models, spheres, cubes)
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(uLightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * vec3(1.0, 1.0, 1.0); // white light
    
    vec3 finalColor = (uAmbientLighting + diffuse) * baseColor.rgb;
    
    FragColor = vec4(finalColor, baseColor.a);
}
