#include "coral_generation.h"
#include "GL/glew.h"
#include <cmath>
#include <vector>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <cctype>
#include <iostream>
#include <iomanip>
#include <GLFW/glfw3.h>
#include "ocean_floor.h"


// Helper: build CPU-side segment info for coral (no GPU helpers)
void buildGpuSegmentsForCoral(CoralInstance& coral) {
	std::vector<PbrVertex> totalVertices;
	int segments = 8;
	float radius = coral.config.branch_radius;

	for (size_t i = 0; i < coral.nodes.size(); ++i) {
		const Node& n = coral.nodes[i];
		if (n.parentIndex == -1) continue;

		const Node& p = coral.nodes[n.parentIndex];
		glm::vec3 a(p.x, p.y, p.z);
		glm::vec3 b(n.x, n.y, n.z);

		glm::vec3 dir = b - a;
		float len = glm::length(dir);
		if (len < 1e-6f) continue;
		glm::vec3 z = glm::normalize(dir);

		glm::vec3 tmp = (fabs(z.x) < 0.0001f && fabs(z.y) < 0.0001f) ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(0.0f, 0.0f, 1.0f);
		glm::vec3 x = glm::normalize(glm::cross(tmp, z));
		glm::vec3 y = glm::normalize(glm::cross(z, x));

		for (int j = 0; j < segments; ++j) {
			float theta1 = (float)j / (float)segments * 2.0f * 3.14159265f;
			float theta2 = (float)(j + 1) / (float)segments * 2.0f * 3.14159265f;

			glm::vec3 norm1 = glm::normalize(x * cosf(theta1) + y * sinf(theta1));
			glm::vec3 norm2 = glm::normalize(x * cosf(theta2) + y * sinf(theta2));

			glm::vec3 va1 = a + norm1 * radius;
			glm::vec3 va2 = a + norm2 * radius;
			glm::vec3 vb1 = b + norm1 * radius;
			glm::vec3 vb2 = b + norm2 * radius;

			// Trójkąt 1 (va1 -> vb1 -> va2)
			totalVertices.push_back({ va1, norm1, glm::vec2(0.0f) });
			totalVertices.push_back({ vb1, norm1, glm::vec2(0.0f) });
			totalVertices.push_back({ va2, norm2, glm::vec2(0.0f) });

			// Trójkąt 2 (va2 -> vb1 -> vb2)
			totalVertices.push_back({ va2, norm2, glm::vec2(0.0f) });
			totalVertices.push_back({ vb1, norm1, glm::vec2(0.0f) });
			totalVertices.push_back({ vb2, norm2, glm::vec2(0.0f) });
		}
	}

	if (totalVertices.empty()) {
		coral.segmentVBO = 0;
		coral.segmentVertexCount = 0;
		return;
	}

	// Usunięcie starych buforów zapobiega wyciekom pamięci
	if (coral.segmentVBO != 0) {
		glDeleteBuffers(1, &coral.segmentVBO);
	}

	glGenBuffers(1, &coral.segmentVBO);
	glBindBuffer(GL_ARRAY_BUFFER, coral.segmentVBO);
	glBufferData(GL_ARRAY_BUFFER, totalVertices.size() * sizeof(PbrVertex), totalVertices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	coral.segmentVertexCount = (int)totalVertices.size();
}

void renderCoralPBR(const CoralInstance& coral, GLuint pbrShaderProgram) {
	if (coral.segmentVBO == 0 || coral.segmentVertexCount == 0) return;

	glUseProgram(pbrShaderProgram);

	// Przekaż do Shadera parametry Metallic i Roughness (np. z konfiguracji lub stałe)
	glUniform3fv(glGetUniformLocation(pbrShaderProgram, "albedo"), 1, coral.config.color);
	glUniform1f(glGetUniformLocation(pbrShaderProgram, "metallic"), 0.1f);   // Korale raczej nie są metaliczne
	glUniform1f(glGetUniformLocation(pbrShaderProgram, "roughness"), 0.8f);  // Za to są mocno chropowate

	// Bindowanie bufora
	glBindBuffer(GL_ARRAY_BUFFER, coral.segmentVBO);

	// Linkowanie pozycji (Location 0 w vertex shaderze)
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(PbrVertex), (void*)offsetof(PbrVertex, position));

	// Linkowanie normalnych (Location 1 w vertex shaderze - KLUCZOWE DLA PBR!)
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(PbrVertex), (void*)offsetof(PbrVertex, normal));

	// Renderowanie
	glDrawArrays(GL_TRIANGLES, 0, coral.segmentVertexCount);

	// Czyszczenie stanu
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
// Zapisuje wygenerowane korale do pliku corals_<type>.json (count elementów)
void writeCoralsOfType(int coralType, int count) {
	if (count < 1) count = 1;
	std::string filename = "corals_" + std::to_string(coralType) + ".json";
	std::ofstream f(filename);
	if (!f) {
		std::cerr << "Nie mozna utworzyc pliku: " << filename << std::endl;
		return;
	}
	f << std::fixed << std::setprecision(6);
	f << "{\n  \"version\": 1,\n  \"coralsByType\": {\n    \"" << coralType << "\": [\n";

	for (int i = 0; i < count; ++i) {
		CoralInstance c = generate_single_coral(coralType, glm::vec3(0.0f));
		c.position = glm::vec3(0.0f);
		f << "      {\n";
		f << "        \"type\": " << coralType << ",\n";
		// config
		f << "        \"config\": {\n";
		f << "          \"num_attractors\": " << c.config.num_attractors << ",\n";
		f << "          \"min_dist\": " << c.config.min_dist << ",\n";
		f << "          \"max_dist\": " << c.config.max_dist << ",\n";
		f << "          \"branch_length\": " << c.config.branch_length << ",\n";
		f << "          \"upward_bias\": " << c.config.upward_bias << ",\n";
		f << "          \"color\": [" << c.config.color[0] << ", " << c.config.color[1] << ", " << c.config.color[2] << "],\n";
		f << "          \"branch_radius\": " << c.config.branch_radius << ",\n";
		f << "          \"line_width\": " << c.config.line_width << ",\n";
		f << "          \"point_style\": " << c.config.point_style << "\n";
		f << "        },\n";

		// nodes
		f << "        \"nodes\": [\n";
		for (size_t ni = 0; ni < c.nodes.size(); ++ni) {
			const Node& n = c.nodes[ni];
			f << "          { \"x\": " << n.x << ", \"y\": " << n.y << ", \"z\": " << n.z << ", \"parentIndex\": " << n.parentIndex << ", \"alive\": " << (n.alive ? "true" : "false") << " }";
			if (ni + 1 < c.nodes.size()) f << ",\n";
			else f << "\n";
		}
		f << "        ]\n";
		f << "      }";
		if (i + 1 < count) f << ",\n";
		else f << "\n";
	}

	f << "    ]\n  }\n}\n";
	f.close();
	std::cout << "Zapisano " << count << " korali typu " << coralType << " do pliku: " << filename << std::endl;
}

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Internal globals for generation
static std::vector<Node> coralNodes;
static CoralConfig currentConfig;

