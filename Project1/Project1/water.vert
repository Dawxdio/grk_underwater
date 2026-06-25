varying float vHeight;
uniform float uTime;
uniform float uAmp1; uniform float uFreq1; uniform float uSpeed1;
uniform float uAmp2; uniform float uFreq2; uniform float uSpeed2;

void main() {
	vec4 pos = gl_Vertex;
	float y = uAmp1 * sin(uFreq1 * pos.x + uSpeed1 * uTime) + uAmp2 * sin(uFreq2 * pos.z + uSpeed2 * uTime);
	pos.y = y;
	vHeight = y;
	gl_Position = gl_ModelViewProjectionMatrix * pos;
}