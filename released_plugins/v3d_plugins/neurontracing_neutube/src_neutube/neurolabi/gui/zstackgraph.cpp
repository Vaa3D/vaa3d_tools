#include "zstackgraph.h"

#include "zgraph.h"
#include "tz_error.h"
#include "c_stack.h"
#include "tz_stack_neighborhood.h"
#include "tz_darray.h"
#include "tz_stack_utils.h"
#include "tz_stack_lib.h"
#include "tz_stack_graph.h"
#include "tz_geo3d_utils.h"
#include "tz_math.h"
#include "tz_int_histogram.h"
#include "tz_stack_threshold.h"

ZStackGraph::ZStackGraph()
{
  Default_Stack_Graph_Workspace(&m_workspace);
}

ZStackGraph::~ZStackGraph()
{
  Clean_Stack_Graph_Workspace(&m_workspace);
}

void ZStackGraph::setSignalMask(const Stack *mask)
{
  C_Stack::kill(m_workspace.signal_mask);
  m_workspace.signal_mask = C_Stack::clone(mask);
  if (mask != NULL) {
    Cuboid_I boundBox;
    Stack_Bound_Box(mask, &boundBox);
    Stack_Graph_Workspace_Set_Range(&m_workspace, boundBox.cb[0], boundBox.ce[0],
        boundBox.cb[1], boundBox.ce[1], boundBox.cb[2], boundBox.ce[2]);
  }
}

void ZStackGraph::setSignalMaskAcrossZ(const Stack *mask, int depth)
{
  C_Stack::kill(m_workspace.signal_mask);
  m_workspace.signal_mask = C_Stack::make(
        GREY, C_Stack::width(mask), C_Stack::height(mask), depth);
  //Zero_Stack(m_workspace.signal_mask);
  for (int z = 0; z < depth; ++z) {
    C_Stack::copyPlaneValue(m_workspace.signal_mask, C_Stack::array8(mask), z);
  }

  Cuboid_I boundBox;
  Stack_Bound_Box(mask, &boundBox);
  Stack_Graph_Workspace_Set_Range(&m_workspace, boundBox.cb[0], boundBox.ce[0],
      boundBox.cb[1], boundBox.ce[1], 0, depth - 1);

#ifdef _DEBUG_2
  C_Stack::print(mask);
  C_Stack::print(m_workspace.signal_mask);
#endif
}

void ZStackGraph::setGroupMask(const Stack *mask)
{
  C_Stack::kill(m_workspace.group_mask);
  m_workspace.group_mask = C_Stack::clone(mask);
}

void ZStackGraph::setGroupMaskAcrossZ(const Stack *mask, int depth)
{
  C_Stack::kill(m_workspace.group_mask);
  m_workspace.group_mask = C_Stack::make(
        GREY, C_Stack::width(mask), C_Stack::height(mask), depth);
  //Zero_Stack(m_workspace.group_mask);
  for (int z = 0; z < depth; ++z) {
    C_Stack::copyPlaneValue(m_workspace.group_mask, C_Stack::array8(mask), z);
  }
}

ZGraph* ZStackGraph::buildGraph(const Stack *stack)
{
  //m_workspace.signal_mask = const_cast<Stack*>(stack);

  if (stack == NULL) {
    return NULL;
  }

  ZGraph *graph = new ZGraph(Stack_Graph_W(stack, &m_workspace));

  //m_workspace.signal_mask = NULL;

  return graph;
}

ZGraph* ZStackGraph::buildForegroundGraph(const Stack *stack)
{
  if (stack == NULL) {
    return NULL;
  }

  Stack *oldMask = m_workspace.signal_mask;

  m_workspace.signal_mask = const_cast<Stack*>(stack);
  ZGraph *graph = new ZGraph(Stack_Graph_W(stack, &m_workspace));

  m_workspace.signal_mask = oldMask;

  return graph;
}

