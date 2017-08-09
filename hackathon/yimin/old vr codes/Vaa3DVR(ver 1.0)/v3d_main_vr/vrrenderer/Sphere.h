#pragma once
#include "RenderableObject.h"

class Sphere :
	public RenderableObject
{
public:
	Sphere(const float radius = 0.5f, const int slices = 20, const int stacks = 20);
	~Sphere();

	GLenum GetPrimitiveType();
	void fillVertexBuffer();
	void fillIndexBuffer();

private:
	float radius;
	int slices, stacks;
};

