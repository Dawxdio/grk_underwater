varying float vHeight;
varying vec3 vWorldPos;
uniform float uTime;
uniform float uColR; uniform float uColG; uniform float uColB; uniform float uColA;
uniform vec3 uLightDir;
uniform vec3 uViewPos;

void main() {
    float dx = (0.10 * 0.04 * cos(0.04 * vWorldPos.x + 0.5 * uTime)) + 
               (0.05 * 0.08 * cos(0.08 * vWorldPos.x + 0.9 * uTime));
    float dz = (0.10 * 0.04 * cos(0.04 * vWorldPos.z + 0.5 * uTime)) + 
               (0.05 * 0.08 * cos(0.08 * vWorldPos.z + 0.9 * uTime));
    
    vec3 normal = normalize(vec3(-dx, 1.0, -dz));

    vec3 L = normalize(uLightDir);
    vec3 V = normalize(uViewPos - vWorldPos);
    vec3 H = normalize(L + V);
    
    float spec = pow(max(dot(normal, H), 0.0), 16.0);
    vec3 specularColor = vec3(1.0, 0.9, 0.7);
    
    vec3 baseColor = vec3(uColR, uColG, uColB);
    vec3 ambient = baseColor * 0.4;
    
    // Combine for final color
    vec3 finalCol = ambient + (specularColor * spec * 1.5);
    gl_FragColor = vec4(finalCol, uColA);
}