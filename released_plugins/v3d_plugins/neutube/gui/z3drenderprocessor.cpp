#include "z3drenderprocessor.h"

#include "z3dshaderprogram.h"
#include "z3dcameraparameter.h"
#include "QsLog.h"
#include "z3dgpuinfo.h"
#include <QImageWriter>

Z3DRenderProcessor::Z3DRenderProcessor()
  : Z3DProcessor()
  , m_hardwareSupportVAO(Z3DGpuInfoInstance.isVAOSupported())
  , m_privateVAO(0)
{
  m_rendererBase = new Z3DRendererBase();
}

Z3DRenderProcessor::~Z3DRenderProcessor()
{
  delete m_rendererBase;
}

void Z3DRenderProcessor::initialize()
{
  Z3DProcessor::initialize();

  const std::vector<Z3DRenderOutputPort*> pports = getPrivateRenderPorts();
  for (size_t i=0; i<pports.size(); ++i) {
    pports[i]->initialize();
  }

  if (m_hardwareSupportVAO) {
    glGenVertexArrays(1, &m_privateVAO);
  }
  CHECK_GL_ERROR;
}

void Z3DRenderProcessor::deinitialize()
{
  const std::vector<Z3DRenderOutputPort*> pports = getPrivateRenderPorts();
  for (size_t i=0; i<pports.size(); ++i) {
    pports[i]->deinitialize();
  }

  if (m_hardwareSupportVAO) {
    glDeleteVertexArrays(1, &m_privateVAO);
  }
  CHECK_GL_ERROR;

  Z3DProcessor::deinitialize();
}

void Z3DRenderProcessor::updateSize()
{
  // 1. update outport size
  bool resized = false;

  const std::vector<Z3DOutputPortBase*> outports = getOutputPorts();
  glm::ivec2 maxOutportSize(-1, -1);
  for(size_t i=0; i<outports.size(); ++i) {
    Z3DRenderOutputPort* rp = dynamic_cast<Z3DRenderOutputPort*>(outports[i]);
    if (rp) {
      glm::ivec2 outportSize = rp->getExpectedSize();
      if (outportSize.x > 0 && outportSize != rp->getSize()) {
        resized = true;
        rp->resize(outportSize);
      }

      maxOutportSize = glm::max(maxOutportSize, rp->getSize());
    }
  }

  // 2. update private ports
  const std::vector<Z3DRenderOutputPort*> privatePorts = getPrivateRenderPorts();
  for (size_t i=0; i<privatePorts.size(); ++i) {
    privatePorts[i]->resize(maxOutportSize);
  }

  // 3. update inport expected size
  const std::vector<Z3DInputPortBase*> inports = getInputPorts();
  for (size_t i=0; i<inports.size(); i++) {
    Z3DRenderInputPort *renderInport = dynamic_cast< Z3DRenderInputPort* >(inports[i]);
    if (renderInport)
      renderInport->setExpectedSize(maxOutportSize);
  }

  // 4. notify cameras about viewport change
  if(resized) {
    const std::vector<ZParameter*> parameters = getParameters();
    for (size_t i=0; i<parameters.size(); ++i) {
      Z3DCameraParameter* cameraPara = dynamic_cast<Z3DCameraParameter*>(parameters[i]);
      if (cameraPara) {
        cameraPara->viewportChanged(maxOutportSize);
      }
    }
  }

  invalidate();
}

void Z3DRenderProcessor::addPrivateRenderPort(Z3DRenderOutputPort* port)
{
  port->setProcessor(this);
  m_privateRenderPorts.push_back(port);

  std::map<QString, Z3DOutputPortBase*>::const_iterator it = m_outputPortMap.find(port->getName());
  if (it == m_outputPortMap.end())
    m_outputPortMap.insert(std::make_pair(port->getName(), port));
  else {
    LERROR() << getClassName() << "port" << port->getName() << "has already been inserted!";
    assert(false);
  }
}

void Z3DRenderProcessor::addPrivateRenderPort(Z3DRenderOutputPort& port)
{
  addPrivateRenderPort(&port);
}

void Z3DRenderProcessor::renderScreenQuad(const Z3DShaderProgram &shader)
{
  if (!shader.isLinked())
    return;

  glDepthFunc(GL_ALWAYS);

  if (m_hardwareSupportVAO) {
    glBindVertexArray(m_privateVAO);
  }

  GLfloat vertices[] = {-1.f, 1.f, 0.f, //top left corner
                        -1.f, -1.f, 0.f, //bottom left corner
                        1.f, 1.f, 0.f, //top right corner
                        1.f, -1.f, 0.f}; // bottom right rocner
  GLint attr_vertex = shader.attributeLocation("attr_vertex");

  GLuint bufObjects[1];
  glGenBuffers(1, bufObjects);

  glEnableVertexAttribArray(attr_vertex);
  glBindBuffer(GL_ARRAY_BUFFER, bufObjects[0]);
  glBufferData(GL_ARRAY_BUFFER, 3*4*sizeof(GLfloat), vertices, GL_STATIC_DRAW);
  glVertexAttribPointer(attr_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDeleteBuffers(1, bufObjects);

  glDisableVertexAttribArray(attr_vertex);

  if (m_hardwareSupportVAO) {
    glBindVertexArray(0);
  }

  glDepthFunc(GL_LESS);
}

const std::vector<Z3DRenderOutputPort*> &Z3DRenderProcessor::getPrivateRenderPorts() const
{
  return m_privateRenderPorts;
}

void Z3DRenderProcessor::saveTextureAsImage(Z3DTexture *tex, const QString &filename)
{
  try {
    GLubyte* colorBuffer = 0;
    colorBuffer = tex->downloadTextureToBuffer(GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV);
    glm::ivec2 size = glm::ivec2(tex->getDimensions());
    QImage upsideDownImage((const uchar*)colorBuffer, size.x, size.y,
                           QImage::Format_ARGB32);
    QImage image = upsideDownImage.mirrored(false, true);
    QImageWriter writer(filename);
    writer.setCompression(1);
    if(!writer.write(image)) {
      LERROR() << writer.errorString();
      delete[] colorBuffer;
    }
    delete[] colorBuffer;
  }
  catch (Exception const & e) {
    LERROR() << "Exception:" << e.what();
  }
  catch (std::exception const & e) {
    LERROR() << "std exception:" << e.what();
  }
}
