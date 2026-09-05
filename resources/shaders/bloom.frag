#version 410 core

in vec2 UV;

out vec4 FragColor;
  
uniform sampler2D uHDRColorTex;
uniform bool uHorizontal;
uniform float uWeight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);
uniform float uBloomSpread = 1.5f;

void main() {             
    vec2 texelSize = 1.0 / textureSize(uHDRColorTex, 0); // gets size of single texel
    vec3 result = texture(uHDRColorTex, UV).rgb * uWeight[0]; // current fragment's contribution
    if(uHorizontal) {
        for(int i = 1; i < 5; ++i) {
            result += texture(uHDRColorTex, UV + vec2(texelSize.x * i * uBloomSpread, 0.0)).rgb * uWeight[i];
            result += texture(uHDRColorTex, UV - vec2(texelSize.x * i * uBloomSpread, 0.0)).rgb * uWeight[i];
        }
    } else {
        for(int i = 1; i < 5; ++i) {
            result += texture(uHDRColorTex, UV + vec2(0.0, texelSize.y * i * uBloomSpread)).rgb * uWeight[i];
            result += texture(uHDRColorTex, UV - vec2(0.0, texelSize.y * i * uBloomSpread)).rgb * uWeight[i];
        }
    }
    FragColor = vec4(result, 1.0);
}
