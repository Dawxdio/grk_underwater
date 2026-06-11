#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int main() {
    // Initialize the library
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Create a windowed mode window and its OpenGL context
    GLFWwindow* window = glfwCreateWindow(640, 480, "Hello OpenGL", NULL, NULL);
    if (!window) {
        glfwTerminate();
        std::cerr << "Failed to create GLFW window" << std::endl;
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    // Kamera
    float camX = 0.0f;
    float camY = 2.0f;
    float camZ = 5.0f;

    float yaw = 0.0f;

    // Delta time
    float lastTime = (float)glfwGetTime();

    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window)) {

        // Obliczanie czasu klatki
        float currentTime = (float)glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        float moveSpeed = 3.0f * deltaTime;
        float rotSpeed = 2.0f * deltaTime;

        // Obrót kamery
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            yaw += rotSpeed;

        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            yaw -= rotSpeed;

        // Kierunek patrzenia
        float dirX = sin(yaw);
        float dirZ = -cos(yaw);

        // WASD

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            camX += dirX * moveSpeed;
            camZ += dirZ * moveSpeed;
        }

        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            camX -= dirX * moveSpeed;
            camZ -= dirZ * moveSpeed;
        }

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            camX += dirZ * moveSpeed;
            camZ -= dirX * moveSpeed;
        }

        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            camX -= dirZ * moveSpeed;
            camZ += dirX * moveSpeed;
        }

        // Góra / dół

        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            camY += moveSpeed;

        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
            camY -= moveSpeed;

        // Włączamy bufor głębokości
        glEnable(GL_DEPTH_TEST);

        // Niebo
        glClearColor(0.5f, 0.8f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Macierz projekcji

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        float aspect = (float)width / (float)height;

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        float nearPlane = 0.1f;
        float farPlane = 100.0f;
        float fov = 60.0f;

        float top = tan(fov * M_PI / 360.0f) * nearPlane;
        float bottom = -top;
        float right = top * aspect;
        float left = -right;

        glFrustum(
            left,
            right,
            bottom,
            top,
            nearPlane,
            farPlane
        );

        // Widok kamery

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glRotatef(
            -yaw * 180.0f / (float)M_PI,
            0.0f,
            1.0f,
            0.0f
        );

        glTranslatef(
            -camX,
            -camY,
            -camZ
        );

        // Powierzchnia wody (duży kwadrat)

        glColor3f(0.0f, 0.3f, 0.8f);

        glBegin(GL_QUADS);

        glVertex3f(-50.0f, 0.0f, -50.0f);
        glVertex3f(50.0f, 0.0f, -50.0f);
        glVertex3f(50.0f, 0.0f, 50.0f);
        glVertex3f(-50.0f, 0.0f, 50.0f);

        glEnd();

        // Mały czerwony znacznik w centrum

        glColor3f(1.0f, 0.0f, 0.0f);

        glBegin(GL_QUADS);

        glVertex3f(-0.5f, 0.01f, -0.5f);
        glVertex3f(0.5f, 0.01f, -0.5f);
        glVertex3f(0.5f, 0.01f, 0.5f);
        glVertex3f(-0.5f, 0.01f, 0.5f);

        glEnd();

        // Swap front and back buffers
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}