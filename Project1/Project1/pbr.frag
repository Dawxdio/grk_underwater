#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in mat3 TBN;

uniform sampler2D normalMap;
uniform sampler2D albedoMap;
uniform int useAlbedoMap;
uniform vec3 albedo;
uniform vec3 lightPos;
uniform vec3 fogColor;    // Kolor wody w oddali (np. ciemnoniebieski/turkusowy)
uniform float fogDensity; // Gęstość mgły (dla wody np. 0.03 - 0.07)
uniform vec3 viewPos; 
uniform float time;

float computeCaustics(vec2 uv, float t) {
    uv *= 1.2;
    float f1 = sin(uv.x + t) + cos(uv.y + t);
    float f2 = sin(uv.x - uv.y + t * 1.5) + cos(uv.x + uv.y - t);
    float f3 = sin(uv.y * 1.5 + t * 0.8);
    float wave = (f1 + f2 + f3) / 3.0;
    float caustics = pow(max(0.0, 1.0 - abs(wave)), 3.0);
    return caustics;
}

void main() {
    vec2 uv = TexCoord;
    // Jeśli TexCoord nie został ustawiony (np. floor rysowany natychmiastowo), użyj współrzędnych świata dla UV
    if (dot(uv, uv) < 1e-6) {
        uv = FragPos.xz * 0.2; // skala tekstury; dostosuj według potrzeby
    }

    vec3 currentAlbedo;
    if (useAlbedoMap == 1) {
        currentAlbedo = texture(albedoMap, uv).rgb * 0.9;
    } else {
        currentAlbedo = albedo * 0.35;
    }

    // Próbkujemy normal mapę przy użyciu UV (może być world-space UV dla floor)
    vec3 sampled = texture(normalMap, uv).rgb;

    bool haveTBN = length(TBN[0]) > 0.001 && length(TBN[1]) > 0.001 && length(TBN[2]) > 0.001;
    vec3 norm;
    if (haveTBN) {
        vec3 n = sampled * 2.0 - 1.0;
        norm = normalize(TBN * n);
    } else {
        // Jeżeli brak TBN z vertex shadera, spróbuj obliczyć TBN per-fragment za pomocą pochodnych
        vec3 N = normalize(Normal);
        // obliczenia pochodnych UV i pozycji
        vec3 dp1 = dFdx(FragPos);
        vec3 dp2 = dFdy(FragPos);
        vec2 duv1 = dFdx(uv);
        vec2 duv2 = dFdy(uv);
        vec3 T = normalize(duv2.y * dp1 - duv1.y * dp2);
        vec3 B = normalize(-duv2.x * dp1 + duv1.x * dp2);
        mat3 localTBN = mat3(T, B, N);
        vec3 n = sampled * 2.0 - 1.0;
        norm = normalize(localTBN * n);
    }

    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);

    vec3 ambient = 0.15 * currentAlbedo;
    vec3 diffuse = diff * currentAlbedo;
    vec2 causticUV = FragPos.xz * 0.4;

    float causticEffect = computeCaustics(causticUV, time * 0.5);

    float surfaceMask = max(0.0, norm.y * 0.6 + 0.4);

    vec3 causticColor = vec3(0.5, 0.8, 0.95);
    vec3 totalCaustics = causticColor * causticEffect * surfaceMask * (diff + 0.3) * 1.2;

    vec3 finalColor = ambient + diffuse + totalCaustics;

    float distance = length(viewPos - FragPos);
    float fogFactor = exp(-fogDensity * distance);
    fogFactor = clamp(fogFactor, 0.0, 1.0);

    vec3 colorWithFog = mix(fogColor, finalColor, fogFactor);
    colorWithFog = pow(colorWithFog, vec3(1.0 / 2.2));

    FragColor = vec4(colorWithFog, 1.0);
}
