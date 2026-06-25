#ifndef TURTLE_H
#define TURTLE_H
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "spline_path.h"

class Turtle {
public:
    Turtle();
    ~Turtle();

    // Aktualizuje pozycję i ramkę PTF na podstawie postępu na splajnie
    void update(float progress, SplinePath& path);

    // Rysuje żółwia z użyciem wyliczonej macierzy PTF
void draw(unsigned int shaderProgramID, unsigned int textureID);

private:
    unsigned int VAO, VBO;
    int vertexCount;

    // Pozycja i aktualna baza wektorów PTF
    glm::vec3 position;
    glm::vec3 ptfForward;
    glm::vec3 ptfUp;
    glm::vec3 ptfRight;

    // Flaga do inicjalizacji pierwszej ramki
    bool isInitialized;
    float lastProgress;

    void setupMesh();
};

#endif