#include "ocean_floor.h"
#include <GLFW/glfw3.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

float oceanFloorHeight(float x, float z)
{
	float r =
		sqrtf((x * 0.8f) * (x * 0.8f) +
			(z * 1.2f) * (z * 1.2f));

	const float lakeRadius = 35.0f;
	const float shoreRadius = 50.0f;

	float height = 0.0f;

	if (r <= lakeRadius)
	{
		height =
			-3.0f * cosf((r / lakeRadius) * (float)M_PI / 2.0f) - 3.0f;

		height +=
			0.4f * sinf(x * 0.15f) +
			0.3f * cosf(z * 0.12f);

		float dx1 = x + 12.0f;
		float dz1 = z - 8.0f;
		float pit1 = -2.5f * expf(-(dx1 * dx1 + dz1 * dz1) / 80.0f);

		float dx2 = x - 15.0f;
		float dz2 = z + 10.0f;
		float pit2 = -1.8f * expf(-(dx2 * dx2 + dz2 * dz2) / 120.0f);

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

void generate_ocean_floor()
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
				oceanFloorHeight((float)x, (float)z),
				(float)z
			);

			glVertex3f(
				(float)x,
				oceanFloorHeight((float)x, (float)(z + 1)),
				(float)(z + 1)
			);
		}

		glEnd();
	}
}
