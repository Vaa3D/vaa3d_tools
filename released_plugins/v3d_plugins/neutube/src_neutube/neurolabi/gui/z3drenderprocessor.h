#ifndef Z3DRENDERPROCESSOR_H
#define Z3DRENDERPROCESSOR_H

#include <vector>

#include "z3dprocessor.h"
#include "z3drenderport.h"
#include "z3drendererbase.h"

// The base class for all processor classes that render to Z3DRenderOutputPort or screen
class Z3DRenderProcessor : public Z3DProcessor
{
  Q_OBJECT
public:
  Z3DRenderProcessor();
  virtual ~Z3DRenderProcessor();

  const std::vector<Z3DRenderOutputPort*>& getPrivateRenderPorts() const;

  void saveTextureAsImage(Z3DTexture* tex, const QString &filename);

signals:
  // emit this only if resize starts from current processor.
  void requestUpstreamSizeChange(Z3DRenderProcessor*);

public slots:
  // 1. for each outport, get all expected size from all connected inports, and use the maximum one
  //    as the new size of the outport
  // 2. update private port size
  // 3. Once we get the newsize of all outports, we calculate a expected size for each inport and set it.
  //    default choice for inport expected size is the maximum new outport size
  // 4. notify camera parameter about the change
  // reimplement this if you want different behavior
  virtual void updateSize();

protected:
  // Initializes all private Z3DRenderPorts.
  virtual void initialize();

  // Deinitializes all private Z3DRenderPorts.
  virtual void deinitialize();

  void addPrivateRenderPort(Z3DRenderOutputPort* port);
  void addPrivateRenderPort(Z3DRenderOutputPort& port);

  void renderScreenQuad(const Z3DShaderProgram &shader);

protected:
  Z3DRendererBase *m_rendererBase;
  bool m_hardwareSupportVAO;

private:
  std::vector<Z3DRenderOutputPort*> m_privateRenderPorts;

  GLuint m_privateVAO;
};

#endif // Z3DRENDERPROCESSOR_H
