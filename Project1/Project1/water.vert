#version 330 core
layout (location = 0) in vec3 aPos;

out float vHeight;
out vec3 vWorldPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float uTime;
uniform float uAmp1; uniform float uFreq1; uniform float uSpeed1;
uniform float uAmp2; uniform float uFreq2; uniform float uSpeed2;

void main() {
    vec4 pos = vec4(aPos, 1.0);
    
    // Calculate wave height
    float y = uAmp1 * sin(uFreq1 * pos.x + uSpeed1 * uTime) + 
              uAmp2 * sin(uFreq2 * pos.z + uSpeed2 * uTime);
    pos.y = y;
    vHeight = y;
    
    // Apply matrices
    vec4 worldPosition = model * pos;
    vWorldPos = worldPosition.xyz;
    gl_Position = projection * view * worldPosition;
}