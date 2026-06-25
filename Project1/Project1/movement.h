#ifndef MOVEMENT_H
#define MOVEMENT_H

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

extern glm::vec3 cameraPos;
extern glm::quat cameraQuat;

void processInput(GLFWwindow* window);
glm::mat4 getViewMatrix();
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);

#endif