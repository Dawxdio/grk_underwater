#include "GL/glew.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>
#include "movement.h"
#include "coral_generation.h"
#include "ocean_floor.h"
#include "ShaderLoader.h"
#include "light_rays.h"
#include "spline_path.h"
#include "fish.h"
#include <cstddef>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

float skyboxVertices[] = {
    // Positions          
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

unsigned int loadCubemap(std::vector<std::string> faces) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++) {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            // Notice how we add 'i' to the first target enum to loop through all 6 faces sequentially
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }

    // Set texture filtering and clamping properties
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}


int main() {
    auto programStart = std::chrono::high_resolution_clock::now();

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

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

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);

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

    GLuint skyboxShader = LoadShaders("skybox.vert", "skybox.frag");
    if (skyboxShader == 0) {
        std::cerr << "Blad ladowania shadera skybox!" << std::endl;
        return -1;
    }

    GLuint waterVBO = 0;
    GLuint waterEBO = 0;
    int waterIndexCount = 0;

    // Siatka dla powierzchni wody
    const int GRID_RES = 100;
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

    // Generowanie raf koralowych
    generate_coral_reef(glm::vec2(-5.0f, -5.0f), glm::vec2(5.0f, 5.0f), 0.7f, 4);
    generate_coral_reef(glm::vec2(-15.0f, -15.0f), glm::vec2(-5.0f, -5.0f), 0.15f, 2);
    generate_coral_reef(glm::vec2(5.0f, 5.0f), glm::vec2(15.0f, 15.0f), 0.15f, 3);
    generate_coral_reef(glm::vec2(-15.0f, 5.0f), glm::vec2(-5.0f, 15.0f), 0.15f, 1);
    generate_coral_reef(glm::vec2(5.0f, -15.0f), glm::vec2(15.0f, -5.0f), 0.15f, 1);

    for (auto& coral : coralReef) {
        buildGpuSegmentsForCoral(coral);
    }

    float lastTime = (float)glfwGetTime();

    SplinePath environmentPath;
    environmentPath.addPoint(glm::vec3(0.0f, -1.0f, 0.0f));
    environmentPath.addPoint(glm::vec3(2.0f, -0.5f, -3.0f));
    environmentPath.addPoint(glm::vec3(5.0f, -1.0f, -6.0f));
    environmentPath.addPoint(glm::vec3(2.0f, -2.0f, -9.0f));
    environmentPath.addPoint(glm::vec3(1.0f, -3.0f, -2.0f));
    environmentPath.addPoint(glm::vec3(3.0f, -1.5f, 3.0f));
    environmentPath.addPoint(glm::vec3(0.0f, -1.0f, 0.0f));

    Fish genericFish;
    float pathProgress = 0.0f;
    float swimSpeed = 0.8f;


    // Inicjalizacja skyboxa

    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	std::vector<std::string> faces
	{
		"textures/skybox/px.png",
		"textures/skybox/nx.png",
		"textures/skybox/py.png",
		"textures/skybox/ny.png",
		"textures/skybox/pz.png",
		"textures/skybox/nz.png"
	};

	unsigned int cubemapTextureID = loadCubemap(faces);
    

    // Główna pętla renderowania
    while (!glfwWindowShouldClose(window)) {

        processInput(window);
        float currentTime = (float)glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        glEnable(GL_DEPTH_TEST);


        if (cameraPos.y < 0.0f) {
            glClearColor(0.0f, 0.16f, 0.25f, 1.0f);
            glEnable(GL_FOG);
            GLfloat fogColor[4] = { 0.0f, 0.16f, 0.25f, 1.0f };
            glFogfv(GL_FOG_COLOR, fogColor);
            glFogi(GL_FOG_MODE, GL_EXP2);
            glFogf(GL_FOG_DENSITY, 0.04f);
            glHint(GL_FOG_HINT, GL_NICEST);
        }
        else {
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

        glFrustum(left, right, bottom, top, nearPlane, farPlane);

        // Widok kamery
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glm::mat4 viewMatrix = getViewMatrix();
        glLoadMatrixf(&viewMatrix[0][0]);

        // Czerwony znacznik
        glColor3f(1.0f, 0.0f, 0.0f);
        glBegin(GL_QUADS);
        glVertex3f(-0.5f, 0.01f, -0.5f);
        glVertex3f(0.5f, 0.01f, -0.5f);
        glVertex3f(0.5f, 0.01f, 0.5f);
        glVertex3f(-0.5f, 0.01f, 0.5f);
        glEnd();

        // Rysowanie obiektów z użyciem PBR Shadera
        glUseProgram(pbrShader);

        float modelview[16];
        float projection[16];
        glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
        glGetFloatv(GL_PROJECTION_MATRIX, projection);

        glUniformMatrix4fv(glGetUniformLocation(pbrShader, "view"), 1, GL_FALSE, modelview);
        glUniformMatrix4fv(glGetUniformLocation(pbrShader, "projection"), 1, GL_FALSE, projection);

        glm::vec3 shaderCameraPos = glm::vec3(glm::inverse(viewMatrix)[3]);
        glUniform3fv(glGetUniformLocation(pbrShader, "viewPos"), 1, &shaderCameraPos[0]);

        glUniform1f(glGetUniformLocation(pbrShader, "time"), currentTime);
        glUniform3f(glGetUniformLocation(pbrShader, "lightPos"), 0.0f, 10.0f, 0.0f);
        glUniform3f(glGetUniformLocation(pbrShader, "lightColor"), 150.0f, 150.0f, 150.0f);
        glUniform3f(glGetUniformLocation(pbrShader, "fogColor"), 0.0f, 0.3f, 0.6f);
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
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(PbrVertex), (void*)offsetof(PbrVertex, position));
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(PbrVertex), (void*)offsetof(PbrVertex, normal));

            glDrawArrays(GL_TRIANGLES, 0, coral.segmentVertexCount);

            glDisableVertexAttribArray(0);
            glDisableVertexAttribArray(1);
        }

        glUniform3f(glGetUniformLocation(pbrShader, "albedo"), 0.55f, 0.40f, 0.20f);
        glm::mat4 floorModel = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(pbrShader, "model"), 1, GL_FALSE, &floorModel[0][0]);

        generate_ocean_floor();

        // Rysowanie ryby po spline
        pathProgress += swimSpeed * deltaTime;
        if (pathProgress > environmentPath.controlPoints.size() - 1) {
            pathProgress = 0.0f;
        }
        glm::vec3 fishPosition = environmentPath.getPosition(pathProgress);
        glm::vec3 fishDirection = environmentPath.getTangent(pathProgress);
        genericFish.draw(fishPosition, fishDirection, pbrShader);

        // Rysowanie wody
        if (waterShader != 0 && waterVBO != 0 && waterEBO != 0) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDepthMask(GL_FALSE);

            glUseProgram(waterShader);

            int locTime = glGetUniformLocation(waterShader, "uTime");
            if (locTime >= 0) glUniform1f(locTime, currentTime);

            int locAmp1 = glGetUniformLocation(waterShader, "uAmp1"); if (locAmp1 >= 0) glUniform1f(locAmp1, 0.10f);
            int locFreq1 = glGetUniformLocation(waterShader, "uFreq1"); if (locFreq1 >= 0) glUniform1f(locFreq1, 0.04f);
            int locSpeed1 = glGetUniformLocation(waterShader, "uSpeed1"); if (locSpeed1 >= 0) glUniform1f(locSpeed1, 0.5f);
            int locAmp2 = glGetUniformLocation(waterShader, "uAmp2"); if (locAmp2 >= 0) glUniform1f(locAmp2, 0.05f);
            int locFreq2 = glGetUniformLocation(waterShader, "uFreq2"); if (locFreq2 >= 0) glUniform1f(locFreq2, 0.08f);
            int locSpeed2 = glGetUniformLocation(waterShader, "uSpeed2"); if (locSpeed2 >= 0) glUniform1f(locSpeed2, 0.9f);

            int locR = glGetUniformLocation(waterShader, "uColR"); if (locR >= 0) glUniform1f(locR, 0.0f);
            int locG = glGetUniformLocation(waterShader, "uColG"); if (locG >= 0) glUniform1f(locG, 0.35f);
            int locB = glGetUniformLocation(waterShader, "uColB"); if (locB >= 0) glUniform1f(locB, 0.7f);
            int locA = glGetUniformLocation(waterShader, "uColA"); if (locA >= 0) glUniform1f(locA, 0.75f);

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

        glDisable(GL_BLEND);
        glDisable(GL_FOG);

		// Rysowanie skyboxa
        glUseProgram(skyboxShader);
        
        // Get your current view matrix (with translation removed)
        // Strip the translation component so the skybox stays centered around camera
        glm::mat4 staticView = glm::mat4(glm::mat3(getViewMatrix()));

        // Regenerate projection matrix matching your glFrustum calculation
        glm::mat4 projectionMatrix = glm::perspective(glm::radians(60.0f), aspect, nearPlane, farPlane);

        // Pass matrices to skybox shader
        glUniformMatrix4fv(glGetUniformLocation(skyboxShader, "view"), 1, GL_FALSE, &staticView[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(skyboxShader, "projection"), 1, GL_FALSE, &projectionMatrix[0][0]);

        // Bind skybox VAO and Cubemap Texture
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTextureID);
        glUniform1i(glGetUniformLocation(skyboxShader, "skybox"), 0);

        // Render the cube
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glBindVertexArray(0);

        glDepthMask(GL_TRUE);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}