#include "z3dnetworkevaluator.h"

#include <set>
#include <queue>
#include <algorithm>
#include <boost/graph/topological_sort.hpp>
#include "z3dcanvasrenderer.h"
#include "z3dcameraparameter.h"
#include "z3dprocessor.h"
#include "z3drenderprocessor.h"
#include "QsLog.h"
#include "z3dtexture.h"
#include "z3drendertarget.h"

//#define PROFILE3DRENDERERS

Z3DNetworkEvaluator::Z3DNetworkEvaluator(QObject *parent)
  : QObject(parent)
  , m_renderingOrder()
  , m_processWrappers()
  , m_openGLContext(NULL)
  , m_locked(false)
  , m_processPending(false)
  , m_canvasRenderer(NULL)
{
#if defined(_DEBUG_)
  addProcessWrapper(new Z3DCheckOpenGLStateProcessWrapper());
#endif
#if defined(PROFILE3DRENDERERS)
  addProcessWrapper(new Z3DProfileProcessWrapper());
#endif
}

Z3DNetworkEvaluator::~Z3DNetworkEvaluator()
{
  clearProcessWrappers();
}

void Z3DNetworkEvaluator::setNetworkSink(Z3DCanvasRenderer *canvasRenderer)
{
  if (m_canvasRenderer == canvasRenderer)
    return;

  if (m_canvasRenderer)
    deinitializeNetwork();

  m_canvasRenderer = canvasRenderer;

  buildNetwork();
}

QString Z3DNetworkEvaluator::process(bool stereo)
{
  if (!m_canvasRenderer)
    return "No Network";

  if (m_locked) {
    LDEBUG() << "locked. Scheduling.";
    m_processPending = true;
    return "";
  }

  lock();

  QString error;

  // notify process wrappers
  for (size_t j = 0; j < m_processWrappers.size(); ++j)
    m_processWrappers[j]->beforeNetworkProcess();
  CHECK_GL_ERROR;

  // Iterate over processing in rendering order
  for (size_t i = 0; i < m_renderingOrder.size(); ++i) {
    Z3DProcessor* currentProcessor = m_renderingOrder[i];

    // all processors should have been initialized at this point
    if (!currentProcessor->isInitialized()) {
      LWARN() << "Skipping uninitialized processor" << currentProcessor->getClassName();
      continue;
    }

    Z3DEye eye = stereo ? LeftEye : CenterEye;

    // run the processor, if it needs processing and is ready
    if (!currentProcessor->isValid(eye) && currentProcessor->isReady(eye)) {
      // notify process wrappers
      for (size_t j=0; j < m_processWrappers.size(); ++j)
        m_processWrappers[j]->beforeProcess(currentProcessor);
      CHECK_GL_ERROR;

      try {
        getGLFocus();
        currentProcessor->process(eye);
        currentProcessor->setValid(eye);
        CHECK_GL_ERROR;
      }
      catch (Exception& e) {
        LERROR() << "Exception from"
                 << currentProcessor->getClassName() << ":" << e.what();
        error += e.what();
        break;
      }
      catch (std::exception& e) {
        LERROR() << "std exception from"
                 << currentProcessor->getClassName() << ":" << e.what();
        error += e.what();
        break;
      }

      // notify process wrappers
      getGLFocus();
      for (size_t j = 0; j < m_processWrappers.size(); ++j)
        m_processWrappers[j]->afterProcess(currentProcessor);
      CHECK_GL_ERROR;
    }

    if (stereo && !currentProcessor->isValid(RightEye) && currentProcessor->isReady(RightEye)) {
      // notify process wrappers
      for (size_t j=0; j < m_processWrappers.size(); ++j)
        m_processWrappers[j]->beforeProcess(currentProcessor);
      CHECK_GL_ERROR;

      try {
        getGLFocus();
        currentProcessor->process(RightEye);
        currentProcessor->setValid(RightEye);
        CHECK_GL_ERROR;
      }
      catch (Exception& e) {
        LERROR() << "Exception from"
                 << currentProcessor->getClassName() << ":" << e.what();
        error += e.what();
        break;
      }
      catch (std::exception& e) {
        LERROR() << "std exception from"
                 << currentProcessor->getClassName() << ":" << e.what();
        error += e.what();
        break;
      }

      // notify process wrappers
      getGLFocus();
      for (size_t j = 0; j < m_processWrappers.size(); ++j)
        m_processWrappers[j]->afterProcess(currentProcessor);
      CHECK_GL_ERROR;
    }
  }

  // notify process wrappers
  for (size_t j = 0; j < m_processWrappers.size(); ++j)
    m_processWrappers[j]->afterNetworkProcess();
  CHECK_GL_ERROR;

  unlock();

  // make sure that canvases are repainted, if their update has been blocked by the locked evaluator
  if (m_processPending) {
    m_processPending = false;
    m_canvasRenderer->invalidate();
  }

  return error;
}

