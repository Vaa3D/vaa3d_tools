#include "zswcpositionadjuster.h"
#include "zvoxelarray.h"
#include "swctreenode.h"
#include "c_stack.h"
#include "tz_math.h"
#include "zstackgraph.h"
#include "neutubeconfig.h"

ZSwcPositionAdjuster::ZSwcPositionAdjuster() :
  m_signal(NULL), m_mask(NULL), m_workspace(NULL),
  m_background(NeuTube::IMAGE_BACKGROUND_DARK)
{
}

ZSwcPositionAdjuster::~ZSwcPositionAdjuster()
{
  C_Stack::kill(m_mask);
  Kill_Sp_Grow_Workspace(m_workspace);
}

void ZSwcPositionAdjuster::adjustPosition(ZSwcPath &swcPath)
{
  if (m_signal == NULL) {
    return;
  }

  int sourceX = iround(SwcTreeNode::x(swcPath.front()));
  int sourceY = iround(SwcTreeNode::y(swcPath.front()));

  int targetX = iround(SwcTreeNode::x(swcPath.back()));
  int targetY = iround(SwcTreeNode::y(swcPath.back()));

  if (sourceX != targetX || sourceY != targetY) {
    Stack *mask = C_Stack::make(
          GREY, C_Stack::width(m_signal), C_Stack::height(m_signal), 1);

    Zero_Stack(mask);
    swcPath.labelStackAcrossZ(mask, 1);

#ifdef _DEBUG_2
    C_Stack::write(GET_DATA_DIR + "/test.tif", mask);
#endif

    ZStackGraph stackGraph;
    stackGraph.setSignalMaskAcrossZ(mask, C_Stack::depth(m_signal));

    Zero_Stack(mask);

    size_t sourceIndex = sourceX + C_Stack::width(mask) * sourceY;
    size_t targetIndex = targetX + C_Stack::width(mask) * targetY;
    mask->array[sourceIndex] = 1;
    mask->array[targetIndex] = 2;
    stackGraph.setGroupMaskAcrossZ(mask, C_Stack::depth(m_signal));

    if (m_background == NeuTube::IMAGE_BACKGROUND_BRIGHT) {
      stackGraph.setWeightFunction(Stack_Voxel_Weight);
    } else {
      stackGraph.setWeightFunction(Stack_Voxel_Weight_R);
    }

    std::vector<int> path = stackGraph.computeShortestPath(
          m_signal, sourceIndex, targetIndex);

    if (path.size() > 1) {
      ZVoxelArray voxelArray;
      size_t startIndex = 0;
      size_t endIndex = path.size() - 1;

      if (stackGraph.getGroupId(path[0]) == stackGraph.getGroupId(path[1])) {
        startIndex = 1;
      }

      if (stackGraph.getGroupId(path.back()) ==
          stackGraph.getGroupId(path[path.size() - 2])) {
        endIndex = path.size() - 2;
      }

      for (size_t i = startIndex; i <= endIndex; ++i) {
        int x, y, z;
        C_Stack::indexToCoord(
              path[i], C_Stack::width(m_signal), C_Stack::height(m_signal),
              &x, &y, &z);
        voxelArray.append(ZVoxel(x, y, z));
      }

      if (!voxelArray.empty()) {
        size_t currentIndex = 0;
        for (ZSwcPath::iterator iter = swcPath.begin(); iter != swcPath.end(); ++iter) {
          size_t index = voxelArray.findClosest(
                SwcTreeNode::x(*iter), SwcTreeNode::y(*iter));
          if (index > currentIndex || currentIndex == 0) {
            SwcTreeNode::setPos(*iter, voxelArray[index].x(), voxelArray[index].y(),
                                voxelArray[index].z());
            currentIndex = index;
          }
        }
      }
    }
  }
}

/*
void ZSwcPositionAdjuster::adjustPosition(ZSwcPath &swcPath)
{
  if (m_signal == NULL) {
    return;
  }

  int sourceX = iround(SwcTreeNode::x(swcPath.front()));
  int sourceY = iround(SwcTreeNode::y(swcPath.front()));

  int targetX = iround(SwcTreeNode::x(swcPath.back()));
  int targetY = iround(SwcTreeNode::y(swcPath.back()));

  if (sourceX != targetX || sourceY != targetY) {
    if (m_mask == NULL) {
      m_mask = C_Stack::make(
            GREY, C_Stack::width(m_signal), C_Stack::height(m_signal),
            C_Stack::depth(m_signal));
    }

    Zero_Stack(m_mask);
    uint8_t barrier = SP_GROW_BARRIER;
    Stack_Set_Constant(m_mask, &barrier);

    swcPath.labelStackAcrossZ(m_mask, 0);
    if (m_workspace != NULL) {
      Kill_Sp_Grow_Workspace(m_workspace);
    }

    m_workspace = New_Sp_Grow_Workspace();
    Sp_Grow_Workspace_Set_Mask(m_workspace, m_mask->array);
    if (m_background == NeuTube::IMAGE_BACKGROUND_BRIGHT) {
      m_workspace->wf = Stack_Voxel_Weight;
    } else {
      m_workspace->wf = Stack_Voxel_Weight_R;
    }

    uint8_t *array = C_Stack::array8(m_mask);
    size_t area = C_Stack::area(m_mask);
    size_t sourceIndex = sourceX + C_Stack::width(m_mask) * sourceY;
    size_t targetIndex = targetX + C_Stack::width(m_mask) * targetY;
    for (int z = 0; z < C_Stack::depth(m_signal); ++z) {
      array[sourceIndex] = SP_GROW_SOURCE;
      array[targetIndex] = SP_GROW_TARGET;
      sourceIndex += area;
      targetIndex += area;
    }

    Int_Arraylist *path = Stack_Sp_Grow(m_signal, NULL, 0, NULL, 0, m_workspace);
    ZVoxelArray voxelArray;

    for (int i = 0; i < path->length; ++i) {
      int x, y, z;
      C_Stack::indexToCoord(
            path->array[i], C_Stack::width(m_signal), C_Stack::height(m_signal),
            &x, &y, &z);
      bool isRedundant = false;
      if (m_mask->array[path->array[i]] == SP_GROW_SOURCE) {
        if (i < path->length - 1) {
          if (m_mask->array[i + 1] == SP_GROW_SOURCE) {
            isRedundant = true;
          }
        }
      }
      if (!isRedundant) {
        voxelArray.append(ZVoxel(x, y, z));
      }
    }
    Kill_Int_Arraylist(path);

    size_t currentIndex = 0;
    for (ZSwcPath::iterator iter = swcPath.begin(); iter != swcPath.end(); ++iter) {
      size_t index = voxelArray.findClosest(
            SwcTreeNode::x(*iter), SwcTreeNode::y(*iter));
      if (index > currentIndex || currentIndex == 0) {
        SwcTreeNode::setPos(*iter, voxelArray[index].x(), voxelArray[index].y(),
                            voxelArray[index].z());
        currentIndex = index;
      }
    }
  }
}
*/

void ZSwcPositionAdjuster::adjustPosition(ZSwcTree &tree)
{
  startProgress();
  std::vector<ZSwcPath> branchArray = tree.getBranchArray();
  for (std::vector<ZSwcPath>::iterator iter = branchArray.begin();
       iter != branchArray.end(); ++iter) {
    adjustPosition(*iter);
    advanceProgress(1.0 / branchArray.size());
  }
  endProgress();
}
