#include "fish.h"
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

Fish::Fish() {
    setupMesh();
}

Fish::~Fish() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void Fish::setupMesh() {
    std::vector<float> vertices = {
        // Ciało
         0.0f,  0.2f,  0.0f,    0.0f, -0.2f,  0.0f,    0.3f,  0.0f, -0.5f, // Right side front
         0.0f,  0.2f,  0.0f,    0.0f, -0.2f,  0.0f,   -0.3f,  0.0f, -0.5f, // Left side front
         0.3f,  0.0f, -0.5f,   -0.3f,  0.0f, -0.5f,    0.0f,  0.2f,  0.0f, // Top nose lid
         0.3f,  0.0f, -0.5f,   -0.3f,  0.0f, -0.5f,    0.0f, -0.2f,  0.0f, // Bottom nose lid

         // Ogon
          0.0f,  0.0f,  0.0f,    0.0f,  0.3f,  0.4f,    0.0f, -0.3f,  0.4f
    };

    vertexCount = vertices.size() / 3;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Fish::draw(const glm::vec3& position, const glm::vec3& direction, unsigned int shaderProgramID) {
    glm::vec3 forward = direction;
    glm::vec3 upTarget = glm::vec3(0.0f, 1.0f, 0.0f);

    // Zapobieganie błędom jeśli ryba płynie prosto w górę
    if (std::abs(glm::dot(forward, upTarget)) > 0.99f) {
        upTarget = glm::vec3(1.0f, 0.0f, 0.0f);
    }

    glm::vec3 right = glm::normalize(glm::cross(upTarget, forward));
    glm::vec3 up = glm::cross(forward, right);

    glm::mat4 rotation = glm::mat4(1.0f);
    rotation[0] = glm::vec4(right, 0.0f);
    rotation[1] = glm::vec4(up, 0.0f);
    rotation[2] = glm::vec4(-forward, 0.0f); // Domyślna siatka jest w stronę -Z, więc odwracamy forward

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = model * rotation;

    int modelLoc = glGetUniformLocation(shaderProgramID, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    glBindVertexArray(0);
}