// Configure currentConfig for a given coral type (shared helper)
static void configure_config(int coral_type) {
	switch (coral_type) {
	case 0: // Staghorn - zamiast jaskrawego różu: głęboka, ciemna purpura/biskupia czerwień
		currentConfig = { 500, 0.08f, 0.6f, 0.05f, 0.3f, {0.45f, 0.12f, 0.28f}, 0.02f, 2.0f, 0 };
		break;
	case 1: // Sea Fan - zamiast jasnego turkusowego: głęboki, ciemny cyjan / morski granat
		currentConfig = { 700, 0.04f, 0.4f, 0.03f, 0.4f, {0.05f, 0.32f, 0.40f}, 0.015f, 1.2f, 0 };
		break;
	case 2: // Brain - zamiast piaskowego: ciemny, oliwkowy brąz / zgniła zieleń (bardzo naturalna dla koralowców)
		currentConfig = { 800, 0.03f, 0.25f, 0.02f, 0.1f, {0.28f, 0.35f, 0.18f}, 0.035f, 3.5f, 1 };
		break;
	case 3: // Typ 3 - zamiast jasnego żółtego: ciepły, ciemny bursztyn / głęboka musztarda
		currentConfig = { 600, 0.10f, 0.7f, 0.06f, 0.6f, {0.52f, 0.34f, 0.15f}, 0.025f, 4.0f, 0 };
		break;
	case 4: // Typ 4 - zamiast tej bieli (0.84, 0.88, 0.90): ciemny, stonowany fiolet indygo / szary fiolet
		currentConfig = { 250, 0.20f, 0.9f, 0.14f, 0.8f, {0.25f, 0.20f, 0.38f}, 0.035f, 1.5f, 2 };
		break;
	default:
		currentConfig = { 400, 0.1f, 0.5f, 0.05f, 0.2f, {0.3f, 0.3f, 0.3f}, 0.02f, 1.5f, 0 };
		break;
	}
}

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

