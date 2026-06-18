#include "movement.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

glm::vec3 cameraPos = glm::vec3(0.0f, 2.0f, 5.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float yaw = -90.0f;

float movementSpeed = 0.02f;
float rotationSpeed = 0.50f;

static void updateCameraVectors() {
	glm::vec3 front;
	front.x = cos(glm::radians(yaw));
	front.y = 0.0f;
	front.z = sin(glm::radians(yaw));

	cameraFront = glm::normalize(front);
}

void processInput(GLFWwindow* window) {
	// --- OBRÓT KAMERY (Q / E) ---
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
		yaw -= rotationSpeed;
		updateCameraVectors();
	}
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
		yaw += rotationSpeed;
		updateCameraVectors();
	}

	// --- RUCH KAMERY (W / A / S / D) ---
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		cameraPos += movementSpeed * cameraFront;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		cameraPos -= movementSpeed * cameraFront;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * movementSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * movementSpeed;
	}

	// --- RUCH GÓRA / DÓŁ (SPACJA / C) ---
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		cameraPos += movementSpeed * cameraUp; // Ruch w górę
	}
	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
		cameraPos -= movementSpeed * cameraUp; // Ruch w dół
	}
}

glm::mat4 getViewMatrix() {
	// glm::lookAt(pozycja_kamery, punkt_na_który_się_patrzy, wektor_góry)
	return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}
