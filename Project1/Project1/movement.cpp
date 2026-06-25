#include "movement.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <cmath>
#include <iostream>

// Pozycja i orientacja kamery (jako kwaternion)
glm::vec3 cameraPos = glm::vec3(0.0f, 2.0f, 5.0f);
glm::quat cameraQuat = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // Kwaternion tożsamościowy (brak obrotu)

float movementSpeed = 0.10f;
float mouseSensitivity = 0.10f;

// Zmienne do obsługi myszy
float lastX = 400.0f, lastY = 300.0f;
bool firstMouse = true;

// Funkcja callback dla myszy
float totalYaw = -90.0f;  // Początkowy yaw (taki sam jak miałeś na początku)
float totalPitch = 0.0f;  // Początkowy pitch (patrzymy na wprost)

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // Odwrócone, bo współrzędne Y idą od dołu do góry
    lastX = xpos;
    lastY = ypos;

    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    // Aktualizujemy globalne kąty
    totalYaw += xoffset;
    totalPitch += yoffset;

    if (totalPitch > 89.0f)  totalPitch = 89.0f;
    if (totalPitch < -89.0f) totalPitch = -89.0f;

    // 1. Tworzymy czyste kwaterniony dla Yaw (wokół osi Y świata) i Pitch (wokół osi X świata)
    glm::quat qYaw = glm::angleAxis(glm::radians(-totalYaw), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::quat qPitch = glm::angleAxis(glm::radians(totalPitch), glm::vec3(1.0f, 0.0f, 0.0f));

    cameraQuat = qYaw * qPitch;
    cameraQuat = glm::normalize(cameraQuat);
}
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
        return; // Przerywamy dalsze przetwarzanie, bo i tak zamykamy okno
    }
    // Pobieranie aktualnych wektorów kierunku bezpośrednio z kwaternionu orientacji
    glm::vec3 front = cameraQuat * glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 right = cameraQuat * glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 up = cameraQuat * glm::vec3(0.0f, 1.0f, 0.0f);

    // --- RUCH KAMERY (W / A / S / D) ---
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        cameraPos += movementSpeed * front;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        cameraPos -= movementSpeed * front;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        cameraPos -= movementSpeed * right;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        cameraPos += movementSpeed * right;
    }

    // --- RUCH GÓRA / DÓŁ (SPACJA / C) ---
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        cameraPos += movementSpeed * up;
    }
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
        cameraPos -= movementSpeed * up;
    }
}

glm::mat4 getViewMatrix() {
    // Za pomocą kwaternionu tworzymy macierz widoku bez używania glm::lookAt
    glm::mat4 view = glm::mat4_cast(glm::conjugate(cameraQuat)); // Sprzężenie kwaternionu odwraca obrót dla kamery
    view = glm::translate(view, -cameraPos);
    return view;
}