bool loadRandomCoralFromFile(int coralType, CoralInstance& out) {
	// Use numeric type in filename even for negative types (e.g. corals_-1.json)
	std::string key = std::to_string(coralType);
	std::string filename = "corals_" + std::to_string(coralType) + ".json";
	std::ifstream f(filename);
	if (!f) return false;
	std::stringstream ss; ss << f.rdbuf();
	std::string s = ss.str();
	// find the array for this type
	std::string typeKey = std::string("\"") + key + "\":";
	size_t pos = s.find(typeKey);
	if (pos == std::string::npos) return false;
	pos = s.find('[', pos);
	if (pos == std::string::npos) return false;
	// extract top-level objects in the array
	std::vector<std::pair<size_t, size_t>> objs;
	bool inObject = false;
	int braceDepth = 0;
	size_t objStart = 0;
	for (size_t i = pos + 1; i < s.size(); ++i) {
		char c = s[i];
		if (c == '{') {
			if (!inObject) { inObject = true; objStart = i; braceDepth = 1; }
			else braceDepth++;
		}
		else if (c == '}') {
			if (inObject) {
				braceDepth--;
				if (braceDepth == 0) {
					size_t objEnd = i;
					objs.emplace_back(objStart, objEnd);
					inObject = false;
				}
			}
		}
		else if (c == ']' && !inObject) {
			break; // end of array
		}
	}
	if (objs.empty()) return false;
	size_t pick = (size_t)(std::rand() / (float)RAND_MAX * (float)objs.size());
	if (pick >= objs.size()) pick = objs.size() - 1;
	std::string obj = s.substr(objs[pick].first, objs[pick].second - objs[pick].first + 1);

	auto extractNumber = [&](const std::string& hay, const std::string& key)->std::string {
		std::string q = std::string("\"") + key + "\"";
		size_t p = hay.find(q);
		if (p == std::string::npos) return std::string();
		p = hay.find(':', p);
		if (p == std::string::npos) return std::string();
		++p;
		// read until comma or closing brace/bracket
		while (p < hay.size() && isspace((unsigned char)hay[p])) ++p;
		size_t e = p;
		while (e < hay.size() && hay[e] != ',' && hay[e] != '}' && hay[e] != ']') ++e;
		std::string token = hay.substr(p, e - p);
		// trim
		size_t a = token.find_first_not_of(" \t\n\r");
		size_t b = token.find_last_not_of(" \t\n\r");
		if (a == std::string::npos) return std::string();
		return token.substr(a, b - a + 1);
		};

	auto parseFloat = [&](const std::string& hay, const std::string& k)->float {
		std::string t = extractNumber(hay, k);
		if (t.empty()) return 0.0f;
		try { return std::stof(t); }
		catch (...) { return 0.0f; }
		};
	auto parseInt = [&](const std::string& hay, const std::string& k)->int {
		std::string t = extractNumber(hay, k);
		if (t.empty()) return 0;
		try { return std::stoi(t); }
		catch (...) { return 0; }
		};

	// parse config
	CoralConfig cfg{};
	cfg.num_attractors = parseInt(obj, "num_attractors");
	cfg.min_dist = parseFloat(obj, "min_dist");
	cfg.max_dist = parseFloat(obj, "max_dist");
	cfg.branch_length = parseFloat(obj, "branch_length");
	cfg.upward_bias = parseFloat(obj, "upward_bias");
	cfg.branch_radius = parseFloat(obj, "branch_radius");
	cfg.line_width = parseFloat(obj, "line_width");
	cfg.point_style = parseInt(obj, "point_style");
	// color array
	size_t colPos = obj.find("\"color\"");
	if (colPos != std::string::npos) {
		size_t b = obj.find('[', colPos);
		size_t e = obj.find(']', b == std::string::npos ? colPos : b);
		if (b != std::string::npos && e != std::string::npos) {
			std::string inside = obj.substr(b + 1, e - b - 1);
			std::stringstream cs(inside);
			float r, g, bv; char comma;
			if (cs >> r) {
				if (cs.peek() == ',') cs >> comma;
				if (cs >> g) { if (cs.peek() == ',') cs >> comma; if (cs >> bv) { cfg.color[0] = r; cfg.color[1] = g; cfg.color[2] = bv; } }
			}
		}
	}

	// parse nodes array
	std::vector<Node> nodes;
	size_t nodesPos = obj.find("\"nodes\"");
	if (nodesPos != std::string::npos) {
		size_t arrStart = obj.find('[', nodesPos);
		if (arrStart != std::string::npos) {
			// find node objects inside
			bool inObj = false; int bd = 0; size_t nstart = 0;
			for (size_t i = arrStart + 1; i < obj.size(); ++i) {
				char c = obj[i];
				if (c == '{') {
					if (!inObj) { inObj = true; nstart = i; bd = 1; }
					else bd++;
				}
				else if (c == '}') {
					if (inObj) {
						bd--; if (bd == 0) {
							size_t nend = i; std::string no = obj.substr(nstart, nend - nstart + 1);
							Node n{}; n.x = parseFloat(no, "x"); n.y = parseFloat(no, "y"); n.z = parseFloat(no, "z"); n.parentIndex = parseInt(no, "parentIndex"); n.dirX = n.dirY = n.dirZ = 0.0f; n.count = 0; std::string al = extractNumber(no, "alive"); n.alive = (al == "true"); nodes.push_back(n); inObj = false;
						}
					}
				}
				else if (c == ']') {
					break;
				}
			}
		}
	}

	if (nodes.empty()) return false;

	out.nodes = nodes;
	out.config = cfg;
	out.segmentVBO = 0; out.segmentVertexCount = 0; out.rotationY = 0.0f;
	return true;
}