bool Z3DNetworkEvaluator::initializeNetwork()
{
  if (m_locked) {
    LDEBUG() << "locked.";
    return false;
  }

  lock();

  bool failed = false;
  for (size_t i = 0; i < m_renderingOrder.size(); ++i) {
    Z3DProcessor* processor = m_renderingOrder[i];
    if (!processor->isInitialized()) {
      try {
        getGLFocus();
        processor->initialize();
        processor->m_initialized = true;
        CHECK_GL_ERROR;
      }
      catch (const Exception& e) {
        LERROR() << "Failed to initialize processor" << processor->getClassName() << e.what();

        // deinitialize processor, in order to make sure that all resources are freed
        LINFO() << "Deinitializing" << processor->getClassName() << "...";
        getGLFocus();
        processor->m_initialized = true;
        processor->deinitialize();
        processor->m_initialized = false;

        // don't break, try to initialize the other processors even if one failed
        failed = true;
      }
    }
  }

  // update size
  sizeChangedFromProcessor();
  for (size_t i=0; i<m_reverseSortedRenderProcessors.size(); i++) {
    QObject::disconnect(m_reverseSortedRenderProcessors[i],
                        SIGNAL(requestUpstreamSizeChange(Z3DRenderProcessor*)),
                        0, 0);
    connect(m_reverseSortedRenderProcessors[i], SIGNAL(requestUpstreamSizeChange(Z3DRenderProcessor*)),
            this, SLOT(sizeChangedFromProcessor(Z3DRenderProcessor*)));
  }

  unlock();
  CHECK_GL_ERROR;
  return !failed;
}

bool Z3DNetworkEvaluator::deinitializeNetwork()
{
  if (m_locked) {
    LWARN() << "locked.";
    return false;
  }

  if (!m_canvasRenderer) {
    LWARN() << "no network.";
    return false;
  }

  lock();

  bool failed = false;
  for (size_t i = 0; i < m_renderingOrder.size(); ++i) {
    Z3DProcessor* processor = m_renderingOrder[i];
    if (processor->isInitialized()) {
      try {
        getGLFocus();
        processor->deinitialize();
        processor->m_initialized = false;
        CHECK_GL_ERROR;
      }
      catch (const Exception& e) {
        LERROR() << "Failed to deinitialize processor" << processor->getClassName() << e.what();
        // don't break, try to deinitialize the other processors even if one failed
        failed = true;
      }
    }
  }

  unlock();
  return !failed;
}

void Z3DNetworkEvaluator::addProcessWrapper(Z3DProcessWrapper* w)
{
  m_processWrappers.push_back(w);
}

void Z3DNetworkEvaluator::removeProcessWrapper(const Z3DProcessWrapper* w)
{
  std::vector<Z3DProcessWrapper*>::iterator it = std::find(m_processWrappers.begin(), m_processWrappers.end(), w);
  if (it != m_processWrappers.end()) {
    m_processWrappers.erase(it);
    delete w;
  }
}

void Z3DNetworkEvaluator::clearProcessWrappers()
{
  for (size_t i=0; i<m_processWrappers.size(); ++i) {
    delete m_processWrappers[i];
  }

  m_processWrappers.clear();
}

void Z3DNetworkEvaluator::updateNetwork()
{
  buildNetwork();
  initializeNetwork();
}

