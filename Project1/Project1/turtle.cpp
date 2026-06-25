#include <GL/glew.h>
#include "turtle.h"
#include "model_loader.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <cmath>

// --- KONSTRUKTOR ---
Turtle::Turtle() : isInitialized(false), lastProgress(0.0f), position(0.0f),
ptfForward(0.0f, 0.0f, -1.0f), ptfUp(0.0f, 1.0f, 0.0f), ptfRight(1.0f, 0.0f, 0.0f) {
    setupMesh();
}

// --- DESTRUKTOR ---
Turtle::~Turtle() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

// --- SETUP MESH ---
void Turtle::setupMesh() {
    std::vector<Vertex> loadedVertices;

    if (!loadOBJ("textures/uploads_files_1921252_turtle.obj", loadedVertices)) {
        std::cerr << "Nie udalo sie zaladowac modelu zolwia!" << std::endl;
        return;
    }

    vertexCount = loadedVertices.size();

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

// --- UPDATE (Logika Parallel Transport Frames) ---
void Turtle::update(float progress, SplinePath& path) {
    position = path.getPosition(progress);
    glm::vec3 newForward = glm::normalize(path.getTangent(progress));

    if (!isInitialized || progress < lastProgress) {
        ptfForward = newForward;
        glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);

        if (std::abs(glm::dot(ptfForward, worldUp)) > 0.99f) {
            worldUp = glm::vec3(1.0f, 0.0f, 0.0f);
        }
        ptfRight = glm::normalize(glm::cross(worldUp, ptfForward));
        ptfUp = glm::cross(ptfForward, ptfRight);

        isInitialized = true;
        lastProgress = progress;
        return;
    }

    if (glm::length(ptfForward - newForward) > 0.0001f) {
        glm::vec3 rotationAxis = glm::cross(ptfForward, newForward);

        if (glm::length(rotationAxis) > 0.0001f) {
            rotationAxis = glm::normalize(rotationAxis);
            float dotProduct = glm::clamp(glm::dot(ptfForward, newForward), -1.0f, 1.0f);
            float angle = std::acos(dotProduct);

            glm::mat4 rotMatrix = glm::rotate(glm::mat4(1.0f), angle, rotationAxis);

            ptfUp = glm::normalize(glm::vec3(rotMatrix * glm::vec4(ptfUp, 0.0f)));
            ptfRight = glm::normalize(glm::vec3(rotMatrix * glm::vec4(ptfRight, 0.0f)));
        }
    }

    ptfForward = newForward;
    lastProgress = progress;
}

// --- DRAW ---
void Turtle::draw(unsigned int shaderProgramID, unsigned int textureID) {
    glm::mat4 rotation = glm::mat4(1.0f);
    rotation[0] = glm::vec4(ptfRight, 0.0f);
    rotation[1] = glm::vec4(ptfUp, 0.0f);
    rotation[2] = glm::vec4(-ptfForward, 0.0f);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = model * rotation;
    model = glm::scale(model, glm::vec3(0.01f)); // Twoja dopasowana skala

    int modelLoc = glGetUniformLocation(shaderProgramID, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Łączymy jednostkę GL_TEXTURE0 ze zmienną albedoMap w shaderze
    glUniform1i(glGetUniformLocation(shaderProgramID, "albedoMap"), 0);
    // Włączamy używanie mapy albedo w shaderze
    glUniform1i(glGetUniformLocation(shaderProgramID, "useAlbedoMap"), 1);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    glBindVertexArray(0);

    glUniform1i(glGetUniformLocation(shaderProgramID, "useAlbedoMap"), 0);
}