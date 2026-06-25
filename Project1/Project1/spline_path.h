#pragma once
#include <vector>
#include <cmath>
#include <glm/glm.hpp>

class SplinePath {
public:
    std::vector<glm::vec3> controlPoints;

    void addPoint(const glm::vec3& point) { controlPoints.push_back(point); }

    glm::vec3 getPosition(float globalT) {
        int p1_index = static_cast<int>(std::floor(globalT));
        float localT = globalT - std::floor(globalT);

        int p0_index = std::max(0, p1_index - 1);
        int p2_index = std::min((int)controlPoints.size() - 1, p1_index + 1);
        int p3_index = std::min((int)controlPoints.size() - 1, p2_index + 1);
        p1_index = std::max(0, std::min((int)controlPoints.size() - 1, p1_index));

        if (p1_index >= controlPoints.size() - 1) return controlPoints.back();

        float t2 = localT * localT;
        float t3 = t2 * localT;

		// Wzór Catmull-Rom do interpolacji między punktami kontrolnymi
        return 0.5f * (
            (2.0f * controlPoints[p1_index]) +
            (-controlPoints[p0_index] + controlPoints[p2_index]) * localT +
            (2.0f * controlPoints[p0_index] - 5.0f * controlPoints[p1_index] + 4.0f * controlPoints[p2_index] - controlPoints[p3_index]) * t2 +
            (-controlPoints[p0_index] + 3.0f * controlPoints[p1_index] - 3.0f * controlPoints[p2_index] + controlPoints[p3_index]) * t3
            );
    }

    glm::vec3 getTangent(float globalT) {
        int p1_index = static_cast<int>(std::floor(globalT));
        float localT = globalT - std::floor(globalT);

        int p0_index = std::max(0, p1_index - 1);
        int p2_index = std::min((int)controlPoints.size() - 1, p1_index + 1);
        int p3_index = std::min((int)controlPoints.size() - 1, p2_index + 1);
        p1_index = std::max(0, std::min((int)controlPoints.size() - 1, p1_index));

        float t2 = localT * localT;

		// Pochodna ze wzoru Catmull-Rom do obracania obiektu wzdłuż ścieżki
        glm::vec3 tangent = 0.5f * (
            (-controlPoints[p0_index] + controlPoints[p2_index]) +
            2.0f * (2.0f * controlPoints[p0_index] - 5.0f * controlPoints[p1_index] + 4.0f * controlPoints[p2_index] - controlPoints[p3_index]) * localT +
            3.0f * (-controlPoints[p0_index] + 3.0f * controlPoints[p1_index] - 3.0f * controlPoints[p2_index] + controlPoints[p3_index]) * t2
            );

        return glm::normalize(tangent);
    }
};
