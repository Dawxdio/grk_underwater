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
#include "TextureLoader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "turtle.h"
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

    // Ensure STB doesn't flip cubemaps vertically (cubemap coordinates differ from 2D textures)
    stbi_set_flip_vertically_on_load(false);

    int width, height, nrChannels;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    for (unsigned int i = 0; i < faces.size(); i++) {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            // Dynamically assign the format based on the image's actual channels
            GLenum format = GL_RGB;
            if (nrChannels == 1) format = GL_RED;
            else if (nrChannels == 3) format = GL_RGB;
            else if (nrChannels == 4) format = GL_RGBA;

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
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

unsigned int loadTexture2D(const char* path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrChannels;
    // Odwracamy teksturę pionowo, bo OpenGL czyta od dołu do góry, a pliki graficzne od góry do dołu
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);

    if (data) {
        GLenum format = GL_RGB;
        if (nrChannels == 1) format = GL_RED;
        else if (nrChannels == 3) format = GL_RGB;
        else if (nrChannels == 4) format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else {
        std::cout << "Blad wczytywania tekstury pod sciezka: " << path << std::endl;
        stbi_image_free(data);
    }
    // Przywracamy domyślne zachowanie dla skyboxa
    stbi_set_flip_vertically_on_load(false);

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

    // Load per-type coral textures (normal + albedo). Types assumed in range 0..MAX_CORAL_TYPE (adjust if needed)
    const int MAX_CORAL_TYPE = 4;
    std::vector<GLuint> coralNormalMaps(MAX_CORAL_TYPE + 1, 0);
    std::vector<GLuint> coralAlbedoMaps(MAX_CORAL_TYPE + 1, 0);

    for (int t = 0; t <= MAX_CORAL_TYPE; ++t) {
        std::string normalName = "coral_normal" + std::to_string(t) + ".png";
        coralNormalMaps[t] = loadTexture(normalName.c_str());
        std::string albedoName = "coral_texture" + std::to_string(t) + ".jpg";
        coralAlbedoMaps[t] = loadTexture(albedoName.c_str());
    }

    // Load sand textures for ocean floor
    GLuint sandNormalTex = loadTexture("sand_normal.png");
    GLuint sandAlbedoTex = loadTexture("sand_texture.jpg");

    // Fallback textures: neutral normal (128,128,255) and white albedo
    GLuint fallbackNormal = 0;
    GLuint fallbackAlbedo = 0;
    {
        unsigned char npx[3] = { 128, 128, 255 };
        glGenTextures(1, &fallbackNormal);
        glBindTexture(GL_TEXTURE_2D, fallbackNormal);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, npx);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    {
        unsigned char apx[3] = { 255,255,255 };
        glGenTextures(1, &fallbackAlbedo);
        glBindTexture(GL_TEXTURE_2D, fallbackAlbedo);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, apx);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    // Set sampler bindings in shader (normal -> unit 0, albedo -> unit 1)
    glUseProgram(pbrShader);
    glUniform1i(glGetUniformLocation(pbrShader, "normalMap"), 0);
    glUniform1i(glGetUniformLocation(pbrShader, "albedoMap"), 1);
    glUseProgram(0);

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

    SplinePath turtlePath;
    turtlePath.addPoint(glm::vec3(-10.0f, -4.0f, -5.0f));
    turtlePath.addPoint(glm::vec3(-5.0f, -3.5f, -12.0f));
    turtlePath.addPoint(glm::vec3(4.0f, -5.0f, -8.0f));
    turtlePath.addPoint(glm::vec3(8.0f, -3.0f, 2.0f));
    turtlePath.addPoint(glm::vec3(0.0f, -4.5f, 10.0f));
    turtlePath.addPoint(glm::vec3(-8.0f, -5.0f, 4.0f));
    turtlePath.addPoint(glm::vec3(-10.0f, -4.0f, -5.0f));

    Fish genericFish;
    float pathProgress = 0.0f;
    float swimSpeed = 0.8f;

    Turtle myTurtle;
    float turtleProgress = 0.0f;
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
    unsigned int turtleTextureID = loadTexture2D("textures/tutle/body_Base_color.png");

    bool startCoralGrowth = false;
    float coralGrowthFactor = 0.0f; // 0.0 = brak korala, 1.0 = koral w pełni wyrośnięty
    const float GROWTH_SPEED = 0.2f;
    // Główna pętla renderowania
    while (!glfwWindowShouldClose(window)) {

        processInput(window);
        float currentTime = (float)glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        glEnable(GL_DEPTH_TEST);

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

        // Rysowanie skyboxa
        glDepthFunc(GL_LEQUAL);
        glUseProgram(skyboxShader);

        glm::mat4 staticView = glm::mat4(glm::mat3(getViewMatrix()));
        glm::mat4 projectionMatrix = glm::perspective(glm::radians(60.0f), aspect, nearPlane, farPlane);

        glUniformMatrix4fv(glGetUniformLocation(skyboxShader, "view"), 1, GL_FALSE, &staticView[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(skyboxShader, "projection"), 1, GL_FALSE, &projectionMatrix[0][0]);

        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTextureID);
        glUniform1i(glGetUniformLocation(skyboxShader, "skybox"), 0);

        glDrawArrays(GL_TRIANGLES, 0, 36);

        glBindVertexArray(0);

        glDepthFunc(GL_LESS);
        glDepthMask(GL_TRUE);


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
        glUniform3f(glGetUniformLocation(pbrShader, "lightPos"), -25.0f, 50.0f, -60.0f);
        glUniform3f(glGetUniformLocation(pbrShader, "lightColor"), 300.0f, 300.0f, 300.0f);
        glUniform3f(glGetUniformLocation(pbrShader, "fogColor"), 0.0f, 0.3f, 0.6f);
        glUniform1f(glGetUniformLocation(pbrShader, "fogDensity"), 0.04f);

        // Rysowanie korali
        for (size_t i = 0; i < coralReef.size(); i++) {
            const auto& coral = coralReef[i];
            if (coral.segmentVBO == 0) continue;

            int ctype = (coral.coralType >= 0 && coral.coralType <= MAX_CORAL_TYPE) ? coral.coralType : 0;
            GLuint normalTex = coralNormalMaps[ctype] != 0 ? coralNormalMaps[ctype] : fallbackNormal;
            GLuint albedoTex = coralAlbedoMaps[ctype] != 0 ? coralAlbedoMaps[ctype] : fallbackAlbedo;
            bool hasAlbedo = (coralAlbedoMaps[ctype] != 0);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, normalTex);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, albedoTex);

            glUniform1i(glGetUniformLocation(pbrShader, "useAlbedoMap"), hasAlbedo ? 1 : 0);

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, coral.position);
            model = glm::rotate(model, glm::radians(coral.rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
            glUniformMatrix4fv(glGetUniformLocation(pbrShader, "model"), 1, GL_FALSE, &model[0][0]);

            if (!hasAlbedo) {
                GLint albedoLoc = glGetUniformLocation(pbrShader, "albedo");
                glUniform3fv(albedoLoc, 1, coral.config.color);
            }

            glUniform1f(glGetUniformLocation(pbrShader, "metallic"), 0.05f);
            glUniform1f(glGetUniformLocation(pbrShader, "roughness"), 0.85f);

            glBindBuffer(GL_ARRAY_BUFFER, coral.segmentVBO);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(PbrVertex), (void*)offsetof(PbrVertex, position));
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(PbrVertex), (void*)offsetof(PbrVertex, normal));
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(PbrVertex), (void*)offsetof(PbrVertex, texCoords));
            glEnableVertexAttribArray(3);
            glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(PbrVertex), (void*)offsetof(PbrVertex, tangent));
            glEnableVertexAttribArray(4);
            glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(PbrVertex), (void*)offsetof(PbrVertex, bitangent));

            const int VERTICES_PER_CYLINDER = 60;

            int totalCylinders = coral.segmentVertexCount / VERTICES_PER_CYLINDER;
            if (totalCylinders < 1) totalCylinders = 1;

            int visibleCylinders = int(totalCylinders * coralGrowthFactor);
            int verticesToDraw = visibleCylinders * VERTICES_PER_CYLINDER;

            if (verticesToDraw == 0 && coralGrowthFactor > 0.001f) {
                verticesToDraw = VERTICES_PER_CYLINDER;
            }
            // Zabezpieczenie przed wyjściem poza zakres bufora
            if (verticesToDraw > coral.segmentVertexCount) {
                verticesToDraw = coral.segmentVertexCount;
            }

            glDrawArrays(GL_TRIANGLES, 0, verticesToDraw);

            glDisableVertexAttribArray(0);
            glDisableVertexAttribArray(1);
            glDisableVertexAttribArray(2);
            glDisableVertexAttribArray(3);
            glDisableVertexAttribArray(4);
        }

        // Bind sand textures for ocean floor (normal->unit0, albedo->unit1)
        GLuint bindNormal = (sandNormalTex != 0) ? sandNormalTex : fallbackNormal;
        GLuint bindAlbedo = (sandAlbedoTex != 0) ? sandAlbedoTex : fallbackAlbedo;
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, bindNormal);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, bindAlbedo);
        // Notify shader to use albedoMap for floor
        glUniform1i(glGetUniformLocation(pbrShader, "useAlbedoMap"), 1);

        glm::mat4 floorModel = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(pbrShader, "model"), 1, GL_FALSE, &floorModel[0][0]);

        generate_ocean_floor();

        // After drawing floor, disable albedoMap usage for subsequent objects unless set per-object
        glUniform1i(glGetUniformLocation(pbrShader, "useAlbedoMap"), 0);

        // Rysowanie ryby po spline
        pathProgress += swimSpeed * deltaTime;
        if (pathProgress > environmentPath.controlPoints.size() - 1) {
            pathProgress = 0.0f;
        }
        glm::vec3 fishPosition = environmentPath.getPosition(pathProgress);
        glm::vec3 fishDirection = environmentPath.getTangent(pathProgress);
        genericFish.draw(fishPosition, fishDirection, pbrShader);

        turtleProgress += (swimSpeed * 0.5f) * deltaTime; // Żółw pływa wolniej
        if (turtleProgress > turtlePath.controlPoints.size() - 1) {
            turtleProgress = 0.0f;
        }

        // Podajemy nową ścieżkę jako argument metody update
        myTurtle.update(turtleProgress, turtlePath); // tutaj używasz nowej trasy, którą zrobiliśmy krok wcześniej
        myTurtle.draw(pbrShader, turtleTextureID);   // przekazujemy ID tekstury

        // Rysowanie wody
        if (waterShader != 0 && waterVBO != 0 && waterEBO != 0) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDepthMask(GL_FALSE);

            glUseProgram(waterShader);

            int locTime = glGetUniformLocation(waterShader, "uTime");
            if (locTime >= 0) glUniform1f(locTime, currentTime);

            glUniform3f(glGetUniformLocation(waterShader, "uLightDir"), -25.0f, 50.0f, -60.0f);
            glUniform3fv(glGetUniformLocation(waterShader, "uViewPos"), 1, &shaderCameraPos[0]);

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
        glDepthMask(GL_TRUE);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}