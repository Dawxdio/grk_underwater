#include "coral_generation.h"

#include <cmath>
#include <vector>
#include <cstdlib>
#include <GLFW/glfw3.h>
#include "ocean_floor.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Internal globals for generation
static std::vector<Node> coralNodes;
static CoralConfig currentConfig;

// Global scale to reduce total number of coral instances generated (1.0 = no change)
const float CORAL_DENSITY_SCALE = 0.4f; // zmniejsza ilość generowanych korali do 40%

// Definicja zewnętrznej listy korali
std::vector<CoralInstance> coralReef;

// Implementacja rysowania przybliżonego walca (używane podczas renderingu)
void draw_cylinder_between(const glm::vec3& a, const glm::vec3& b, float radius, int segments) {
	glm::vec3 dir = b - a;
	float len = glm::length(dir);
	if (len < 1e-6f) return;
	glm::vec3 z = glm::normalize(dir);

	glm::vec3 tmp = (fabs(z.x) < 0.0001f && fabs(z.y) < 0.0001f) ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 x = glm::normalize(glm::cross(tmp, z));
	glm::vec3 y = glm::normalize(glm::cross(z, x));

	glBegin(GL_TRIANGLE_STRIP);
	for (int i = 0; i <= segments; ++i) {
		float theta = (float)i / (float)segments * 2.0f * (float)M_PI;
		float c = cosf(theta);
		float s = sinf(theta);
		glm::vec3 offset = x * (c * radius) + y * (s * radius);

		glm::vec3 va = a + offset;
		glm::vec3 vb = b + offset;

		glVertex3f(va.x, va.y, va.z);
		glVertex3f(vb.x, vb.y, vb.z);
	}
	glEnd();
}

