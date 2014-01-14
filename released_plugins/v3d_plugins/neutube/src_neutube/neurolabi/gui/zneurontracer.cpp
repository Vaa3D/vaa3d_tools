#include "zneurontracer.h"
#include "zlocsegchain.h"
#include "swctreenode.h"
#include "c_stack.h"
#include "zswcconnector.h"
#include "tz_math.h"
#include "zstackgraph.h"
#include "zvoxelarray.h"

ZNeuronTracer::ZNeuronTracer() : m_stack(NULL), m_traceWorkspace(NULL),
  m_connWorkspace(NULL), m_swcConnector(NULL),
  m_backgroundType(NeuTube::IMAGE_BACKGROUND_DARK)
{
  m_swcConnector = new ZSwcConnector;
  m_resolution[0] = 1.0;
  m_resolution[1] = 1.0;
  m_resolution[2] = 1.0;
}

ZNeuronTracer::~ZNeuronTracer()
{
  delete m_swcConnector;
}

void ZNeuronTracer::setIntensityField(Stack *stack)
{
  m_stack = stack;
}

ZSwcPath ZNeuronTracer::trace(double x, double y, double z)
{
  if (m_traceWorkspace->trace_mask == NULL) {
    m_traceWorkspace->trace_mask =
        C_Stack::make(GREY, C_Stack::width(m_stack), C_Stack::height(m_stack),
                      C_Stack::depth(m_stack));
    Zero_Stack(m_traceWorkspace->trace_mask);
  }

  double pos[3];
  pos[0] = x;
  pos[1] = y;
  pos[2] = z;

  Local_Neuroseg *locseg = New_Local_Neuroseg();
  Set_Neuroseg(&(locseg->seg), 3.0, 0.0, 11.0, TZ_PI_4, 0.0, 0.0, 0.0, 1.0);

  Set_Neuroseg_Position(locseg, pos, NEUROSEG_CENTER);

  Locseg_Fit_Workspace *ws =
      (Locseg_Fit_Workspace*) m_traceWorkspace->fit_workspace;
  Local_Neuroseg_Optimize_W(locseg, m_stack, 1.0, 1, ws);

  Trace_Record *tr = New_Trace_Record();
  tr->mask = ZERO_BIT_MASK;
  Trace_Record_Set_Fix_Point(tr, 0.0);
  Trace_Record_Set_Direction(tr, DL_BOTHDIR);
  Locseg_Node *p = Make_Locseg_Node(locseg, tr);
  Locseg_Chain *locseg_chain = Make_Locseg_Chain(p);

  Trace_Workspace_Set_Trace_Status(m_traceWorkspace, TRACE_NORMAL,
                                   TRACE_NORMAL);
  Trace_Locseg(m_stack, 1.0, locseg_chain, m_traceWorkspace);
  Locseg_Chain_Remove_Overlap_Ends(locseg_chain);
  Locseg_Chain_Remove_Turn_Ends(locseg_chain, 1.0);

  int n;
  Geo3d_Circle *circles =
      Locseg_Chain_To_Geo3d_Circle_Array(locseg_chain, NULL, &n);

  ZSwcPath path;
  for (int i = 0; i < n; ++i) {
    Swc_Tree_Node *tn = SwcTreeNode::makePointer(circles[i].center[0],
        circles[i].center[1], circles[i].center[2], circles[i].radius);
    if (!path.empty()) {
      SwcTreeNode::setParent(tn, path.back());
    }
    path.push_back(tn);
  }

  return path;
}

void ZNeuronTracer::updateMask(const ZSwcPath &branch)
{
  Swc_Tree_Node_Label_Workspace workspace;
  Default_Swc_Tree_Node_Label_Workspace(&workspace);
  for (ZSwcPath::const_iterator iter = branch.begin(); iter != branch.end();
       ++iter) {
    Swc_Tree_Node_Label_Stack(*iter, m_traceWorkspace->trace_mask, &workspace);
  }
}

void ZNeuronTracer::updateMask(Swc_Tree *tree)
{
  Swc_Tree_Node_Label_Workspace workspace;
  Default_Swc_Tree_Node_Label_Workspace(&workspace);
  Swc_Tree_Label_Stack(tree, m_traceWorkspace->trace_mask, &workspace);
}

void ZNeuronTracer::setTraceWorkspace(Trace_Workspace *workspace)
{
  m_traceWorkspace = workspace;
}

void ZNeuronTracer::setConnWorkspace(Connection_Test_Workspace *workspace)
{
  m_connWorkspace = workspace;
}

#define MAX_P2P_TRACE_DISTANCE 100

Swc_Tree* ZNeuronTracer::trace(double x1, double y1, double z1, double r1,
                               double x2, double y2, double z2, double r2)
{
  if (x1 < 0 || y1 < 0 || z1 < 0 || x1 >= C_Stack::width(m_stack) ||
      y1 >= C_Stack::height(m_stack) || z1 >= C_Stack::depth(m_stack)) {
    return NULL;
  }

  if (ZPoint(x1, y1, z1).distanceTo(x2, y2, z2) > MAX_P2P_TRACE_DISTANCE) {
    return NULL;
  }

  /*
  int start[3];
  int end[3];

  start[0] = iround(x1);
  start[1] = iround(y1);
  start[2] = iround(z1);
  end[0] = iround(x2);
  end[1] = iround(y2);
  end[2] = iround(z2);
  */

  ZStackGraph stackGraph;
  stackGraph.setResolution(m_resolution);
  if (m_backgroundType == NeuTube::IMAGE_BACKGROUND_BRIGHT) {
    stackGraph.setWeightFunction(Stack_Voxel_Weight);
  } else {
    stackGraph.setWeightFunction(Stack_Voxel_Weight_S);
  }

  stackGraph.inferWeightParameter(m_stack);

  int startIndex = C_Stack::indexFromCoord(x1, y1, z1, C_Stack::width(m_stack),
                                           C_Stack::height(m_stack),
                                           C_Stack::depth(m_stack));
  int endIndex = C_Stack::indexFromCoord(x2, y2, z2, C_Stack::width(m_stack),
                                         C_Stack::height(m_stack),
                                         C_Stack::depth(m_stack));

  std::vector<int> path =
      stackGraph.computeShortestPath(m_stack, startIndex, endIndex);

  ZVoxelArray voxelArray;
  for (size_t i = path.size(); i > 0; --i) {
    int x, y, z;
    C_Stack::indexToCoord(path[i - 1], C_Stack::width(m_stack),
                          C_Stack::height(m_stack), &x, &y, &z);
    voxelArray.append(ZVoxel(x, y, z));
  }

  double length = voxelArray.getCurveLength();
  double dist = 0.0;

  for (size_t i = 0; i < path.size(); ++i) {
    double ratio = dist / length;
    double r = r1 * ratio + r2 * (1 - ratio);
    voxelArray.setValue(i, r);
    if (i < path.size() - 1) {
      dist += voxelArray[i].distanceTo(voxelArray[i+1]);
    }
  }

  return voxelArray.toSwcTree();
}
