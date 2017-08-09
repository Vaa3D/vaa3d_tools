#pragma once

#include <vector>
// GL Includes
#include <GL/glew.h> // Contains all the necessery OpenGL includes
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

class RenderableObject
{
public:
	RenderableObject();
	~RenderableObject();

	virtual GLenum GetPrimitiveType() = 0;
	virtual void fillVertexBuffer() = 0;
	virtual void fillIndexBuffer() = 0;

	void Init();
	void Render();

	/*  Mesh Data  */
	vector<glm::vec3> vertices;
	vector<glm::vec3> normals;
	vector<GLuint> indices;

protected:
	GLuint vaoID;
	GLuint vboVerticesID;
	GLuint vboNormalsID;
	GLuint vboIndicesID;

	GLenum primType;
};

