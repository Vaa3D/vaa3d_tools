#ifndef ZNEURONTRACER_H
#define ZNEURONTRACER_H

#include "zswcpath.h"
#include "tz_trace_defs.h"
#include "tz_trace_utils.h"

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

private:
  Stack *m_stack;
  Trace_Workspace *m_traceWorkspace;
  Connection_Test_Workspace *m_connWorkspace;
  ZSwcConnector *m_swcConnector;
};

#endif // ZNEURONTRACER_H
