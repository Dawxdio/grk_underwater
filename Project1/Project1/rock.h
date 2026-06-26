#ifndef ROCK_H
#define ROCK_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

class Rock {
public:
    Rock(const std::string& objPath, const glm::vec3& startPosition, float startScale, float startRotationY);
    ~Rock();

    void draw(unsigned int shaderProgramID, unsigned int textureID);

private:
    void setupMesh(const std::string& objPath);
    float rotationY;
    GLuint VAO, VBO;
    GLsizei vertexCount;
    glm::vec3 position;
    float scale;
};

void generateRandomRocks(std::vector<Rock>& outRocks,
    float minX, float maxX,
    float minZ, float maxZ,
    int density);

#endif // ROCK_H