std::vector<int> ZStackGraph::computeShortestPath(
    const Stack *stack, int startIndex, int endIndex, bool fgOnly)
{
  updateRange(startIndex, endIndex, C_Stack::width(stack),
              C_Stack::height(stack), C_Stack::depth(stack));

  ZGraph *graph = NULL;

  if (fgOnly) {
    graph = buildForegroundGraph(stack);
  } else {
    graph = buildGraph(stack);
  }

  int swidth = m_workspace.range[1] - m_workspace.range[0] + 1;
  int sarea = swidth * (m_workspace.range[3] - m_workspace.range[2] + 1);
  int width = C_Stack::width(stack);
  int area = C_Stack::area(stack);
  int x0 = m_workspace.range[0];
  int y0 = m_workspace.range[2];
  int z0 = m_workspace.range[4];

  startIndex = Stack_Subindex(startIndex, -x0, -y0, -z0, width, area, swidth, sarea);
  endIndex = Stack_Subindex(endIndex, -x0, -y0, -z0, width, area, swidth, sarea);

  std::vector<int> path = graph->computeShortestPath(startIndex, endIndex);

  delete graph;

  std::vector<int> indexPath = path;

  if (x0 != 0 || y0 != 0 || z0 != 0) {
    for (size_t i = 0; i < path.size(); ++i) {
      indexPath[i] = Stack_Subindex(
            path[i], x0, y0, z0, swidth, sarea, width, area);
    }
  }

  if (m_workspace.signal_mask == NULL) {
    return indexPath;
  }

  std::vector<int> cleanedPath;
  for (size_t i = 0; i < path.size(); ++i) {
    if (path[i] < m_workspace.virtualVertex) {
      cleanedPath.push_back(indexPath[i]);
    }
  }

  return cleanedPath;
}

int ZStackGraph::getGroupId(int voxelIndex)
{
  int groupId = 0;
  if (m_workspace.group_mask != NULL) {
    groupId = C_Stack::array8(m_workspace.group_mask)[voxelIndex];
  }

  return groupId;
}

void ZStackGraph::setWeightFunction(Weight_Func_t f)
{
  m_workspace.wf = f;
}

void ZStackGraph::inferWeightParameter(const Stack *stack)
{
  if (m_workspace.wf == Stack_Voxel_Weight_S) {
    int *hist = Stack_Hist(stack);

    double c1, c2;
    int thre = Hist_Rcthre_R(hist, Int_Histogram_Min(hist),
                             Int_Histogram_Max(hist), &c1, &c2);
    free(hist);
    m_workspace.argv[3] = thre;
    m_workspace.argv[4] = c2 - c1;
    if (m_workspace.argv[4] < 1.0) {
      m_workspace.argv[4] = 1.0;
    }
    m_workspace.argv[4] /= 9.2;
  }
}

void ZStackGraph::updateRange(
    size_t startIndex, size_t endIndex, int width, int height, int depth)
{
  int x1, y1, z1;
  int x2, y2, z2;
  C_Stack::indexToCoord(startIndex, width, height, &x1, &y1, &z1);
  C_Stack::indexToCoord(endIndex, width, height, &x2, &y2, &z2);
  updateRange(x1, y1, z1, x2, y2, z2, width, height, depth);
}

void ZStackGraph::updateRange(int x1, int y1, int z1, int x2, int y2, int z2,
                              int width, int height, int depth)
{
  if (m_workspace.range == NULL) {
    double dist = Geo3d_Dist(x1, y1, z1, x2, y2, z2);
    int margin[3];
    margin[0] = iround(dist - abs(x2 - x1 + 1));
    margin[1] = iround(dist - abs(y2 - y1 + 1));
    margin[2] = iround(dist - abs(z2 - z1 + 1));
    for (int i = 0; i < 3; ++i) {
      if (margin[i] < 0) {
        margin[i] = 0;
      }
    }

    Stack_Graph_Workspace_Set_Range(&m_workspace, x1, x2, y1, y2, z1, z2);
    Stack_Graph_Workspace_Expand_Range(&m_workspace, margin[0], margin[0],
        margin[1], margin[1], margin[2], margin[2]);
    Stack_Graph_Workspace_Validate_Range(&m_workspace, width, height, depth);
  }
}

#if 0
void ZStackGraph::initRange(const Stack *stack, int *stack_range)
{
  int *range = m_workspace.range;

  if (range == NULL) {
    stack_range[0] = 0;
    stack_range[1] = C_Stack::width(stack) - 1;
    stack_range[2] = 0;
    stack_range[3] = C_Stack::height(stack) - 1;
    stack_range[4] = 0;
    stack_range[5] = C_Stack::depth(stack) - 1;
  } else {
    stack_range[0] = imax2(0, range[0]);
    stack_range[1] = imin2(C_Stack::width(stack) - 1, range[1]);
    stack_range[2] = imax2(0, range[2]);
    stack_range[3] = imin2(C_Stack::height(stack) - 1, range[3]);
    stack_range[4] = imax2(0, range[4]);
    stack_range[5] = imin2(C_Stack::depth(stack) - 1, range[5]);
  }
}

