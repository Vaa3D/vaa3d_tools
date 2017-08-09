#pragma once
#include "RenderableObject.h"

class Cylinder :
	public RenderableObject
{
public:
	Cylinder(const float topRadius = 0.5f, const float bottomRadius = 0.5f, const float height = 1.0f, const int slices = 20, const int stacks = 2);
	~Cylinder();

	GLenum GetPrimitiveType();
	void fillVertexBuffer();
	void fillIndexBuffer();

private:
	float topRadius, bottomRadius;
	float height;
	int slices, stacks;
};