CoralInstance generate_single_coral(int coral_type, glm::vec3 start_pos) {
	CoralInstance coral;
	coral.position = start_pos;
	coralNodes.clear();
	std::vector<Attractor> attractors;

	// Define structural presets mimicking the reference profiles
	switch (coral_type) {
	case 0: // Staghorn Coral / Classic Branching
		currentConfig = { 500, 0.08f, 0.6f, 0.05f, 0.3f, {1.0f, 0.42f, 0.55f}, 0.02f, 2.0f, 0 };
		for (int i = 0; i < currentConfig.num_attractors; ++i) {
			float theta = (rand() / (float)RAND_MAX) * (float)M_PI;
			float r = 0.5f + (rand() / (float)RAND_MAX) * 3.0f;
			attractors.push_back({ r * cos(theta), r * sin(theta) + 0.2f, true });
		}
		break;

	case 1: // Sea Fan (Broad, flat, lacy growth layout)
		currentConfig = { 700, 0.04f, 0.4f, 0.03f, 0.4f, {0.17f, 0.79f, 0.84f}, 0.015f, 1.2f, 0 };
		for (int i = 0; i < currentConfig.num_attractors; ++i) {
			float x = ((rand() / (float)RAND_MAX) * 2.0f - 1.0f) * 2.5f;
			float y = 0.2f + (rand() / (float)RAND_MAX) * 3.5f;
			attractors.push_back({ x, y, true });
		}
		break;

	case 2: // Brain Coral (Convoluted, dense low-lying profile)
		currentConfig = { 800, 0.03f, 0.25f, 0.02f, 0.1f, {0.82f, 0.70f, 0.55f}, 0.035f, 3.5f, 1 };
		for (int i = 0; i < currentConfig.num_attractors; ++i) {
			float x = ((rand() / (float)RAND_MAX) * 2.0f - 1.0f) * 2.5f;
			float y = 0.1f + (rand() / (float)RAND_MAX) * 2.0f;
			attractors.push_back({ x, y, true });
		}
		break;

	case 3: // Brush Coral / Shelf Plates
		currentConfig = { 600, 0.10f, 0.7f, 0.06f, 0.6f, {1.0f, 0.8f, 0.5f}, 0.025f, 4.0f, 0 };
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
		currentConfig = { 250, 0.20f, 0.9f, 0.14f, 0.8f, {0.84f, 0.88f, 0.90f}, 0.035f, 1.5f, 2 };
		for (int i = 0; i < currentConfig.num_attractors; ++i) {
			int spire = rand() % 3;
			float x_base = -1.5f + spire * 1.5f;
			float x = x_base + ((rand() / (float)RAND_MAX) * 2.0f - 1.0f) * 0.2f;
			float y = 0.4f + (rand() / (float)RAND_MAX) * 3.8f;
			attractors.push_back({ x, y, true });
		}
		break;

	default: // Fallback to basic setup
		currentConfig = { 400, 0.1f, 0.5f, 0.05f, 0.2f, {1.0f, 1.0f, 1.0f}, 0.02f, 1.5f, 0 };
		break;
	}

	// Initialize root node at the coordinate origin on the floor
	Node root = { 0.0f, 0.0f, -1, 0.0f, 0.0f, 0 };
	coral.nodes.push_back(root);
	coralNodes.push_back(root);

	bool growing = true;
	int iterations = 0;
	int max_iterations = 1000; // Safety ceiling break

	while (growing && iterations < max_iterations) {
		iterations++;

		for (auto& n : coralNodes) {
			n.dirX = 0.0f;
			n.dirY = 0.0f;
			n.count = 0;
		}

		bool attractor_influencing = false;

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

		for (size_t i = 0; i < current_size; ++i) {
			if (coralNodes[i].count > 0 && coralNodes[i].alive) {

				float distFromStart = std::sqrt(coralNodes[i].x * coralNodes[i].x + coralNodes[i].y * coralNodes[i].y);

				float max_radius = 2.2f;

				float ratio = distFromStart / max_radius;
				if (ratio > 1.0f) ratio = 1.0f;

				float death_probability = std::pow(ratio, 4);

				float random_roll = (rand() / (float)RAND_MAX);

				if (random_roll < death_probability) {
					coralNodes[i].alive = false;
					continue;
				}
				float avgX = coralNodes[i].dirX / coralNodes[i].count;
				float avgY = coralNodes[i].dirY / coralNodes[i].count;

				float len = std::sqrt(avgX * avgX + avgY * avgY);
				if (len > 0.0f) { avgX /= len; avgY /= len; }

				avgY += currentConfig.upward_bias;
				len = std::sqrt(avgX * avgX + avgY * avgY);
				if (len > 0.0f) { avgX /= len; avgY /= len; }

				Node n;
				n.x = coralNodes[i].x + avgX * currentConfig.branch_length;
				n.y = coralNodes[i].y + avgY * currentConfig.branch_length;
				n.parentIndex = static_cast<int>(i);
				n.dirX = 0; n.dirY = 0; n.count = 0;
				n.alive = true; 

				newNodes.push_back(n);
			}
		}

		if (newNodes.empty()) break;
		coralNodes.insert(coralNodes.end(), newNodes.begin(), newNodes.end());

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


void generate_coral_reef(glm::vec2 min_bound, glm::vec2 max_bound, float density, int coral_type) {
	float area = (max_bound.x - min_bound.x) * (max_bound.y - min_bound.y);
	int number_of_corals = static_cast<int>(area * density * 0.5f * CORAL_DENSITY_SCALE);

	for (int i = 0; i < number_of_corals; ++i) {
		float x = min_bound.x + (rand() / (float)RAND_MAX) * (max_bound.x - min_bound.x);
		float z = min_bound.y + (rand() / (float)RAND_MAX) * (max_bound.y - min_bound.y);

		// Oblicz wysokość dna, aby koral ustawiony był na podłożu
		float y = oceanFloorHeight(x, z);

		CoralInstance new_coral = generate_single_coral(coral_type, glm::vec3(x, y, z));
		new_coral.rotationY = (rand() / (float)RAND_MAX) * 360.0f;
		coralReef.push_back(new_coral);
	}
}
