#define _USE_MATH_DEFINES
#include <cmath>

#include "Cylinder.h"


Cylinder::Cylinder(const float topRadius, const float bottomRadius, const float height, const int slices, const int stacks) :
	topRadius(topRadius),
	bottomRadius(bottomRadius),
	height(height),
	slices(slices),
	stacks(stacks)
{
	Init();
}

Cylinder::~Cylinder()
{
}

GLenum Cylinder::GetPrimitiveType()
{
	return GL_TRIANGLES;
}

void Cylinder::fillVertexBuffer()
{
	//glm::vec3* vertices = (glm::vec3*)(pBuffer);

	float stackHeight = height / (stacks - 1);
	float radiusStep = (topRadius - bottomRadius) / (stacks - 1);
	int count = 0;

	for (int i = 0; i <stacks; i++) {
		float y = -0.5f*height + i*stackHeight;
		float r = bottomRadius + i*radiusStep;
		float dTheta = float(2.0f*M_PI) / (slices - 1);//slice = 5,dTheta=PI/2
		for (int j = 0; j < slices; j++) {//j=0,1,2,3,4
			float c = cos(j*dTheta);//angle=0,PI/2,PI,3PI/2,2PI, duplicate last
			float s = sin(j*dTheta);

			//vertices[count++] = glm::vec3(r*c, y, r*s);
			vertices.push_back(glm::vec3(r*c, y, r*s));
		}
	}

	//top cap
	float y = 0.5f*height;
	float dTheta = float(2.0f*M_PI) / (slices - 1);

	for (int i = 0; i < slices; i++) {
		float x = topRadius*cos(i*dTheta);
		float z = topRadius*sin(i*dTheta);
		//vertices[count++] = glm::vec3(x, y, z);
		vertices.push_back(glm::vec3(x, y, z));
	}
	//vertices[count++] = glm::vec3(0, y, 0);
	vertices.push_back(glm::vec3(0, y, 0));

	//bottom cap
	y = -y;

	for (int i = 0; i < slices; i++) {
		float x = bottomRadius*cos(i*dTheta);
		float z = bottomRadius*sin(i*dTheta);
		//vertices[count++] = glm::vec3(x, y, z);
		vertices.push_back(glm::vec3(x, y, z));
	}
	//vertices[count++] = glm::vec3(0, y, 0);
	vertices.push_back(glm::vec3(0, y, 0));
}

void Cylinder::fillIndexBuffer()
{
	//fill indices array 
	
	int ringVertexCount = slices;//slice = 5
	for (int i = 0; i < stacks - 1; i++) {
		for (int j = 0; j < slices - 1; j++) {//j=0,1,2,3
			indices.push_back(i*ringVertexCount + j);
			indices.push_back((i + 1)*ringVertexCount + j);
			indices.push_back((i + 1)*ringVertexCount + j + 1);

			indices.push_back(i*ringVertexCount + j);
			indices.push_back((i + 1)*ringVertexCount + j + 1);
			indices.push_back(i*ringVertexCount + j + 1);
		}
	}

	//top cap
	int baseIndex = (slices)*(stacks)+1;
	int centerIndex = baseIndex + (slices)-1;

	for (int i = 0; i < slices - 1; i++) {
		indices.push_back(centerIndex);
		indices.push_back(baseIndex + i + 1);
		indices.push_back(baseIndex + i);
	}

	//bottom cap
	centerIndex = centerIndex + (slices)+1;
	baseIndex = 0;

	for (int i = 0; i < slices - 1; i++) {
		indices.push_back(centerIndex);
		indices.push_back(baseIndex + i);
		indices.push_back(baseIndex + i + 1);
	}
}
