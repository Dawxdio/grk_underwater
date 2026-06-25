varying float vHeight;
uniform float uColR; uniform float uColG; uniform float uColB; uniform float uColA;

void main() {
	float intensity = clamp(0.5 + vHeight * 1.5, 0.0, 1.0);
	vec3 base = vec3(uColR, uColG, uColB);
	vec3 col = base * (0.6 + 0.4 * intensity);
	gl_FragColor = vec4(col, uColA);
}