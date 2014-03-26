#ifndef ZNEURONTRACER_H
#define ZNEURONTRACER_H

#include "zswcpath.h"
#include "tz_trace_defs.h"
#include "tz_trace_utils.h"
#include "neutube.h"

class ZStack;
class ZSwcTree;
class ZSwcConnector;

class ZNeuronTracer
{
public:
  ZNeuronTracer();
  ~ZNeuronTracer();

public:
  ZSwcPath trace(double x, double y, double z);
  void updateMask(const ZSwcPath &branch);
  void setIntensityField(Stack *stack);
  void updateMask(Swc_Tree *tree);
  void setTraceWorkspace(Trace_Workspace *workspace);
  void setConnWorkspace(Connection_Test_Workspace *workspace);

  Swc_Tree* trace(double x1, double y1, double z1, double r1,
                 double x2, double y2, double z2, double r2);

  inline void setBackgroundType(NeuTube::EImageBackground bg) {
    m_backgroundType = bg;
  }

  inline void setResolution(double x, double y, double z) {
    m_resolution[0] = x;
    m_resolution[1] = y;
    m_resolution[2] = z;
  }

private:
  Stack *m_stack;
  Trace_Workspace *m_traceWorkspace;
  Connection_Test_Workspace *m_connWorkspace;
  ZSwcConnector *m_swcConnector;
  NeuTube::EImageBackground m_backgroundType;
  double m_resolution[3];
};

#endif // ZNEURONTRACER_H
