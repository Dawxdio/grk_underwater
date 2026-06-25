#include "GL/glew.h"
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
#include "ShaderLoader.h"
#include "light_rays.h"
#include "spline_path.h"
#include "fish.h"
#include <cstddef>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


int main() {
    auto programStart = std::chrono::high_resolution_clock::now();

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Zapis korali do plików JSON (tylko raz, zakomentowane, bo nie chcemy nadpisywać przy każdym uruchomieniu)
    
    //writeCoralsOfType(0, 100);
    //writeCoralsOfType(1, 100);
    //writeCoralsOfType(2, 100);
    //writeCoralsOfType(3, 100);
    //writeCoralsOfType(4, 100);
    //writeCoralsOfType(-1, 100);

    GLFWwindow* window = glfwCreateWindow(640, 480, "Hello OpenGL", NULL, NULL);
    if (!window) {
        glfwTerminate();
        std::cerr << "Failed to create GLFW window" << std::endl;
        return -1;
    }

	
    glfwMakeContextCurrent(window);
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW!" << std::endl;
        return -1;
    }

    GLuint pbrShader = LoadShaders("pbr.vert", "pbr.frag");
    if (pbrShader == 0) {
        std::cerr << "Blad ladowania shadera PBR!" << std::endl;
        return -1;
    }

	GLuint waterShader = LoadShaders("water.vert", "water.frag");
    if (waterShader == 0) {
        std::cerr << "Blad ladowania shadera wody!" << std::endl;
        return -1;
    }

	GLuint waterVBO = 0;
    GLuint waterEBO = 0;
    int waterIndexCount = 0;

    // Siatka dla powierzchni wody
	const int GRID_RES = 100; // Rozdzielczość siatki
    const float SIZE = 50.0f;
    std::vector<float> verts;
    std::vector<unsigned int> inds;
    verts.reserve((GRID_RES + 1) * (GRID_RES + 1) * 3);
    for (int z = 0; z <= GRID_RES; ++z) {
        for (int x = 0; x <= GRID_RES; ++x) {
            float fx = ((float)x / (float)GRID_RES) * 2.0f * SIZE - SIZE;
            float fz = ((float)z / (float)GRID_RES) * 2.0f * SIZE - SIZE;
            verts.push_back(fx);
            verts.push_back(0.0f);
            verts.push_back(fz);
        }
    }
    for (int z = 0; z < GRID_RES; ++z) {
        for (int x = 0; x < GRID_RES; ++x) {
            int i0 = z * (GRID_RES + 1) + x;
            int i1 = i0 + 1;
            int i2 = i0 + (GRID_RES + 1);
            int i3 = i2 + 1;
			// Dwa trójkąty
            inds.push_back(i0);
            inds.push_back(i2);
            inds.push_back(i1);
            inds.push_back(i1);
            inds.push_back(i2);
            inds.push_back(i3);
        }
    }
    waterIndexCount = (int)inds.size();

    glGenBuffers(1, (unsigned int*)&waterVBO);
    glBindBuffer(GL_ARRAY_BUFFER, waterVBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, (unsigned int*)&waterEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, waterEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, inds.size() * sizeof(unsigned int), inds.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


    // Testowanie wyglądu korali w scenie (te na powierzchni)
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

    for (auto& coral : coralReef) {
        buildGpuSegmentsForCoral(coral); // To prześle geometrię korali na GPU przed uruchomieniem pętli renderu!
    }

    std::cout << "Liczba korali w coralReef: " << coralReef.size() << std::endl;

    for (size_t i = 0; i < coralReef.size(); ++i) {
        std::cout << "Koral [" << i << "]: "
            << "Pozycja: (" << coralReef[i].position.x << ", " << coralReef[i].position.y << ", " << coralReef[i].position.z << ") | "
            << "Liczba wezlow: " << coralReef[i].nodes.size() << " | "
            << "Szerokosc linii: " << coralReef[i].config.line_width << " | "
            << "Kolor RGB: (" << coralReef[i].config.color[0] << ", " << coralReef[i].config.color[1] << ", " << coralReef[i].config.color[2] << ")"
            << std::endl;
    }

    float lastTime = (float)glfwGetTime();

    auto now = std::chrono::high_resolution_clock::now();
    float elapsedTime =
        std::chrono::duration<float>(now - programStart).count();

    std::cout << "\rCzas programu: " << elapsedTime << " s" << std::flush;

	// Deklaracja ścieżki spline dla ryby
    SplinePath environmentPath;
    environmentPath.addPoint(glm::vec3(0.0f, -1.0f, 0.0f));
    environmentPath.addPoint(glm::vec3(2.0f, -0.5f, -3.0f));
    environmentPath.addPoint(glm::vec3(5.0f, -1.0f, -6.0f));
    environmentPath.addPoint(glm::vec3(2.0f, -2.0f, -9.0f));
    environmentPath.addPoint(glm::vec3(1.0f, -3.0f, -2.0f));
    environmentPath.addPoint(glm::vec3(3.0f, -1.5f, 3.0f));
    environmentPath.addPoint(glm::vec3(0.0f, -1.0f, 0.0f));

    Fish genericGoldfish;

    float pathProgress = 0.0f;
    float swimSpeed = 0.8f;


	// Główna pętla renderowania

    while (!glfwWindowShouldClose(window)) {


		processInput(window);
        float currentTime = (float)glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;


        glEnable(GL_DEPTH_TEST);

        if (cameraPos.y < 0.0f) {
            // Kamera pod wodą
            glClearColor(0.0f, 0.16f, 0.25f, 1.0f);

            // Mgła zależna od odległości
            glEnable(GL_FOG);
            GLfloat fogColor[4] = { 0.0f, 0.16f, 0.25f, 1.0f };
            glFogfv(GL_FOG_COLOR, fogColor);
            glFogi(GL_FOG_MODE, GL_EXP2);
            float baseDensity = 0.04f; // gęstość mgły
            float density = baseDensity * (1.0f + 0.2f * sinf(currentTime * 1.5f)); // lekkie falowanie
            glFogf(GL_FOG_DENSITY, density);
            glHint(GL_FOG_HINT, GL_NICEST);
        }
        else {
            // Niebo
            glDisable(GL_FOG);
            glClearColor(0.5f, 0.8f, 1.0f, 1.0f);
        }

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


        // Mały czerwony znacznik w centrum

        glColor3f(1.0f, 0.0f, 0.0f);

        glBegin(GL_QUADS);

        glVertex3f(-0.5f, 0.01f, -0.5f);
        glVertex3f(0.5f, 0.01f, -0.5f);
        glVertex3f(0.5f, 0.01f, 0.5f);
        glVertex3f(-0.5f, 0.01f, 0.5f);

        glEnd();


		// Rysowanie obiektów z użyciem PBR Shadera

        glUseProgram(pbrShader);

        // Wyciągamy stare macierze z OpenGL 1.1, żeby pasowały do reszty programu
        float modelview[16];
        float projection[16];
        glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
        glGetFloatv(GL_PROJECTION_MATRIX, projection);

        // Przekazujemy macierze do vertex shadera
        glUniformMatrix4fv(glGetUniformLocation(pbrShader, "view"), 1, GL_FALSE, modelview);
        glUniformMatrix4fv(glGetUniformLocation(pbrShader, "projection"), 1, GL_FALSE, projection);

        viewMatrix = getViewMatrix();
        glm::vec3 cameraPos = glm::vec3(glm::inverse(viewMatrix)[3]);
        glUniform1f(glGetUniformLocation(pbrShader, "time"), currentTime);

        glUniform3fv(glGetUniformLocation(pbrShader, "viewPos"), 1, &cameraPos[0]);
        // Ustawienie światła dla PBR
        glUniform3f(glGetUniformLocation(pbrShader, "lightPos"), 0.0f, 10.0f, 0.0f);
        glUniform3f(glGetUniformLocation(pbrShader, "lightColor"), 150.0f, 150.0f, 150.0f); // Mocne światło PBR
        glUniform3f(glGetUniformLocation(pbrShader, "fogColor"), 0.0f, 0.3f, 0.6f); // Taki sam jak kolor wody/tła
        glUniform1f(glGetUniformLocation(pbrShader, "fogDensity"), 0.04f);

        // Rysowanie korali
        for (const auto& coral : coralReef) {
            if (coral.segmentVBO == 0) continue;

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, coral.position);
            model = glm::rotate(model, glm::radians(coral.rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
            glUniformMatrix4fv(glGetUniformLocation(pbrShader, "model"), 1, GL_FALSE, &model[0][0]);

            GLint albedoLoc = glGetUniformLocation(pbrShader, "albedo");
            glUniform3fv(albedoLoc, 1, coral.config.color);

            glUniform1f(glGetUniformLocation(pbrShader, "metallic"), 0.05f);
            glUniform1f(glGetUniformLocation(pbrShader, "roughness"), 0.85f);

            glBindBuffer(GL_ARRAY_BUFFER, coral.segmentVBO);

            glEnableVertexAttribArray(0); // pozycja
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(PbrVertex), (void*)offsetof(PbrVertex, position));

            glEnableVertexAttribArray(1); // normalne
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(PbrVertex), (void*)offsetof(PbrVertex, normal));

            // Rysowanie geometrii
            glDrawArrays(GL_TRIANGLES, 0, coral.segmentVertexCount);

            glDisableVertexAttribArray(0);
            glDisableVertexAttribArray(1);
        }

        glUniform3f(glGetUniformLocation(pbrShader, "albedo"), 0.55f, 0.40f, 0.20f);

		// Macierz modelu dla dna (tożsamościowa, bo dno jest w stałej pozycji)
        glm::mat4 floorModel = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(pbrShader, "model"), 1, GL_FALSE, &floorModel[0][0]);


		// Rysowanie ryby poruszającej się po spline
        
        pathProgress += swimSpeed * deltaTime;

        if (pathProgress > environmentPath.controlPoints.size() - 1) {
            pathProgress = 0.0f;
        }

        glm::vec3 fishPosition = environmentPath.getPosition(pathProgress);
        glm::vec3 fishDirection = environmentPath.getTangent(pathProgress);

        genericGoldfish.draw(fishPosition, fishDirection, pbrShader);


        generate_ocean_floor();


		// Rysowanie wody

        if (waterShader != 0 && waterVBO != 0 && waterEBO != 0) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDepthMask(GL_FALSE);

            glUseProgram(waterShader);

            float nearPlane = 0.1f;
            float farPlane = 100.0f;
            float fov = 60.0f;
            float top = tan(fov * M_PI / 360.0f) * nearPlane;
            float bottom = -top;
            float right = top * ((float)width / (float)height);
            float left = -right;
            glm::mat4 proj = glm::frustum(left, right, bottom, top, nearPlane, farPlane);

            int locTime = glGetUniformLocation(waterShader, "uTime");
            if (locTime >= 0) glUniform1f(locTime, currentTime);

			// Parametry fali na powierzchni wody
            int locAmp1 = glGetUniformLocation(waterShader, "uAmp1"); if (locAmp1 >= 0) glUniform1f(locAmp1, 0.10f);
            int locFreq1 = glGetUniformLocation(waterShader, "uFreq1"); if (locFreq1 >= 0) glUniform1f(locFreq1, 0.04f);
            int locSpeed1 = glGetUniformLocation(waterShader, "uSpeed1"); if (locSpeed1 >= 0) glUniform1f(locSpeed1, 0.5f);
            int locAmp2 = glGetUniformLocation(waterShader, "uAmp2"); if (locAmp2 >= 0) glUniform1f(locAmp2, 0.05f);
            int locFreq2 = glGetUniformLocation(waterShader, "uFreq2"); if (locFreq2 >= 0) glUniform1f(locFreq2, 0.08f);
            int locSpeed2 = glGetUniformLocation(waterShader, "uSpeed2"); if (locSpeed2 >= 0) glUniform1f(locSpeed2, 0.9f);

            // Kolor wody (poszczególne składowe)
            int locR = glGetUniformLocation(waterShader, "uColR"); if (locR >= 0) glUniform1f(locR, 0.0f);
            int locG = glGetUniformLocation(waterShader, "uColG"); if (locG >= 0) glUniform1f(locG, 0.35f);
            int locB = glGetUniformLocation(waterShader, "uColB"); if (locB >= 0) glUniform1f(locB, 0.7f);
            int locA = glGetUniformLocation(waterShader, "uColA"); if (locA >= 0) glUniform1f(locA, 0.75f);

            // Rysowanie za pomocą client arrays
            glBindBuffer(GL_ARRAY_BUFFER, waterVBO);
            glEnableClientState(GL_VERTEX_ARRAY);
            glVertexPointer(3, GL_FLOAT, 0, (void*)0);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, waterEBO);
            glDrawElements(GL_TRIANGLES, waterIndexCount, GL_UNSIGNED_INT, 0);

            glDisableClientState(GL_VERTEX_ARRAY);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

            glUseProgram(0);
            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);
        }

        glUseProgram(0);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        glDepthMask(GL_FALSE);

        draw_god_rays();

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}