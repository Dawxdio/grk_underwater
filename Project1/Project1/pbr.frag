#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 albedo;
uniform vec3 lightPos;
uniform vec3 fogColor;    // Kolor wody w oddali (np. ciemnoniebieski/turkusowy)
uniform float fogDensity; // Gęstość mgły (dla wody np. 0.03 - 0.07)
uniform vec3 viewPos; 
void main() {
    vec3 currentAlbedo = albedo;
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    
    vec3 ambient = 0.15 * currentAlbedo; 
    vec3 diffuse = diff * currentAlbedo;
    vec3 finalColor = ambient + diffuse;
    
    finalColor = pow(finalColor, vec3(1.0 / 2.2));
    FragColor = vec4(finalColor, 1.0);

    float distance = length(viewPos - FragPos);
    
    // 2. Liczymy współczynnik mgły (wzór wykładniczy)
    float fogFactor = exp(-fogDensity * distance);
    fogFactor = clamp(fogFactor, 0.0, 1.0); // Upewniamy się, że jest w przedziale 0-1

    // 3. Miksujemy kolor oświetlonego obiektu z kolorem mgły
    // Jeśli obiekt jest blisko (fogFactor = 1), dominuje finalColor. W oddali dominuje fogColor.
    vec3 colorWithFog = mix(fogColor, finalColor, fogFactor);

    // 4. Korekcja gamma na samym końcu
    colorWithFog = pow(colorWithFog, vec3(1.0 / 2.2));
    
    FragColor = vec4(colorWithFog, 1.0);
}