#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <cstdlib>
#include <ctime>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// --- Space Colonization Structures ---
struct Node {
    float x, y;
    int parentIndex;
    float dirX, dirY;
    int count;
};

struct Attractor {
    float x, y;
    bool active;
};

struct CoralConfig {
    int num_attractors;
    float min_dist;
    float max_dist;
    float branch_length;
    float upward_bias;
    float color[3];
    float line_width;
    int point_style; // 0: None, 1: Spotted, 2: Segmented Nodes
};

// Global structures to hold the generated coral data
std::vector<Node> coralNodes;
CoralConfig currentConfig;

// --- Space Colonization Generation Function ---
void generate_coral(int coral_type) {
    coralNodes.clear();
    std::vector<Attractor> attractors;

    // Seed random number generator
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // Define structural presets mimicking the reference profiles
    switch (coral_type) {
    case 0: // Staghorn Coral / Classic Branching
        currentConfig = { 500, 0.08f, 0.6f, 0.05f, 0.3f, {1.0f, 0.42f, 0.55f}, 2.0f, 0 };
        for (int i = 0; i < currentConfig.num_attractors; ++i) {
            float theta = (rand() / (float)RAND_MAX) * (float)M_PI;
            float r = 0.5f + (rand() / (float)RAND_MAX) * 3.0f;
            attractors.push_back({ r * cos(theta), r * sin(theta) + 0.2f, true });
        }
        break;

    case 1: // Sea Fan (Broad, flat, lacy growth layout)
        currentConfig = { 700, 0.04f, 0.4f, 0.03f, 0.4f, {0.17f, 0.79f, 0.84f}, 1.2f, 0 };
        for (int i = 0; i < currentConfig.num_attractors; ++i) {
            float x = ((rand() / (float)RAND_MAX) * 2.0f - 1.0f) * 2.5f;
            float y = 0.2f + (rand() / (float)RAND_MAX) * 3.5f;
            attractors.push_back({ x, y, true });
        }
        break;

    case 2: // Brain Coral (Convoluted, dense low-lying profile)
        currentConfig = { 600, 0.03f, 0.25f, 0.02f, 0.1f, {0.82f, 0.70f, 0.55f}, 3.5f, 1 };
        for (int i = 0; i < currentConfig.num_attractors; ++i) {
            float x = ((rand() / (float)RAND_MAX) * 2.0f - 1.0f) * 1.5f;
            float y = 0.1f + (rand() / (float)RAND_MAX) * 1.2f;
            attractors.push_back({ x, y, true });
        }
        break;

    case 3: // Brush Coral / Shelf Plates
        currentConfig = { 600, 0.10f, 0.7f, 0.06f, 0.6f, {1.0f, 0.8f, 0.5f}, 4.0f, 0 };
        for (int i = 0; i < currentConfig.num_attractors; ++i) {
            int layer = rand() % 3;
            float y_min = 0.4f + layer * 1.1f;
            float x_range = 1.0f + layer * 0.7f;
            float x = ((rand() / (float)RAND_MAX) * 2.0f - 1.0f) * x_range;
            float y = y_min + (rand() / (float)RAND_MAX) * 0.4f;
            attractors.push_back({ x, y, true });
        }
        break;

    case 4: // Bamboo Coral (Segmented straight vertical nodes)
        currentConfig = { 250, 0.20f, 0.9f, 0.14f, 0.8f, {0.84f, 0.88f, 0.90f}, 1.5f, 2 };
        for (int i = 0; i < currentConfig.num_attractors; ++i) {
            int spire = rand() % 3;
            float x_base = -1.5f + spire * 1.5f;
            float x = x_base + ((rand() / (float)RAND_MAX) * 2.0f - 1.0f) * 0.2f;
            float y = 0.4f + (rand() / (float)RAND_MAX) * 3.8f;
            attractors.push_back({ x, y, true });
        }
        break;

    default: // Fallback to basic setup
        currentConfig = { 400, 0.1f, 0.5f, 0.05f, 0.2f, {1.0f, 1.0f, 1.0f}, 1.5f, 0 };
        break;
    }

    // Initialize root node at the coordinate origin on the floor
    Node root = { 0.0f, 0.0f, -1, 0.0f, 0.0f, 0 };
    coralNodes.push_back(root);

    bool growing = true;
    int iterations = 0;
    int max_iterations = 1000; // Safety ceiling break

    // Pre-calculate full growth sequence instantly
    while (growing && iterations < max_iterations) {
        iterations++;

        for (auto& n : coralNodes) {
            n.dirX = 0.0f;
            n.dirY = 0.0f;
            n.count = 0;
        }

        bool attractor_influencing = false;

        // Associate active attractors with their closest node counterpart
        for (auto& a : attractors) {
            if (!a.active) continue;

            float minDist = 1e9f;
            int closestIdx = -1;

            for (size_t i = 0; i < coralNodes.size(); ++i) {
                float dx = a.x - coralNodes[i].x;
                float dy = a.y - coralNodes[i].y;
                float dist = std::sqrt(dx * dx + dy * dy);
                if (dist < minDist) {
                    minDist = dist;
                    closestIdx = static_cast<int>(i);
                }
            }

            if (closestIdx != -1 && minDist < currentConfig.max_dist) {
                attractor_influencing = true;
                float dx = a.x - coralNodes[closestIdx].x;
                float dy = a.y - coralNodes[closestIdx].y;
                if (minDist > 0.0f) {
                    coralNodes[closestIdx].dirX += dx / minDist;
                    coralNodes[closestIdx].dirY += dy / minDist;
                    coralNodes[closestIdx].count++;
                }
            }
        }

        if (!attractor_influencing) break;

        size_t current_size = coralNodes.size();
        std::vector<Node> newNodes;

        // Process growth expansions
        for (size_t i = 0; i < current_size; ++i) {
            if (coralNodes[i].count > 0) {
                float avgX = coralNodes[i].dirX / coralNodes[i].count;
                float avgY = coralNodes[i].dirY / coralNodes[i].count;

                float len = std::sqrt(avgX * avgX + avgY * avgY);
                if (len > 0.0f) { avgX /= len; avgY /= len; }

                // Inject upward tropism parameters
                avgY += currentConfig.upward_bias;
                len = std::sqrt(avgX * avgX + avgY * avgY);
                if (len > 0.0f) { avgX /= len; avgY /= len; }

                Node n;
                n.x = coralNodes[i].x + avgX * currentConfig.branch_length;
                n.y = coralNodes[i].y + avgY * currentConfig.branch_length;
                n.parentIndex = static_cast<int>(i);
                n.dirX = 0; n.dirY = 0; n.count = 0;

                newNodes.push_back(n);
            }
        }

        if (newNodes.empty()) break;
        coralNodes.insert(coralNodes.end(), newNodes.begin(), newNodes.end());

        // Pruning phase: evaluate reached attractors
        for (auto& a : attractors) {
            if (!a.active) continue;
            for (const auto& n : coralNodes) {
                float dx = a.x - n.x;
                float dy = a.y - n.y;
                if ((dx * dx + dy * dy) < (currentConfig.min_dist * currentConfig.min_dist)) {
                    a.active = false;
                    break;
                }
            }
        }
    }
}

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

	generate_coral(0);

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

        glLineWidth(currentConfig.line_width);
        glColor3f(currentConfig.color[0], currentConfig.color[1], currentConfig.color[2]);

        glBegin(GL_LINES);
        for (const auto& n : coralNodes) {
            if (n.parentIndex != -1) {
                const Node& parentNode = coralNodes[n.parentIndex];
                // Render flat 2D on the XY viewport plane, rising above y = 0
                glVertex3f(parentNode.x, parentNode.y, 0.0f);
                glVertex3f(n.x, n.y, 0.0f);
            }
        }
        glEnd();

        // --- Render Extra Textures/Points if Specified ---
        if (currentConfig.point_style > 0) {
            if (currentConfig.point_style == 1) { // Spotted Texture (e.g., Brain Coral)
                glPointSize(2.0f);
                glColor3f(0.1f, 0.1f, 0.1f);
            }
            else if (currentConfig.point_style == 2) { // Segmented Node Rings (e.g., Bamboo Coral)
                glPointSize(5.0f);
                glColor3f(0.05f, 0.15f, 0.22f);
            }

            glBegin(GL_POINTS);
            for (const auto& n : coralNodes) {
                glVertex3f(n.x, n.y, 0.001f); // Micro-offset Z to prevent depth fighting over the lines
            }
            glEnd();
        }

        // Swap front and back buffers
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}