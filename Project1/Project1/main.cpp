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

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// --- Space Colonization Structures ---
struct Node {
    float x, y;
    int parentIndex;
    float dirX, dirY;
    int count;
    bool alive = true;
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

struct CoralInstance {
    std::vector<Node> nodes;
    CoralConfig config;
    glm::vec3 position; // Miejsce na dnie (X, Y z bottomHeight, Z)
    float rotationY;
};

// Globalna lista wszystkich korali na scenie
std::vector<CoralInstance> coralReef;

// --- Space Colonization Generation Function ---
CoralInstance generate_single_coral(int coral_type, glm::vec3 start_pos) {
    CoralInstance coral;
    coral.position = start_pos;
    coralNodes.clear();
    std::vector<Attractor> attractors;

    // Seed random number generator

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
        currentConfig = { 800, 0.03f, 0.25f, 0.02f, 0.1f, {0.82f, 0.70f, 0.55f}, 3.5f, 1 }; // Zwiększyłem num_attractors do 800 dla gęstości
        for (int i = 0; i < currentConfig.num_attractors; ++i) {
            float x = ((rand() / (float)RAND_MAX) * 2.0f - 1.0f) * 2.5f; // większy zasięg x
            float y = 0.1f + (rand() / (float)RAND_MAX) * 2.0f;          // większy zasięg y
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
    coral.nodes.push_back(root); // Dla zwracanego obiektu
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
            if (coralNodes[i].count > 0 && coralNodes[i].alive) {

                // 1. Obliczamy odległość od środka
                float distFromStart = std::sqrt(coralNodes[i].x * coralNodes[i].x + coralNodes[i].y * coralNodes[i].y);

                // 2. Zwiększamy maksymalny promień (koral będzie mógł być większy)
                float max_radius = 2.2f;

                // Podstawa prawdopodobieństwa (od 0.0 do 1.0)
                float ratio = distFromStart / max_radius;
                if (ratio > 1.0f) ratio = 1.0f;

                // --- ZMIANA: Podnosimy ratio do potęgi (np. 4 lub 5) ---
                // Dzięki temu dla ratio = 0.5 (połowa drogi), szansa na śmierć to 0.5^4 = 0.06 (czyli tylko 6% zamiast 50%!)
                float death_probability = std::pow(ratio, 4);

                // Losowanie
                float random_roll = (rand() / (float)RAND_MAX);

                if (random_roll < death_probability) {
                    coralNodes[i].alive = false;
                    continue;
                }
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
                n.alive = true; // Nowy węzeł domyślnie zaczyna jako żywy

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
    coral.nodes = coralNodes;
    coral.config = currentConfig;
	return coral;
}


// Movement implementation moved to movement.h / movement.cpp

// Generacja podłogi
float bottomHeight(float x, float z)
{
    float r =
        sqrtf((x * 0.8f) * (x * 0.8f) +
            (z * 1.2f) * (z * 1.2f));

    const float lakeRadius = 35.0f;
    const float shoreRadius = 50.0f;

    // Podstawowa misa jeziora
    float height = 0.0f;

    if (r <= lakeRadius)
    {
        height =
            -3.0f * cosf((r / lakeRadius) * (float)M_PI / 2.0f)-3.0f;

        // Małe nierówności
        height +=
            0.4f * sinf(x * 0.15f) +
            0.3f * cosf(z * 0.12f);

        // Głębsza część 1
        float dx1 = x + 12.0f;
        float dz1 = z - 8.0f;
        float pit1 = -2.5f * expf(-(dx1 * dx1 + dz1 * dz1) / 80.0f);

        // Głębsza część 2
        float dx2 = x - 15.0f;
        float dz2 = z + 10.0f;
        float pit2 = -1.8f * expf(-(dx2 * dx2 + dz2 * dz2) / 120.0f);

        // Rów podwodny
        float trench =
            -1.2f * expf(-((x + 5.0f) * (x + 5.0f)) / 30.0f);

        height += pit1 + pit2 + trench;

        return height;
    }

    if (r <= shoreRadius)
    {
        float t = (r - lakeRadius) / (shoreRadius - lakeRadius);

        height =
            4.0f * sinf(t * (float)M_PI / 2.0f);

        // Nierówne brzegi
        height +=
            0.6f * sinf(x * 0.1f) +
            0.5f * cosf(z * 0.08f);

        return height;
    }

    return
        4.0f +
        0.8f * sinf(x * 0.08f) +
        0.7f * cosf(z * 0.06f);
}

void generate_floor()
{
    glColor3f(0.55f, 0.40f, 0.20f);

    const int size = 50;

    for (int z = -size; z < size; z++)
    {
        glBegin(GL_QUAD_STRIP);

        for (int x = -size; x <= size; x++)
        {
            glVertex3f(
                (float)x,
                bottomHeight((float)x, (float)z),
                (float)z
            );

            glVertex3f(
                (float)x,
                bottomHeight((float)x, (float)(z + 1)),
                (float)(z + 1)
            );
        }

        glEnd();
    }
}

void generate_coral_reef(glm::vec2 min_bound, glm::vec2 max_bound, float density, int coral_type) {
    // Przelicz gęstość na realną liczbę korali w danym obszarze
    float area = (max_bound.x - min_bound.x) * (max_bound.y - min_bound.y);
    int number_of_corals = static_cast<int>(area * density * 0.5f); // Współczynnik dopasowania

    for (int i = 0; i < number_of_corals; ++i) {
        // 1. Wylosuj pozycję X i Z wewnątrz zadanego kwadratu
        float x = min_bound.x + (rand() / (float)RAND_MAX) * (max_bound.x - min_bound.x);
        float z = min_bound.y + (rand() / (float)RAND_MAX) * (max_bound.y - min_bound.y);

        // 2. Pobierz wysokość dna w tym miejscu, żeby koral nie wisiał w powietrzu
        float y = bottomHeight(x, z);

        // 3. Wygeneruj koral i dodaj do rafy
        CoralInstance new_coral = generate_single_coral(coral_type, glm::vec3(x, y, z));
        new_coral.rotationY = (rand() / (float)RAND_MAX) * 360.0f;
        coralReef.push_back(new_coral);
    }
}

int main() {
    auto programStart = std::chrono::high_resolution_clock::now();

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

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

            // Rysujemy linie (szkielet) korala
            glBegin(GL_LINES);
            for (const auto& n : coral.nodes) {
                if (n.parentIndex != -1) {
                    const Node& parentNode = coral.nodes[n.parentIndex];
                    glVertex3f(parentNode.x, parentNode.y, 0.0f);
                    glVertex3f(n.x, n.y, 0.0f);
                }
            }
            glEnd();

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
                    glVertex3f(n.x, n.y, 0.001f);
                }
                glEnd();
            }

            glPopMatrix(); // Przywracamy macierz, żeby kolejny koral rysował się od czystej pozycji
        }

        generate_floor();

        // Swap front and back buffers
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}