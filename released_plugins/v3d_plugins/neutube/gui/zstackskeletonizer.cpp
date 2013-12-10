#include "zstackskeletonizer.h"

#include <iostream>
#include "c_stack.h"
#include "zswctree.h"
#include "tz_stack_lib.h"
#include "tz_stack_bwmorph.h"
#include "tz_sp_grow.h"
#include "tz_stack_objlabel.h"
#include "zspgrowparser.h"
#include "tz_stack_stat.h"
#include "tz_stack_math.h"
#include "zswcforest.h"
#include "swctreenode.h"
#include "zswcgenerator.h"

using namespace std;

ZStackSkeletonizer::ZStackSkeletonizer() : m_lengthThreshold(15.0),
  m_distanceThreshold(-1.0), m_rebase(false), m_interpolating(false),
  m_removingBorder(false), m_minObjSize(0), m_keepingSingleObject(false),
  m_level(0), m_connectingBranch(true)
{
  m_resolution[0] = 1.0;
  m_resolution[1] = 1.0;
  m_resolution[2] = 1.0;
}

ZSwcTree* ZStackSkeletonizer::makeSkeleton(const Stack *stack)
{
  startProgress();
  Stack *stackData = C_Stack::clone(stack);

  if (m_level > 0) {
    Stack_Level_Mask(stackData, m_level);
    Translate_Stack(stackData, GREY, 1);
  }
  advanceProgress(0.05);

  if (Stack_Max(stackData, NULL) != 1) {
    cout << "Not a binary image. No skeleton generated." << endl;
    return NULL;
  }
  advanceProgress(0.05);

  Stack *out = stackData;

  if (m_removingBorder) {
    cout << "Remove 1-pixel gaps ..." << endl;
    Stack_Not(out, out);
    Stack* solid = Stack_Majority_Filter(out, NULL, 8);
    Kill_Stack(out);

    Stack_Not(solid, solid);
    stackData = solid;
  }
  advanceProgress(0.05);

  if (m_interpolating) {
    cout << "Interpolating ..." << endl;
    out = Stack_Bwinterp(stackData, NULL);
    Kill_Stack(stackData);
    stackData = out;
  }
  advanceProgress(0.05);

  cout << "Label objects ...\n" << endl;
  int nobj = Stack_Label_Large_Objects_N(
        stackData, NULL, 1, 2, m_minObjSize, 26);
  //int nobj = Stack_Label_Objects_N(stackData, NULL, 1, 2, 26);
  if (nobj == 0) {
    cout << "No object found in the image. No skeleton generated." << endl;
    Kill_Stack(stackData);
    return NULL;
  }

  advanceProgress(0.1);

  if (nobj > 65533) {
    cout << "Too many objects ( > 65533). No skeleton generated." << endl;
    Kill_Stack(stackData);
    return NULL;
  }

  Swc_Tree *tree = New_Swc_Tree();
  tree->root = Make_Virtual_Swc_Tree_Node();

  size_t voxelNumber = C_Stack::voxelNumber(stackData);

  double step = 0.6 / nobj;
  for (int objIndex = 0; objIndex < nobj; objIndex++) {
    cout << "Skeletonizing object " << objIndex + 1 << "/" << nobj << endl;
    Swc_Tree *subtree = New_Swc_Tree();
    subtree->root = Make_Virtual_Swc_Tree_Node();

    Stack *objstack = Copy_Stack(stackData);
    size_t objSize = Stack_Level_Mask(objstack, 3 + objIndex);
    if (C_Stack::kind(objstack) == GREY16) {
      Translate_Stack(objstack, GREY, 1);
    }

    if (objSize == 1) {
      if (m_keepingSingleObject || m_lengthThreshold <= 1) {
        int x = 0;
        int y = 0;
        int z = 0;
        for (size_t offset = 0; offset < voxelNumber; ++offset) {
          if (objstack->array[offset] == 1) {
            C_Stack::indexToCoord(offset, C_Stack::width(objstack),
                                  C_Stack::height(objstack), &x, &y, &z);
            break;
          }
        }
        Swc_Tree_Node *tn = SwcTreeNode::makePointer(x, y, z, 1.0);
        SwcTreeNode::setParent(tn, subtree->root);
      }
    } else {
      cout << "Build distance map ..." << endl;
      Stack *tmpdist = Stack_Bwdist_L_U16P(objstack, NULL, 0);

      cout << "Shortest path grow ..." << endl;
      Sp_Grow_Workspace *sgw = New_Sp_Grow_Workspace();
      sgw->wf = Stack_Voxel_Weight_I;
      size_t max_index;
      Stack_Max(tmpdist, &max_index);

      Stack *mask = Make_Stack(GREY, Stack_Width(tmpdist),
                               Stack_Height(tmpdist),
                               Stack_Depth(tmpdist));
      Zero_Stack(mask);

      size_t nvoxel = Stack_Voxel_Number(stackData);
      size_t i;
      for (i = 0; i < nvoxel; i++) {
        if (objstack->array[i] == 0) {
          mask->array[i] = SP_GROW_BARRIER;
        }
      }

      mask->array[max_index] = SP_GROW_SOURCE;
      Sp_Grow_Workspace_Set_Mask(sgw, mask->array);
      Stack_Sp_Grow(tmpdist, NULL, 0, NULL, 0, sgw);

      ZSpGrowParser parser(sgw);

      if (m_rebase) {
        cout << "Replacing start point ..." << endl;
        ZVoxelArray path = parser.extractLongestPath(NULL);
        for (i = 0; i < nvoxel; i++) {
          if (mask->array[i] != SP_GROW_BARRIER) {
            mask->array[i] = 0;
          }
        }

        ssize_t seedIndex = path[0].toIndex(
              C_Stack::width(mask), C_Stack::height(mask),
              C_Stack::depth(mask));
        mask->array[seedIndex] = SP_GROW_SOURCE;
        Stack_Sp_Grow(tmpdist, NULL, 0, NULL, 0, sgw);
      }

      double lengthThreshold = m_lengthThreshold;

      std::vector<ZVoxelArray> pathArray =
          parser.extractAllPath(lengthThreshold, tmpdist);

      if (pathArray.empty() && m_keepingSingleObject) {
        pathArray.push_back(parser.extractLongestPath(NULL));
      }

      //Make a subtree from a single object
      for (std::vector<ZVoxelArray>::iterator iter = pathArray.begin();
           iter != pathArray.end(); ++iter) {
        (*iter).sample(tmpdist, sqrt);
        //(*iter).labelStack(stackData, 255.0);

        ZSwcTree *branchWrapper =
            ZSwcGenerator::createSwc(*iter, ZSwcGenerator::NO_PROCESS);
        Swc_Tree *branch  = branchWrapper->data();
        branchWrapper->setData(branch, ZSwcTree::LEAVE_ALONE);

        //branch = (*iter).toSwcTree();
#if 1
        if (SwcTreeNode::firstChild(branch->root) != NULL) {
          if (SwcTreeNode::radius(branch->root) * 2.0 <
              SwcTreeNode::radius(SwcTreeNode::firstChild(branch->root))) {
            Swc_Tree_Node *oldRoot = branch->root;
            Swc_Tree_Node *newRoot = SwcTreeNode::firstChild(branch->root);
            SwcTreeNode::detachParent(newRoot);
            branch->root = newRoot;
            SwcTreeNode::kill(oldRoot);
          }
        }

        Swc_Tree_Node *leaf = SwcTreeNode::firstChild(branch->root);
        if (leaf != NULL) {
          while (SwcTreeNode::firstChild(leaf) != NULL) {
            leaf = SwcTreeNode::firstChild(leaf);
          }

          if (SwcTreeNode::radius(leaf) * 2.0 <
              SwcTreeNode::radius(SwcTreeNode::parent(leaf))) {
            SwcTreeNode::detachParent(leaf);
            SwcTreeNode::kill(leaf);
          }
        }

        Swc_Tree_Node *tn = Swc_Tree_Connect_Branch(subtree, branch->root);

        if (SwcTreeNode::isRegular(SwcTreeNode::parent(tn))) {
          if (SwcTreeNode::hasOverlap(tn, SwcTreeNode::parent(tn))) {
            SwcTreeNode::mergeToParent(tn);
          }
        }
#else

        SwcTreeNode::setParent(branch->root, subtree->root);
#endif

        branch->root = NULL;
        Kill_Swc_Tree(branch);
      }

      Kill_Stack(mask);
      Kill_Stack(tmpdist);
    }

    C_Stack::kill(objstack);

    if (Swc_Tree_Regular_Root(subtree) != NULL) {
      cout << Swc_Tree_Node_Fsize(subtree->root) - 1<< " nodes added" << endl;
      Swc_Tree_Merge(tree, subtree);
    }

    Kill_Swc_Tree(subtree);

    advanceProgress(step);
  }

  ZSwcTree *wholeTree = NULL;

  if (Swc_Tree_Regular_Root(tree) != NULL) {
    wholeTree = new ZSwcTree;
    wholeTree->setData(tree);
    wholeTree->resortId();
    if (m_connectingBranch) {
      reconnect(wholeTree);
    }
  }

  advanceProgress(0.05);
  endProgress();

  return wholeTree;
}

void ZStackSkeletonizer::reconnect(ZSwcTree *tree)
{
  if (tree->regularRootNumber() > 1) {
    double z_scale = m_resolution[2];
    if (m_resolution[0] != 1.0) {
      z_scale /= m_resolution[0];
    }
    Swc_Tree_Reconnect(tree->data(), z_scale,
                       m_distanceThreshold / m_resolution[0]);
    tree->resortId();
  }
}
