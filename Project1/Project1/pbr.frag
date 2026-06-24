#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 albedo;
uniform vec3 lightPos;
uniform vec3 fogColor;    // Kolor wody w oddali (np. ciemnoniebieski/turkusowy)
uniform float fogDensity; // Gęstość mgły (dla wody np. 0.03 - 0.07)
uniform vec3 viewPos; 
uniform float time;

float computeCaustics(vec2 uv, float t) {
    // Skalowanie współrzędnych, aby zagęścić/rozrzedzić siatkę świetlną
    uv *= 1.2; 
    
    // Nakładanie na siebie fal pod różnymi kątami i z różną prędkością czasu
    float f1 = sin(uv.x + t) + cos(uv.y + t);
    float f2 = sin(uv.x - uv.y + t * 1.5) + cos(uv.x + uv.y - t);
    float f3 = sin(uv.y * 1.5 + t * 0.8);
    
    float wave = (f1 + f2 + f3) / 3.0;
    
    // Kluczowy trik: podniesienie do wysokiej potęgi i odwrócenie 
    // tworzy ostre, jasne linie ("żyłki") zamiast łagodnych fal
    float caustics = pow(max(0.0, 1.0 - abs(wave)), 3.0);    
    return caustics;
}

void main() {
    vec3 currentAlbedo = albedo*0.35;
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    
    vec3 ambient = 0.15 * currentAlbedo; 
    vec3 diffuse = diff * currentAlbedo;
    vec2 causticUV = FragPos.xz * 0.4; 

    // 2. Prędkość ruchu (time * 1.0 lub 1.2 daje naturalny, spokojny ruch wody)
    float causticEffect = computeCaustics(causticUV, time * 0.5); 



    float surfaceMask = max(0.0, norm.y * 0.6 + 0.4); 

    // Mnożymy przez (diff + 0.3), aby bliki były mocniejsze w świetle, ale widoczne też w półcieniu
    vec3 causticColor = vec3(0.5, 0.8, 0.95); // Piękny, jasnoturkusowy kolor przefiltrowanego słońca
    vec3 totalCaustics = causticColor * causticEffect * surfaceMask * (diff + 0.3) * 1.2; 

    // Łączymy z resztą sceny
    vec3 finalColor = ambient + diffuse + totalCaustics;
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