#define STACK_GRAPH_ADD_EDGE(cond) \
  for (i = 0; i < m_workspace.conn; i++) { \
    if (cond) { \
      if (Graph_Is_Weighted(graph)) { \
        double weight = dist[i]; \
        if (m_workspace.wf != NULL) { \
          m_workspace.argv[0] = dist[i]; \
          \
          m_workspace.argv[1] = Get_Stack_Pixel((Stack *)stack, \
              x + stack_range[0], \
              y + stack_range[2], \
              z + stack_range[4], 0); \
          m_workspace.argv[2] = \
          Get_Stack_Pixel((Stack *)stack, \
              x + stack_range[0] + x_offset[i], \
              y + stack_range[2] + y_offset[i], \
              z + stack_range[4] + z_offset[i], 0); \
          weight = m_workspace.wf(m_workspace.argv); \
        } \
        Graph_Add_Weighted_Edge(graph, offset, offset + neighbor[i], \
            weight); \
      } else { \
        Graph_Add_Edge(graph, offset, offset + neighbor[i]); \
        m_workspace.intensity[offset] = Get_Stack_Pixel((Stack*) stack, \
              x + stack_range[0], \
              y + stack_range[2], \
              z + stack_range[4], 0); \
      } \
    } \
  }

ZGraph* ZStackGraph::buildGraph(const Stack *stack, const Stack *mask)
{
  TZ_ASSERT(C_Stack::kind(mask) == GREY, "Invalid stack kind");

  int x, y, z;
  int offset = 0;
  int is_in_bound[26];
  int nbound;
  int i;
  int stack_range[6];

  initRange(stack, stack_range);

  int cdepth = stack_range[5] - stack_range[4];
  int cheight = stack_range[3] - stack_range[2];
  int cwidth = stack_range[1] - stack_range[0];

  int nvertex = (cwidth + 1) * (cheight + 1) * (cdepth + 1);

  BOOL weighted = TRUE;
  if (m_workspace.sp_option == 1) {
    weighted = FALSE;
    m_workspace.intensity = darray_malloc(nvertex + 1);
    m_workspace.intensity[nvertex] = Infinity;
  }
  Graph *graph = Make_Graph(nvertex, nvertex, weighted);

  int neighbor[26];
  int scan_mask[26];
  Stack_Neighbor_Offset(m_workspace.conn, cwidth + 1, cheight + 1, neighbor);

  int org_neighbor[26];
  Stack_Neighbor_Offset(m_workspace.conn, C_Stack::width(stack),
                        C_Stack::height(stack), org_neighbor);

  double dist[26];
  Stack_Neighbor_Dist_R(m_workspace.conn, m_workspace.resolution, dist);
  //const double *dist = Stack_Neighbor_Dist(m_workspace.conn);
  const int *x_offset = Stack_Neighbor_X_Offset(m_workspace.conn);
  const int *y_offset = Stack_Neighbor_Y_Offset(m_workspace.conn);
  const int *z_offset = Stack_Neighbor_Z_Offset(m_workspace.conn);

  /* go forward */
  for (i = 0; i < m_workspace.conn; i++) {
    scan_mask[i] = (neighbor[i] > 0);
  }
  int swidth = cwidth + 1;
  int sarea =  (cwidth + 1) * (cheight + 1);
  int area = stack->width * stack->height;
  for (z = 0; z <= cdepth; z++) {
    for (y = 0; y <= cheight; y++) {
      for (x = 0; x <= cwidth; x++) {
        nbound = Stack_Neighbor_Bound_Test_S(m_workspace.conn, cwidth, cheight,
                                             cdepth,
                                             x, y, z, is_in_bound);
        size_t offset2 = Stack_Subindex((size_t) offset, stack_range[0],
            stack_range[2], stack_range[4], swidth, sarea, stack->width, area);

        if (nbound == m_workspace.conn) {
          STACK_GRAPH_ADD_EDGE((scan_mask[i] == 1) &&
              (m_workspace.signal_mask == NULL ? 1 :
               ((Stack_Array_Value(m_workspace.signal_mask, offset2) > 0) &&
                (Stack_Array_Value(m_workspace.signal_mask, offset2+org_neighbor[i]) >
                0))))
        } else {
          STACK_GRAPH_ADD_EDGE(((scan_mask[i] == 1) && is_in_bound[i]) &&
              (m_workspace.signal_mask == NULL ? 1 :
               ((Stack_Array_Value(m_workspace.signal_mask, offset2) > 0) &&
                (Stack_Array_Value(m_workspace.signal_mask, offset2+org_neighbor[i]) >
                0))))
        }
        if (m_workspace.group_mask != NULL) {
          if (m_workspace.group_mask->array[offset2] > 0) {
            Graph_Add_Weighted_Edge(graph, nvertex, offset, 0.0);
          }
        }

        offset++;
      }
    }
  }

  return new ZGraph(graph);
}
#endif

void ZStackGraph::setResolution(const double *res)
{
  for (int i = 0; i < 3; ++i) {
    m_workspace.resolution[i] = res[i];
  }
}