void Z3DNetworkEvaluator::buildNetwork()
{
  std::set<Z3DProcessor*> prevProcessors(m_renderingOrder.begin(), m_renderingOrder.end());

  m_renderingOrder.clear();
  m_processorToVertexMapper.clear();
  m_processorGraph.clear();
  m_reverseSortedRenderProcessors.clear();

  // nothing more to do, if no network sink is present
  if (!m_canvasRenderer)
    return;

  std::set<Z3DProcessor*> processed;
  std::queue<Z3DProcessor*> processQueue;

  processQueue.push(m_canvasRenderer);
  Vertex v = boost::add_vertex(VertexInfo(m_canvasRenderer), m_processorGraph);
  m_processorToVertexMapper[m_canvasRenderer] = v;

  // build graph of all connected processors
  while (!processQueue.empty()) {
    Z3DProcessor *processor = processQueue.front();
    const std::vector<Z3DInputPortBase*> inports = processor->getInputPorts();
    for (size_t i = 0; i < inports.size(); ++i) {
      const std::vector<Z3DOutputPortBase*> connected = inports[i]->getConnected();
      for (size_t j = 0; j < connected.size(); ++j) {
        Z3DProcessor *outProcessor = connected[j]->getProcessor();
        if (m_processorToVertexMapper.find(outProcessor) == m_processorToVertexMapper.end()) {
          processQueue.push(outProcessor);
          Vertex v = boost::add_vertex(VertexInfo(outProcessor), m_processorGraph);
          m_processorToVertexMapper[outProcessor] = v;
        }
        boost::add_edge(m_processorToVertexMapper[outProcessor],
                        m_processorToVertexMapper[processor],
                        EdgeInfo(connected[j], inports[i]),
                        m_processorGraph);
      }
    }

    processed.insert(processor);
    processQueue.pop();
  }

  // sort to get rendering order
  std::vector<Vertex> sorted;
  boost::topological_sort(m_processorGraph, std::back_inserter(sorted));
  for (std::vector<Vertex>::reverse_iterator rit = sorted.rbegin();
       rit != sorted.rend(); rit++) {
    m_renderingOrder.push_back(m_processorGraph[*rit].processor);
  }

#ifdef _DEBUG_
  LINFO() << "Rendering Order: ";
  for (size_t i=0; i<m_renderingOrder.size(); i++) {
    LINFO() << "  " << i << ": " << m_renderingOrder[i]->getClassName();
  }
  LINFO() << "";
#endif

  // compare processors in network before and after updating, deinitialize removed processors
  std::set<Z3DProcessor*> currProcessors(m_renderingOrder.begin(), m_renderingOrder.end());
  std::set<Z3DProcessor*> removedProcessors;
  std::set_difference(prevProcessors.begin(), prevProcessors.end(), currProcessors.begin(), currProcessors.end(),
                      std::inserter(removedProcessors, removedProcessors.end()));

  if (!removedProcessors.empty()) {
#ifdef _DEBUG_
    LINFO() << "Deinitialized processors after updating: ";
#endif
    for (std::set<Z3DProcessor*>::iterator it=removedProcessors.begin();
         it != removedProcessors.end(); ++it) {
#ifdef _DEBUG_
      LINFO() << "  " << " : " << (*it)->getClassName();
#endif
      try {
        getGLFocus();
        (*it)->deinitialize();
        (*it)->m_initialized = false;
        CHECK_GL_ERROR;
      }
      catch (const Exception& e) {
        LERROR() << "Failed to deinitialize processor" << (*it)->getClassName() << e.what();
      }
    }
#ifdef _DEBUG_
    LINFO() << "";
#endif
  }

  // update reverse sorted renderprocessors
  for (std::vector<Z3DProcessor*>::reverse_iterator rit = m_renderingOrder.rbegin();
       rit != m_renderingOrder.rend(); rit++) {
    if (dynamic_cast<Z3DRenderProcessor*>(*rit) == 0)
      continue;
    m_reverseSortedRenderProcessors.push_back(dynamic_cast<Z3DRenderProcessor*>(*rit));
  }
}

void Z3DNetworkEvaluator::sizeChangedFromProcessor(Z3DRenderProcessor *rp)
{
  if (rp) {
    bool started = false;
    for (size_t i=0; i<m_reverseSortedRenderProcessors.size(); i++) {
      if (started)
        m_reverseSortedRenderProcessors[i]->updateSize();
      else {
        if (rp == m_reverseSortedRenderProcessors[i])
          started = true;
      }
    }
  } else {
    for (size_t i=0; i<m_reverseSortedRenderProcessors.size(); i++) {
      m_reverseSortedRenderProcessors[i]->updateSize();
    }
  }
}

bool Z3DNetworkEvaluator::hasCameraParameter(Z3DProcessor *processor) const
{
  std::vector<Z3DCameraParameter*> cams = processor->getParametersByType<Z3DCameraParameter>();
  return !cams.empty();
}


// ----------------------------------------------------------------------------

void Z3DCheckOpenGLStateProcessWrapper::afterProcess(const Z3DProcessor* p)
{
  checkState(p);
}

