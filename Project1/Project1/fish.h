#pragma once
#include <glm/glm.hpp>

class Fish {
private:
    unsigned int VAO, VBO;
    int vertexCount;
    void setupMesh();

public:
    Fish();
    ~Fish();

    void draw(const glm::vec3& position, const glm::vec3& direction, unsigned int shaderProgramID);
};
