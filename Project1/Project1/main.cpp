#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <chrono>
#include "movement.h"
#include "coral_generation.h"
#include "ocean_floor.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int main() {
    auto programStart = std::chrono::high_resolution_clock::now();

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // Initialize the library
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Zapis korali do plików JSON (tylko raz, zakomentowane, bo nie chcemy nadpisywać przy każdym uruchomieniu)
    /*
    writeCoralsOfType(0, 100);
    writeCoralsOfType(1, 100);
    writeCoralsOfType(2, 100);
    writeCoralsOfType(3, 100);
    writeCoralsOfType(4, 100);
    writeCoralsOfType(-1, 100);*/

    // Create a windowed mode window and its OpenGL context
    GLFWwindow* window = glfwCreateWindow(640, 480, "Hello OpenGL", NULL, NULL);
    if (!window) {
        glfwTerminate();
        std::cerr << "Failed to create GLFW window" << std::endl;
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    // testowanie wyglądu korali w scenie(te na powierzchni)

    {
        float surfaceY = 0.01f; // tu jest powierzchnia wody w scenie
        float startX = -6.0f;
        float spacing = 2.0f;
        std::vector<int> testTypes = { 0, 1, 2, 3, 4, -1 };
        for (size_t i = 0; i < testTypes.size(); ++i) {
            glm::vec3 pos(startX + (float)i * spacing, surfaceY, 0.0f);
            CoralInstance c;
            bool loaded = loadRandomCoralFromFile(testTypes[i], c);
            if (!loaded) {
                std::cerr << "Brak pliku lub blad parsowania dla typu korala: " << testTypes[i] << ". Pomijam." << std::endl;
                continue; // nie generujemy proceduralnie, idziemy dalej
            }
            c.position = pos;
            c.rotationY = (std::rand() / (float)RAND_MAX) * 360.0f;
            coralReef.push_back(c);
        }
    }

	generate_single_coral(0, glm::vec3(0.0f, 0.0f, 0.0f));

    glm::vec2 min_obszar1(-5.0f, -5.0f);
    glm::vec2 max_obszar1(5.0f, 5.0f);
    float gestosc1 = 0.7f;
    int typ_korala1 = 4;

    generate_coral_reef(min_obszar1, max_obszar1, gestosc1, typ_korala1);


    glm::vec2 min_obszar2(-15.0f, -15.0f);
    glm::vec2 max_obszar2(-5.0f, -5.0f);
    float gestosc2 = 0.15f;
    int typ_korala2 = 2;

    generate_coral_reef(min_obszar2, max_obszar2, gestosc2, typ_korala2);

    glm::vec2 min_obszar3(5.0f, 5.0f);
    glm::vec2 max_obszar3(15.0f, 15.0f);
    float gestosc3 = 0.15f;
    int typ_korala3 = 3;

    generate_coral_reef(min_obszar3, max_obszar3, gestosc3, typ_korala3);

    glm::vec2 min_obszar4(-15.0f, 5.0f);
    glm::vec2 max_obszar4(-5.0f, 15.0f);
    float gestosc4 = 0.15f;
    int typ_korala4 = 1;

    generate_coral_reef(min_obszar4, max_obszar4, gestosc4, typ_korala4);

    glm::vec2 min_obszar5(5.0f, -15.0f);
    glm::vec2 max_obszar5(15.0f, -5.0f);
    float gestosc5 = 0.15f;
    int typ_korala5 = 1;

    generate_coral_reef(min_obszar5, max_obszar5, gestosc5, typ_korala5);


    std::cout << "Liczba korali w coralReef: " << coralReef.size() << std::endl;

    for (size_t i = 0; i < coralReef.size(); ++i) {
        std::cout << "Koral [" << i << "]: "
            << "Pozycja: (" << coralReef[i].position.x << ", " << coralReef[i].position.y << ", " << coralReef[i].position.z << ") | "
            << "Liczba wezlow: " << coralReef[i].nodes.size() << " | "
            << "Szerokosc linii: " << coralReef[i].config.line_width << " | "
            << "Kolor RGB: (" << coralReef[i].config.color[0] << ", " << coralReef[i].config.color[1] << ", " << coralReef[i].config.color[2] << ")"
            << std::endl;
    }
    // Delta time
    float lastTime = (float)glfwGetTime();

    auto now = std::chrono::high_resolution_clock::now();
    float elapsedTime =
        std::chrono::duration<float>(now - programStart).count();

    std::cout << "\rCzas programu: " << elapsedTime << " s" << std::flush;

    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window)) {


		processInput(window);
        // Obliczanie czasu klatki
        float currentTime = (float)glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;


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

        glm::mat4 viewMatrix = getViewMatrix();
        glLoadMatrixf(&viewMatrix[0][0]);
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

        for (const auto& coral : coralReef) {
            glPushMatrix();

            // Przesuwamy się do punktu na dnie, gdzie koral ma rosnąć
            glTranslatef(coral.position.x, coral.position.y, coral.position.z);
            glRotatef(coral.rotationY, 0.0f, 1.0f, 0.0f);
            // Ustawiamy styl linii dla tego konkretnego typu korala
            glLineWidth(coral.config.line_width);
            glColor3f(coral.config.color[0], coral.config.color[1], coral.config.color[2]);

            // Rysujemy gałęzie: jeśli config.branch_radius>0 rysujemy przybliżony walec,
            // w przeciwnym razie rysujemy prostą linię
            for (const auto& n : coral.nodes) {
                if (n.parentIndex != -1) {
                    const Node& parentNode = coral.nodes[n.parentIndex];
                    // Używamy pełnych współrzędnych 3D (x,y,z) dla węzłów
                    glm::vec3 pa(parentNode.x, parentNode.y, parentNode.z);
                    glm::vec3 pb(n.x, n.y, n.z);
                    if (coral.config.branch_radius > 0.0001f) {
                        // Rysuj przybliżony walec między punktami
                        draw_cylinder_between(pa, pb, coral.config.branch_radius, 10);
                    }
                    else {
                        glBegin(GL_LINES);
                        glVertex3f(parentNode.x, parentNode.y, parentNode.z);
                        glVertex3f(n.x, n.y, n.z);
                        glEnd();
                    }
                }
            }

            // Rysujemy dodatkowe punkty/teksturę (jeśli koral ma point_style > 0)
            if (coral.config.point_style > 0) {
                if (coral.config.point_style == 1) {
                    glPointSize(2.0f);
                    glColor3f(0.1f, 0.1f, 0.1f);
                }
                else if (coral.config.point_style == 2) {
                    glPointSize(5.0f);
                    glColor3f(0.05f, 0.15f, 0.22f);
                }

                glBegin(GL_POINTS);
                for (const auto& n : coral.nodes) {
                    glVertex3f(n.x, n.y, n.z);
                }
                glEnd();
            }

            glPopMatrix(); // Przywracamy macierz, żeby kolejny koral rysował się od czystej pozycji
        }

        generate_ocean_floor();

        // Swap front and back buffers
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}