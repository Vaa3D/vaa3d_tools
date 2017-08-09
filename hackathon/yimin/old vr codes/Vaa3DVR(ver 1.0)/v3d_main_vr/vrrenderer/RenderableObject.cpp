#include "RenderableObject.h"



RenderableObject::RenderableObject()
{
}


RenderableObject::~RenderableObject()
{
	//Destroy vao and vbo
	glDeleteBuffers(1, &vboVerticesID);
	//glDeleteBuffers(1, &vboNormalsID);
	glDeleteBuffers(1, &vboIndicesID);
	glDeleteVertexArrays(1, &vaoID);
}

void RenderableObject::Init()
{
	fillVertexBuffer(); 
	fillIndexBuffer();
	
	//setup vao and vbo stuff
	glGenVertexArrays(1, &vaoID);
	glGenBuffers(1, &vboVerticesID);
	//glGenBuffers(1, &vboNormalsID);
	glGenBuffers(1, &vboIndicesID);

	primType = GetPrimitiveType();

	//now allocate buffers
	glBindVertexArray(vaoID);

	glBindBuffer(GL_ARRAY_BUFFER, vboVerticesID);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
	//glBindBuffer(GL_ARRAY_BUFFER, vboNormalsID);
	//glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndicesID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);

	glBindVertexArray(0);
}

void RenderableObject::Render()
{
	glBindVertexArray(vaoID);
	glDrawElements(primType, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}
