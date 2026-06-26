#include "rock.h"
#include "ocean_floor.h"
#include "model_loader.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <random>
#include <vector>

// --- KONSTRUKTOR ---
// Przekazujemy ścieżkę do pliku .obj, pozycję na dnie oraz skalę
Rock::Rock(const std::string& objPath, const glm::vec3& startPosition, float startScale, float startRotationY)
    : VAO(0), VBO(0), vertexCount(0), position(startPosition), scale(startScale), rotationY(startRotationY) {
    setupMesh(objPath);
}

// --- DESTRUKTOR ---
Rock::~Rock() {
    if (VAO != 0) glDeleteVertexArrays(1, &VAO);
    if (VBO != 0) glDeleteBuffers(1, &VBO);
}

// --- SETUP MESH ---
void Rock::setupMesh(const std::string& objPath) {
    std::vector<Vertex> loadedVertices;

    if (!loadOBJ(objPath.c_str(), loadedVertices)) {
        std::cerr << "Nie udalo sie zaladowac modelu kamienia z: " << objPath << std::endl;
        return;
    }

    vertexCount = static_cast<GLsizei>(loadedVertices.size());

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, loadedVertices.size() * sizeof(Vertex), loadedVertices.data(), GL_STATIC_DRAW);

    // 0: Pozycja
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, x));

    // 1: Normalne
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, nx));

    // 2: Koordynaty tekstur
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tx));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

// --- DRAW ---
void Rock::draw(unsigned int shaderProgramID, unsigned int textureID) {
    if (vertexCount == 0) return;

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    // Dodajemy obrót wokół osi Y (0.0f, 1.0f, 0.0f) przed skalowaniem
    model = glm::rotate(model, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(scale));

    int modelLoc = glGetUniformLocation(shaderProgramID, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    // Aktywacja i bindowanie tekstury albedo kamienia
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glUniform1i(glGetUniformLocation(shaderProgramID, "albedoMap"), 0);
    glUniform1i(glGetUniformLocation(shaderProgramID, "useAlbedoMap"), 1);

    // Rysowanie
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    glBindVertexArray(0);

    // Reset stanu shadera
    glUniform1i(glGetUniformLocation(shaderProgramID, "useAlbedoMap"), 0);
}

void generateRandomRocks(std::vector<Rock>& outRocks,
    float minX, float maxX,
    float minZ, float maxZ,
    int density)
{
    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_real_distribution<float> disX(minX, maxX);
    std::uniform_real_distribution<float> disZ(minZ, maxZ);
    std::uniform_real_distribution<float> disRot(0.0f, 360.0f);
    std::uniform_real_distribution<float> disScale(1.0f, 1.5f); // Dopasuj skalę do wielkości sceny
    std::uniform_int_distribution<int> disModel(0, 2);

    std::string modelPaths[] = {
        "textures/the_rock/stone_small_a.obj",
        "textures/the_rock/stone_small_b.obj",
        "textures/the_rock/stone_small_c.obj"
    };

    for (int i = 0; i < density; ++i) {
        float posX = disX(gen);
        float posZ = disZ(gen);
        float rotation = disRot(gen);
        float scale = disScale(gen);
        int modelIdx = disModel(gen);

        // Dynamiczne próbkowanie wysokości Twojego dna morskiego!
        float posY = oceanFloorHeight(posX, posZ);

        glm::vec3 randomPosition(posX, posY, posZ);
        outRocks.emplace_back(modelPaths[modelIdx], randomPosition, scale, rotation);
    }
}