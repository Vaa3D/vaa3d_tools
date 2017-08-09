#define _USE_MATH_DEFINES
#include "Sphere.h"

#include <cmath>



Sphere::Sphere(const float radius, const int slices, const int stacks) :radius(radius), slices(slices), stacks(stacks)
{
	Init();
}


Sphere::~Sphere()
{
}

GLenum Sphere::GetPrimitiveType()
{
	return GL_TRIANGLES;
}

void Sphere::fillVertexBuffer()
{
	//glm::vec3* vertices = (glm::vec3*)(pBuffer);

	float const R = 1.0f / (slices - 1);
	float const S = 1.0f / (stacks - 1);
	int count = 0;

	for (int r = 0; r < slices; ++r) {
		for (int s = 0; s < stacks; ++s) {
			float const y = (float)(sin(-M_PI_2 + M_PI * r * R));
			float const x = (float)(cos(2 * M_PI * s * S) * sin(M_PI * r  * R));
			float const z = (float)(sin(2 * M_PI * s * S) * sin(M_PI * r  * R));

			vertices.push_back(glm::vec3(x, y, z)*radius);
			//vertices[count++] = glm::vec3(x, y, z)*radius;
		}
	}
}

void Sphere::fillIndexBuffer()
{
	for (int r = 0; r < slices - 1; ++r) {
		for (int s = 0; s < stacks - 1; ++s) {
			int curRow = r * stacks;
			int nextRow = (r + 1) * stacks;
			indices.push_back(curRow + s);
			indices.push_back(nextRow + s);
			indices.push_back(nextRow + (s + 1));

			indices.push_back(curRow + s);
			indices.push_back(nextRow + (s + 1));
			indices.push_back(curRow + (s + 1));
		}
	}
}
