#ifndef CORAL_GENERATION_H
#define CORAL_GENERATION_H

#include <vector>
#include <glm/glm.hpp>

// --- Space Colonization Structures ---
struct Node {
	float x, y, z;
	int parentIndex;
	float dirX, dirY, dirZ;
	int count;
	bool alive = true;
};

struct Attractor {
	float x, y, z;
	bool active;
};

struct CoralConfig {
	int num_attractors;
	float min_dist;
	float max_dist;
	float branch_length;
	float upward_bias;
	float color[3];
	float branch_radius; // promień walca używanego do renderowania gałęzi
	float line_width;
	int point_style; // 0: None, 1: Spotted, 2: Segmented Nodes
};

struct CoralInstance {
	std::vector<Node> nodes;
	CoralConfig config;
	glm::vec3 position; // Miejsce na dnie (X, Y z bottomHeight, Z)
	float rotationY;
	unsigned int segmentVBO = 0; // GPU VBO with segment vertices (vec3 pairs)
	int segmentVertexCount = 0;
};

// Zewnętrzny dostęp do listy korali
extern std::vector<CoralInstance> coralReef;

// Generatory
CoralInstance generate_single_coral(int coral_type, glm::vec3 start_pos);
void generate_coral_reef(glm::vec2 min_bound, glm::vec2 max_bound, float density, int coral_type);

// Wczytuje losowy koral z pliku corals_<type>.json wygenerowanego przez program.
// Zwraca true gdy wczytanie powiodlo sie i wypelnia out.nodes oraz out.config (bez pozycji).
bool loadRandomCoralFromFile(int coralType, CoralInstance& out);

// Rysuje prosty walec przybliżony jako wielokąt między punktami a i b
void draw_cylinder_between(const glm::vec3& a, const glm::vec3& b, float radius, int segments = 12);

// Zapisuje "count" korali danego typu do pliku corals_<type>.json (count>=1).
// Funkcja używa generate_single_coral i zapisuje config + nodes (bez pozycji).
void writeCoralsOfType(int coralType, int count);

#endif // CORAL_GENERATION_H
