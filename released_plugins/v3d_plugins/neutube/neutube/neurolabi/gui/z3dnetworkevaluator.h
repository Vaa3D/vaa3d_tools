#ifndef Z3DNETWORKEVALUATOR_H
#define Z3DNETWORKEVALUATOR_H

#include <QObject>
#include <vector>
#include "zbenchtimer.h"
#include "z3dcanvas.h"

#ifndef Q_MOC_RUN
#include "boost/graph/adjacency_list.hpp"
#endif

class Z3DRenderProcessor;
class Z3DCanvasRenderer;
class Z3DProcessor;
class Z3DOutputPortBase;
class Z3DInputPortBase;

class Z3DProcessWrapper
{
public:
  virtual ~Z3DProcessWrapper() {}
  virtual void beforeProcess(const Z3DProcessor*) {}
  virtual void afterProcess(const Z3DProcessor*) {}
  virtual void beforeNetworkProcess() {}
  virtual void afterNetworkProcess() {}
};

class Z3DNetworkEvaluator : public QObject
{
  Q_OBJECT

public:
  Z3DNetworkEvaluator(QObject *parent = 0);
  ~Z3DNetworkEvaluator();

  void setOpenGLContext(Z3DCanvas *context) { m_openGLContext = context; }

  // set canvasrenderer as the sink of rendering network and build network
  void setNetworkSink(Z3DCanvasRenderer* canvasRenderer);

  // process the currently assigned network. The rendering order is determined internally
  // according the network topology and the invalidation levels of the processors.
  // return error message if any
  // stereo means run two passes for left and right eye
  QString process(bool stereo = false);

  // Initializes all related processors, return true if initialization of all processors has been successful
  // must be called before process()
  bool initializeNetwork();
  // Deinitializes all related processors, return true if deinitialization of all processors has been successful
  // must be called before delelting processors
  bool deinitializeNetwork();

  // Manage ProcessWrappers which will be called before and after Processor::process()
  // networkevaluator will take ownership of the passed Z3DProcessWrapper object
  void addProcessWrapper(Z3DProcessWrapper* w);
  void removeProcessWrapper(const Z3DProcessWrapper* w);
  void clearProcessWrappers();

public slots:
  // call when network topology changed
  void updateNetwork();

protected:
  void buildNetwork();

  // Locks the evaluator. In this state, it does not perform
  // any operations, such as initializing or processing, on the processor network
  void lock() { m_locked = true; }
  void unlock() { m_locked = false; }

  inline void getGLFocus() const { if (m_openGLContext) m_openGLContext->getGLFocus(); }

protected slots:
  // update all upstream renderprocessor size. If input renderProcessor is NULL, update all renderProcessor
  void sizeChangedFromProcessor(Z3DRenderProcessor* rp = 0);

private:
  bool hasCameraParameter(Z3DProcessor *processor) const;

  std::vector<Z3DProcessor*> m_renderingOrder;

  std::vector<Z3DProcessWrapper*> m_processWrappers;

  // used to make sure we operate in the correct opengl context
  Z3DCanvas* m_openGLContext;

  bool m_locked;

  bool m_processPending;

  Z3DCanvasRenderer *m_canvasRenderer;

  struct VertexInfo {
    VertexInfo() : processor(0) {}
    VertexInfo(Z3DProcessor* p) : processor(p) {}
    Z3DProcessor *processor;
    //
  };

  struct EdgeInfo {
    EdgeInfo(Z3DOutputPortBase *out, Z3DInputPortBase *in) : outPort(out), inPort(in) {}
    Z3DOutputPortBase *outPort;
    Z3DInputPortBase *inPort;
  };

  typedef boost::adjacency_list<boost::listS, boost::vecS, boost::bidirectionalS, VertexInfo, EdgeInfo> GraphT;
  typedef boost::graph_traits<GraphT>::vertex_descriptor Vertex;
  typedef boost::graph_traits<GraphT>::edge_descriptor Edge;
  std::map<Z3DProcessor*, Vertex> m_processorToVertexMapper;
  GraphT m_processorGraph;

  std::vector<Z3DRenderProcessor*> m_reverseSortedRenderProcessors;
};

// check if OpenGL state conforms to default settings. Log a warning message if not.
class Z3DCheckOpenGLStateProcessWrapper : public Z3DProcessWrapper
{
public:
  void afterProcess(const Z3DProcessor *p);
  void beforeNetworkProcess();
  void checkState(const Z3DProcessor *p = 0);
  void warn(const Z3DProcessor *p, const QString &message);
};

// profile each process and whole network
class Z3DProfileProcessWrapper : public Z3DProcessWrapper
{
  ZBenchTimer m_benchTimer;
public:
  void beforeProcess(const Z3DProcessor*);
  void afterProcess(const Z3DProcessor *p);
  void beforeNetworkProcess();
  void afterNetworkProcess();
};

#endif // Z3DNETWORKEVALUATOR_H
