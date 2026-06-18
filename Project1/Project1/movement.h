#ifndef MOVEMENT_H
#define MOVEMENT_H

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

// Kamera i parametry ruchu (widoczne z innych modułów jeśli potrzebne)
extern glm::vec3 cameraPos;
extern glm::vec3 cameraFront;
extern glm::vec3 cameraUp;

extern float yaw;
extern float movementSpeed;
extern float rotationSpeed;

// Przetwarzanie wejścia (klawiatura) i pobranie macierzy widoku
void processInput(GLFWwindow* window);
glm::mat4 getViewMatrix();

#endif // MOVEMENT_H
