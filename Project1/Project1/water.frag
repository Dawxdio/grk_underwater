varying float vHeight;
varying vec3 vWorldPos;
uniform float uTime;
uniform vec3 uLightDir;
uniform vec3 uViewPos;
uniform float uColR; uniform float uColG; uniform float uColB; uniform float uColA;

void main() {
	float dx = (0.10 * 0.04 * cos(0.04 * vWorldPos.x + 0.5 * uTime)) + 
               (0.05 * 0.08 * cos(0.08 * vWorldPos.x + 0.9 * uTime));
    float dz = (0.10 * 0.04 * cos(0.04 * vWorldPos.z + 0.5 * uTime)) + 
               (0.05 * 0.08 * cos(0.08 * vWorldPos.z + 0.9 * uTime));
    vec3 normal = normalize(vec3(-dx, 1.0, -dz));
	vec3 L = normalize(uLightDir);
    vec3 V = normalize(uViewPos - vWorldPos);
    vec3 H = normalize(L + V); // Halfway vector
    float spec = pow(max(dot(normal, H), 0.0), 16.0);
    
    vec3 base = vec3(uColR, uColG, uColB);
    vec3 ambient = base * 0.6;
    vec3 reflection = vec3(1.0) * spec * 0.8; // Sun color reflection

    vec3 col = ambient + reflection;
    gl_FragColor = vec4(col, uColA);
}