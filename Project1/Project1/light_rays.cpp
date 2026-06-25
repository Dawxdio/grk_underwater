#include "light_rays.h"
#include "GL/glew.h"
#include <GLFW/glfw3.h>
#include <cmath>
#include <vector>
#include <cstdlib>

struct GodRayInstance {
    float x, z;
    float radiusTop;
    float radiusBottom;
    float height;
    float speed;
    float initialAngle;
};

std::vector<GodRayInstance> activeGodRays;
bool godRaysGenerated = false;

void draw_single_ray(const GodRayInstance& ray, float time) {
    int segments = 24;
    float topY = 0.0f;
    float bottomY = topY - ray.height;
    float currentAngle = ray.initialAngle + time * ray.speed;

    glBegin(GL_QUAD_STRIP);

    for (int i = 0; i <= segments; i++) {
        float angle = ((float)i / (float)segments) * 2.0f * 3.14159f + currentAngle;
        float sinA = sinf(angle);
        float cosA = cosf(angle);

		// 1. Wierzchołek na górze (przy powierzchni wody, mniejszy promień)
        float topX = ray.x + sinA * ray.radiusTop;
        float topZ = ray.z + cosA * ray.radiusTop;
        glColor4f(0.4f, 0.7f, 0.9f, 0.02f); // Delikatne światło na wejściu w wodę
        glVertex3f(topX, topY, topZ);

		// 2. Wierzchołek na dole (przy dnie, większy promień)
        float bottomX = ray.x + sinA * ray.radiusBottom;
        float bottomZ = ray.z + cosA * ray.radiusBottom;
        glColor4f(0.1f, 0.4f, 0.6f, 0.0f); // Płynne zanikanie do zera
        glVertex3f(bottomX, bottomY, bottomZ);
    }
    glEnd();
}

// Główna funkcja generująca rozkład promieni
void generate_god_rays_in_area(float minX, float minZ, float maxX, float maxZ, float density) {
    if (godRaysGenerated) return;

    float area = (maxX - minX) * (maxZ - minZ);
    int maxRaysForArea = static_cast<int>(area * 0.003f);
    int rayCount = static_cast<int>(maxRaysForArea * density);

    if (rayCount < 1) rayCount = 1;

    srand(12345);

    for (int i = 0; i < rayCount; i++) {
        GodRayInstance ray;

        // 1. Losowanie pozycji X i Z
        ray.x = minX + (static_cast<float>(rand()) / RAND_MAX) * (maxX - minX);
        ray.z = minZ + (static_cast<float>(rand()) / RAND_MAX) * (maxZ - minZ);

        // 2. Parametry wielkościowe
        ray.radiusTop = 0.5f + (static_cast<float>(rand()) / RAND_MAX) * 1.0f;
        ray.radiusBottom = 3.0f + (static_cast<float>(rand()) / RAND_MAX) * 1.0f;  // Smuklejszy dół stożka
        ray.height = 10.0f + (static_cast<float>(rand()) / RAND_MAX) * 5.0f;       // Wysokość dostosowana pod y=0

        ray.speed = 0.05f + (static_cast<float>(rand()) / RAND_MAX) * 0.1f;
        if (rand() % 2 == 0) ray.speed *= -1.0f; // Co drugi kręci się w drugą stronę
        ray.initialAngle = (static_cast<float>(rand()) / RAND_MAX) * 6.28f;

        activeGodRays.push_back(ray);
    }

    godRaysGenerated = true;
}

void draw_god_rays() {
    generate_god_rays_in_area(-40.0f, -40.0f, 40.0f, 40.0f, 0.6f);
    float t = (float)glfwGetTime();
    // Zapamiętujemy obecny stan tekstur, kolorów i oświetlenia
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    glUseProgram(0); // Wyłączamy shader PBR na czas rysowania promieni
    glDisable(GL_LIGHTING);

    for (const auto& ray : activeGodRays) {
        draw_single_ray(ray, t);
    }

    // 4. Przywracamy stan OpenGL sprzed rysowania promieni
    glPopAttrib();
}