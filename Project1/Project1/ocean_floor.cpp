#include "ocean_floor.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include <cstddef>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Struktura wierzchołka zgodna z Twoim systemem PBR
struct FloorVertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
    glm::vec3 tangent;
    glm::vec3 bitangent;
};

// Globalne zmienne dla buforów podłogi
GLuint floorVAO = 0;
GLuint floorVBO = 0;
GLuint floorEBO = 0;
int floorIndexCount = 0;

glm::vec3 compute_floor_normal(float x, float z) {
    float delta = 0.1f;
    float hL = oceanFloorHeight(x - delta, z);
    float hR = oceanFloorHeight(x + delta, z);
    float hD = oceanFloorHeight(x, z - delta);
    float hU = oceanFloorHeight(x, z + delta);

    glm::vec3 normal;
    normal.x = hL - hR;
    normal.y = 2.0f * delta;
    normal.z = hD - hU;

    return glm::normalize(normal);
}

float oceanFloorHeight(float x, float z) {
    float r = sqrtf((x * 0.8f) * (x * 0.8f) + (z * 1.2f) * (z * 1.2f));
    float lower = 10.0f;
    float height = -3.0f * cosf((r / 10) * (float)M_PI / 2.0f) - lower;

    height += 0.4f * sinf(x * 0.15f) + 0.3f * cosf(z * 0.12f);

    float dx1 = x + 12.0f;
    float dz1 = z - 8.0f;
    float pit1 = -2.5f * expf(-(dx1 * dx1 + dz1 * dz1) / 80.0f);

    float dx2 = x - 15.0f;
    float dz2 = z + 10.0f;
    float pit2 = -1.8f * expf(-(dx2 * dx2 + dz2 * dz2) / 120.0f);

    float trench = -1.2f * expf(-((x + 5.0f) * (x + 5.0f)) / 30.0f);

    height += pit1 + pit2 + trench;
    return height;
}

// Inicjalizacja podłogi - odpalamy to raz przed pętlą while
void init_ocean_floor() {
    std::vector<FloorVertex> vertices;
    std::vector<unsigned int> indices;

    const int size = 100;
    int gridRes = size * 2;

    for (int z = -size; z <= size; z++) {
        for (int x = -size; x <= size; x++) {
            float fx = (float)x;
            float fz = (float)z;

            FloorVertex v;
            v.position = glm::vec3(fx, oceanFloorHeight(fx, fz), fz);
            v.normal = compute_floor_normal(fx, fz);
            v.texCoords = glm::vec2((fx + size) / 5.0f, (fz + size) / 5.0f); // Mapowanie UV z lekkim skalowaniem
            v.tangent = glm::vec3(1.0f, 0.0f, 0.0f); // Proste tangenty na płaszczyznę XZ
            v.bitangent = glm::vec3(0.0f, 0.0f, 1.0f);

            vertices.push_back(v);
        }
    }

    // Generowanie indeksów dla kwadratów (2 trójkąty na kwadrat)
    for (int z = 0; z < gridRes; z++) {
        for (int x = 0; x < gridRes; x++) {
            int topLeft = z * (gridRes + 1) + x;
            int topRight = topLeft + 1;
            int bottomLeft = (z + 1) * (gridRes + 1) + x;
            int bottomRight = bottomLeft + 1;

            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);

            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }

    floorIndexCount = (int)indices.size();

    glGenVertexArrays(1, &floorVAO);
    glGenBuffers(1, &floorVBO);
    glGenBuffers(1, &floorEBO);

    glBindVertexArray(floorVAO);

    glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(FloorVertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, floorEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(FloorVertex), (void*)offsetof(FloorVertex, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(FloorVertex), (void*)offsetof(FloorVertex, normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(FloorVertex), (void*)offsetof(FloorVertex, texCoords));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(FloorVertex), (void*)offsetof(FloorVertex, tangent));
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(FloorVertex), (void*)offsetof(FloorVertex, bitangent));

    glBindVertexArray(0);
}

// Funkcja rysująca dla funkcji renderScene
void draw_ocean_floor() {
    if (floorVAO == 0) return;
    glBindVertexArray(floorVAO);
    glDrawElements(GL_TRIANGLES, floorIndexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}