CoralInstance generate_single_coral(int coral_type, glm::vec3 start_pos) {
	CoralInstance coral;
	coral.position = start_pos;

	const int minNodesRequired = 10; // minimal akceptowalna liczba wezlow
	const int maxAttempts = 5;

	for (int attempt = 0; attempt < maxAttempts; ++attempt) {
		coralNodes.clear();
		// ensure currentConfig is initialized for this coral type (needed for GPU path)
		configure_config(coral_type);
		std::vector<Attractor> attractors;

		// Define structural presets mimicking the reference profiles
		switch (coral_type) {
		case 0: // Staghorn Coral / Classic Branching
			currentConfig = { 500, 0.08f, 0.6f, 0.05f, 0.3f, {1.0f, 0.42f, 0.55f}, 0.02f, 2.0f, 0 };
			for (int i = 0; i < currentConfig.num_attractors; ++i) {
				float theta = (rand() / (float)RAND_MAX) * (float)M_PI;
				float r = 0.5f + (rand() / (float)RAND_MAX) * 3.0f;
				float az = ((rand() / (float)RAND_MAX) * 2.0f - 1.0f) * 0.6f;
				attractors.push_back({ r * cos(theta), r * sin(theta) + 0.2f, az, true });
			}
			break;

		case 1: // Sea Fan (Broad, flat, lacy growth layout)
			currentConfig = { 700, 0.04f, 0.4f, 0.03f, 0.4f, {0.17f, 0.79f, 0.84f}, 0.015f, 1.2f, 0 };
			for (int i = 0; i < currentConfig.num_attractors; ++i) {
				float x = ((rand() / (float)RAND_MAX) * 2.0f - 1.0f) * 2.5f;
				float y = 0.2f + (rand() / (float)RAND_MAX) * 3.5f;
				float zz = ((rand() / (float)RAND_MAX) * 2.0f - 1.0f) * 0.6f;
				attractors.push_back({ x, y, zz, true });
			}
			break;

		case 2: // Brain Coral (Convoluted, dense low-lying profile)
			currentConfig = { 800, 0.03f, 0.25f, 0.02f, 0.1f, {0.82f, 0.70f, 0.55f}, 0.035f, 3.5f, 1 };
			for (int i = 0; i < currentConfig.num_attractors; ++i) {
				float x = ((rand() / (float)RAND_MAX) * 2.0f - 1.0f) * 2.5f;
				float y = 0.1f + (rand() / (float)RAND_MAX) * 2.0f;
				float zz = ((rand() / (float)RAND_MAX) * 2.0f - 1.0f) * 0.4f;
				attractors.push_back({ x, y, zz, true });
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
				float zz = ((rand() / (float)RAND_MAX) * 2.0f - 1.0f) * 0.5f + layer * 0.2f;
				attractors.push_back({ x, y, zz, true });
			}
			break;

		case 4: // Bamboo Coral (Segmented straight vertical nodes)
			currentConfig = { 250, 0.20f, 0.9f, 0.14f, 0.8f, {0.84f, 0.88f, 0.90f}, 0.035f, 1.5f, 2 };
			for (int i = 0; i < currentConfig.num_attractors; ++i) {
				int spire = rand() % 3;
				float x_base = -1.5f + spire * 1.5f;
				float x = x_base + ((rand() / (float)RAND_MAX) * 2.0f - 1.0f) * 0.2f;
				float y = 0.4f + (rand() / (float)RAND_MAX) * 3.8f;
				float zz = ((rand() / (float)RAND_MAX) * 2.0f - 1.0f) * 0.3f + spire * 0.2f;
				attractors.push_back({ x, y, zz, true });
			}
			break;

		default: // Fallback to basic setup
			currentConfig = { 400, 0.1f, 0.5f, 0.05f, 0.2f, {1.0f, 1.0f, 1.0f}, 0.02f, 1.5f, 0 };
			// Default: generate a modest cloud of attractors in 3D so fallback corals grow
			for (int i = 0; i < currentConfig.num_attractors; ++i) {
				float ang = (rand() / (float)RAND_MAX) * 2.0f * (float)M_PI;
				float r = ((rand() / (float)RAND_MAX) * 1.0f) + 0.5f;
				float x = r * cos(ang);
				float y = 0.1f + (rand() / (float)RAND_MAX) * 2.0f;
				float z = ((rand() / (float)RAND_MAX) * 2.0f - 1.0f) * 0.6f;
				attractors.push_back({ x, y, z, true });
			}
			break;
		}

		// Initialize root node at the coordinate origin on the floor

		// Safety: ensure there are some attractors close enough to the root so growth can start.
		{
			bool anyNearby = false;
			for (const auto& a : attractors) {
				float dx = a.x - 0.0f;
				float dy = a.y - 0.0f;
				float dz = a.z - 0.0f;
				float dist = std::sqrt(dx * dx + dy * dy + dz * dz);
				if (dist < currentConfig.max_dist * 1.2f) { anyNearby = true; break; }
			}
			if (!anyNearby) {
				// add a few nearby attractors to guarantee initial growth
				for (int i = 0; i < 8; ++i) {
					float ang = (rand() / (float)RAND_MAX) * 2.0f * (float)M_PI;
					float r = 0.08f + (rand() / (float)RAND_MAX) * (currentConfig.max_dist * 0.9f);
					float ax = r * cos(ang);
					float ay = 0.1f + (rand() / (float)RAND_MAX) * 0.4f;
					float az = ((rand() / (float)RAND_MAX) * 2.0f - 1.0f) * 0.2f;
					attractors.push_back({ ax, ay, az, true });
				}
			}
		}

		Node root = { 0.0f, 0.0f, 0.0f, -1, 0.0f, 0.0f, 0.0f, 0 };
		// Try GPU generation path first
		// Prepare attractors and initial node data in GPU-friendly layout

		struct GpuNode { float x; float y; float z; int parentIndex; float dirX; float dirY; float dirZ; int count; int alive; };
		struct GpuAttractor { float x; float y; float z; int active; };

		GpuNode groot = { 0.0f, 0.0f, 0.0f, -1, 0.0f, 0.0f, 0.0f, 0, 1 };

		// Prepare GPU attractor array
		std::vector<GpuAttractor> gAttractors;
		gAttractors.reserve(attractors.size());
		for (const auto& a : attractors) {
			// small random z-offset already included in attractor.z if present
			GpuAttractor ga = { a.x, a.y, a.z, a.active ? 1 : 0 };
			gAttractors.push_back(ga);
		}

		// GPU path wyłączony — zawsze używamy CPU fallback dla tej wersji
		bool gpuSuccess = false;

		// (oryginalna ścieżka GPU usunięta; brak zależności na funkcjach pomocniczych OpenGL)

		if (gpuSuccess) {
			// Do not populate CPU-side node list; renderer will use GPU VBO
			coral.config = currentConfig;
			// accept GPU result even if node count is small
			break;
		}

		// Fallback CPU generation (existing implementation)
		coral.nodes.push_back(root);
		coralNodes.push_back(root);

		// Seed: dodaj kilka poczatkowych węzłów blisko korzenia, aby uniknąć sytuacji, gdy
		// żadne attractory nie wpływają na root i wzrost nie startuje.
		if (currentConfig.branch_length > 1e-6f) {
			Node s1 = { 0.0f + 0.0f, currentConfig.branch_length * 0.9f, 0.0f + 0.0f, 0, 0.0f, 0.0f, 0.0f, 0 };
			Node s2 = { currentConfig.branch_length * 0.6f, currentConfig.branch_length * 0.8f, ((rand() / (float)RAND_MAX) * 2.0f - 1.0f) * 0.2f, 0, 0.0f, 0.0f, 0.0f, 0 };
			Node s3 = { -currentConfig.branch_length * 0.6f, currentConfig.branch_length * 0.8f, ((rand() / (float)RAND_MAX) * 2.0f - 1.0f) * 0.2f, 0, 0.0f, 0.0f, 0.0f, 0 };
			// parentIndex ustawiony na 0 (root)
			s1.parentIndex = 0; s2.parentIndex = 0; s3.parentIndex = 0;
			coralNodes.push_back(s1);
			coralNodes.push_back(s2);
			coralNodes.push_back(s3);
		}

		bool growing = true;
		int iterations = 0;
		int max_iterations = 1000; // Safety ceiling break

		while (growing && iterations < max_iterations) {
			iterations++;

			for (auto& n : coralNodes) {
				n.dirX = 0.0f;
				n.dirY = 0.0f;
				n.dirZ = 0.0f;
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
					float dz = a.z - coralNodes[i].z;
					float dist = std::sqrt(dx * dx + dy * dy + dz * dz);
					if (dist < minDist) {
						minDist = dist;
						closestIdx = static_cast<int>(i);
					}
				}

				if (closestIdx != -1 && minDist < currentConfig.max_dist) {
					attractor_influencing = true;
					float dx = a.x - coralNodes[closestIdx].x;
					float dy = a.y - coralNodes[closestIdx].y;
					float dz = a.z - coralNodes[closestIdx].z;
					if (minDist > 0.0f) {
						coralNodes[closestIdx].dirX += dx / minDist;
						coralNodes[closestIdx].dirY += dy / minDist;
						coralNodes[closestIdx].dirZ += dz / minDist;
						coralNodes[closestIdx].count++;
					}
				}
			}

			if (!attractor_influencing) break;

			size_t current_size = coralNodes.size();
			std::vector<Node> newNodes;

			for (size_t i = 0; i < current_size; ++i) {
				if (coralNodes[i].count > 0 && coralNodes[i].alive) {

					float distFromStart = std::sqrt(coralNodes[i].x * coralNodes[i].x + coralNodes[i].y * coralNodes[i].y + coralNodes[i].z * coralNodes[i].z);

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
					float avgZ = coralNodes[i].dirZ / coralNodes[i].count;

					float len = std::sqrt(avgX * avgX + avgY * avgY + avgZ * avgZ);
					if (len > 0.0f) { avgX /= len; avgY /= len; avgZ /= len; }

					avgY += currentConfig.upward_bias;
					len = std::sqrt(avgX * avgX + avgY * avgY + avgZ * avgZ);
					if (len > 0.0f) { avgX /= len; avgY /= len; avgZ /= len; }

					Node n;
					n.x = coralNodes[i].x + avgX * currentConfig.branch_length;
					n.y = coralNodes[i].y + avgY * currentConfig.branch_length;
					n.z = coralNodes[i].z + avgZ * currentConfig.branch_length;
					n.parentIndex = static_cast<int>(i);
					n.dirX = 0; n.dirY = 0; n.dirZ = 0; n.count = 0;
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
					float dz = a.z - n.z;
					if ((dx * dx + dy * dy + dz * dz) < (currentConfig.min_dist * currentConfig.min_dist)) {
						a.active = false;
						break;
					}
				}
			}
		}
		coral.nodes = coralNodes;
		coral.config = currentConfig;

		// jeśli mamy wystarczającą liczbę węzłów, zaakceptuj wynik
		if ((int)coral.nodes.size() >= minNodesRequired) break;

		// jeśli to ostatnia próba, zaakceptuj cokolwiek i wyjdź
		if (attempt == maxAttempts - 1) break;

		// w przeciwnym razie spróbuj wygenerować koral ponownie (inne losowe attractory/seed)
	}

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

		// Wczytaj losowy koral z pliku corals_<type>.json zamiast generowania proceduralnego
		CoralInstance new_coral;
		bool loaded = loadRandomCoralFromFile(coral_type, new_coral);
		if (!loaded) {
			std::cerr << "Brak pliku lub blad parsowania dla typu korala: " << coral_type << ". Pomijam." << std::endl;
			continue; // pomiń ten egzemplarz zamiast generować proceduralnie
		}

		// Ustaw pozycję/rotację na dnie w wyliczonym miejscu
		new_coral.position = glm::vec3(x, y, z);
		new_coral.rotationY = (rand() / (float)RAND_MAX) * 360.0f;

		// Nie tworzymy VBO — renderer korzysta z węzłów CPU-side
		new_coral.segmentVBO = 0;
		new_coral.segmentVertexCount = (int)(new_coral.nodes.size() * 2);

		coralReef.push_back(new_coral);
	}
}