void Z3DCheckOpenGLStateProcessWrapper::beforeNetworkProcess()
{
  checkState();
}

void Z3DCheckOpenGLStateProcessWrapper::checkState(const Z3DProcessor *p)
{

  if (!checkGLState(GL_BLEND, false)) {
    glDisable(GL_BLEND);
    warn(p, "GL_BLEND was enabled");
  }

  if (!checkGLState(GL_BLEND_SRC, GL_ONE) || !checkGLState(GL_BLEND_DST, GL_ZERO)) {
    glBlendFunc(GL_ONE, GL_ZERO);
    warn(p, "Modified BlendFunc");
  }

  if (!checkGLState(GL_DEPTH_TEST, false)) {
    glDisable(GL_DEPTH_TEST);
    warn(p, "GL_DEPTH_TEST was enabled");
  }

  if (!checkGLState(GL_CULL_FACE, false)) {
    glDisable(GL_CULL_FACE);
    warn(p, "GL_CULL_FACE was enabled");
  }

  if (!checkGLState(GL_COLOR_CLEAR_VALUE, glm::vec4(0.f))) {
    glClearColor(0.f, 0.f, 0.f, 0.f);
    warn(p, "glClearColor() was not set to all zeroes");
  }

  if (!checkGLState(GL_DEPTH_CLEAR_VALUE, 1.f)) {
    glClearDepth(1.0);
    warn(p, "glClearDepth() was not set to 1.0");
  }

  if (!checkGLState(GL_LINE_WIDTH, 1.f)) {
    glLineWidth(1.f);
    warn(p, "glLineWidth() was not set to 1.0");
  }

  if (!checkGLState(GL_MATRIX_MODE, GL_MODELVIEW)) {
    glMatrixMode(GL_MODELVIEW);
    warn(p, "glMatrixMode was not set to GL_MODELVIEW");
  }

  if (!checkGLState(GL_ACTIVE_TEXTURE, GL_TEXTURE0)) {
    glActiveTexture(GL_TEXTURE0);
    warn(p, "glActiveTexture was not set to GL_TEXTURE0");
  }

  if (!checkGLState(GL_TEXTURE_1D, false)) {
    glDisable(GL_TEXTURE_1D);
    warn(p, "GL_TEXTURE_1D was enabled");
  }

  if (!checkGLState(GL_TEXTURE_2D, false)) {
    glDisable(GL_TEXTURE_2D);
    warn(p, "GL_TEXTURE_2D was enabled");
  }

  if (!checkGLState(GL_TEXTURE_3D, false)) {
    glDisable(GL_TEXTURE_3D);
    warn(p, "GL_TEXTURE_3D was enabled");
  }

  GLint id;
  glGetIntegerv(GL_CURRENT_PROGRAM, &id);
  if (id != 0) {
    glUseProgram(0);
    warn(p, "A shader was active");
  }

  if (Z3DRenderTarget::getCurrentBoundDrawFBO() != 0) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    warn(p, "A render target was bound (releaseTarget() missing?)");
  }

  if (!checkGLState(GL_DEPTH_FUNC, GL_LESS)) {
    glDepthFunc(GL_LESS);
    warn(p, "glDepthFunc was not set to GL_LESS");
  }

  if (!checkGLState(GL_CULL_FACE_MODE, GL_BACK)) {
    glCullFace(GL_BACK);
    warn(p, "glCullFace was not set to GL_BACK");
  }
}

void Z3DCheckOpenGLStateProcessWrapper::warn(const Z3DProcessor* p, const QString& message)
{
  if (p) {
    LWARN() << "Invalid OpenGL state after processing" << p->getClassName() << ":" << message;
  }
  else {
    LWARN() << "Invalid OpenGL state before network processing:" << message;
  }
}


void Z3DProfileProcessWrapper::beforeProcess(const Z3DProcessor *)
{
  m_benchTimer.start();
}

void Z3DProfileProcessWrapper::afterProcess(const Z3DProcessor *p)
{
  m_benchTimer.stop();
  LINFO() << "Process" << p->getClassName() << "took time:" << m_benchTimer.time() << "seconds.";
}

void Z3DProfileProcessWrapper::beforeNetworkProcess()
{
  m_benchTimer.reset();
}

void Z3DProfileProcessWrapper::afterNetworkProcess()
{
  LINFO() << "Network took time:" << m_benchTimer.total() << "seconds.";
}
