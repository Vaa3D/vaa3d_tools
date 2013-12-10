#ifndef Z3DTEXTURECOORDINATERENDERER_H
#define Z3DTEXTURECOORDINATERENDERER_H

#include "z3dprimitiverenderer.h"

class Z3DTriangleList;

// render 3d texture coordinates as color
class Z3DTextureCoordinateRenderer : public Z3DPrimitiveRenderer
{
  Q_OBJECT
public:
  explicit Z3DTextureCoordinateRenderer(QObject *parent = 0);

  // triangle list should contains vertexs and 3d texture coordinates
  void setTriangleList(const Z3DTriangleList *mesh) { m_mesh = mesh; m_dataChanged = true; }
  // todo: add function to set data (vertex, texture coordinate, triangle type, indexes) separately

signals:

public slots:

protected:
  virtual void compile();
  virtual void initialize();
  virtual void deinitialize();

  virtual void renderUsingOpengl();
  virtual void renderPickingUsingOpengl();

  virtual void renderUsingGLSL(Z3DEye eye);
  virtual void renderPickingUsingGLSL(Z3DEye);

  const Z3DTriangleList *m_mesh;

  Z3DShaderProgram m_renderTextureCoordinateShader;

  std::vector<GLuint> m_VBOs;
  bool m_dataChanged;
};

#endif // Z3DTEXTURECOORDINATERENDERER_H
