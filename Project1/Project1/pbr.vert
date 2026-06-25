#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitangent;

out vec2 TexCoord;
out vec3 FragPos;
out mat3 TBN;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // Pozycja fragmentu w przestrzeni świata
    FragPos = vec3(model * vec4(aPos, 1.0));

    // Transfomacja normalnej z użyciem macierzy odwrotno-transponowanej (bez efektów skalowania)
    Normal = normalize(mat3(transpose(inverse(model))) * aNormal);

    // Obliczamy T i B przy użyciu macierzy model (obrót/skalowanie) - jeśli nie są dostarczone, będą zerami
    vec3 T = normalize(mat3(model) * aTangent);
    vec3 B = normalize(mat3(model) * aBitangent);

    // Użyj Normal jako trzeciej kolumny TBN (bezpośrednio w przestrzeni świata)
    TBN = mat3(T, B, normalize(Normal));

    TexCoord = aTexCoord;

    gl_Position = projection * view * vec4(FragPos, 1.0);
}
