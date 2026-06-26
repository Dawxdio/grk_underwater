#version 330 core
in float vHeight;
in vec3 vWorldPos;

out vec4 FragColor;

uniform float uTime;
uniform float uColR; uniform float uColG; uniform float uColB; uniform float uColA;
uniform vec3 uLightDir;
uniform vec3 uViewPos;

void main() {
    float speedFactor = 0.5;
    
    float dx = (0.06 * 1.5 * cos(1.5 * vWorldPos.x + 2.0 * uTime * speedFactor)) + 
               (0.03 * 3.5 * cos(3.5 * vWorldPos.z - 2.5 * uTime * speedFactor));
               
    float dz = (0.06 * 1.5 * cos(1.5 * vWorldPos.z + 1.8 * uTime * speedFactor)) + 
               (0.03 * 3.5 * cos(3.5 * vWorldPos.x + 2.2 * uTime * speedFactor));
    
    vec3 normal = normalize(vec3(-dx, 1.0, -dz));

    vec3 L = normalize(uLightDir);
    vec3 V = normalize(uViewPos - vWorldPos);
    vec3 H = normalize(L + V);
    
    // Let the ripples catch the light so they stand out from the floor background
    float ndotl = max(dot(normal, L), 0.0);
    float spec = pow(max(dot(normal, H), 0.0), 32.0);
    vec3 specularColor = vec3(1.0, 0.95, 0.8);
    
    vec3 baseColor = vec3(uColR, uColG, uColB);
    vec3 diffuse = baseColor * ndotl * 0.5;
    vec3 ambient = baseColor * 0.25;
    
    vec3 finalCol = ambient + diffuse + (specularColor * spec * 2.0);
    
    // Looking straight down at the seabed is highly transparent (low alpha).
    // Looking out towards the horizon makes the surface more solid/reflective (high alpha).
    float fresnel = pow(1.0 - max(dot(vec3(0.0, 1.0, 0.0), V), 0.0), 3.0);
    float dynamicAlpha = mix(0.25, 0.75, fresnel); 
    
    FragColor = vec4(finalCol, dynamicAlpha);
}