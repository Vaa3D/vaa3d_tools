#include <iostream>
#include <sstream>
#include <fstream>

#include <string.h>
#include <stdlib.h>

#include "tz_utilities.h"
#include "zsegmentmaparray.h"
#include "zsuperpixelmaparray.h"
#include "tz_stack_lib.h"
#include "tz_image_io.h"
#include "tz_stack_attribute.h"
#include "tz_stack_document.h"
#include "tz_xml_utils.h"
#include "tz_stack_bwmorph.h"
#include "tz_stack_objlabel.h"
#include "tz_swc_tree.h"
#include "zswctree.h"
#include "zswcforest.h"
#include "tz_sp_grow.h"
#include "zspgrowparser.h"
#include "tz_stack_stat.h"
#include "tz_stack_math.h"

using namespace std;

int main(int argc, char *argv[])
{
  if (Show_Version(argc, argv, "0.1") == 1) {
    return 0;
  }

  static char const *Spec[] = {"<input:string> [-o <string>]",
    "[--length_threshold <int>] [--interpolate] [--rmborder]", 
    "[--cluster --cmd_dir <string>]",  NULL};

  Process_Arguments(argc, argv, const_cast<char**>(Spec), 1);

  char *input = Get_String_Arg(const_cast<char*>("input"));
  string inputDir(input);

  /* Skeletonization */
  printf("Read stack ...\n");
  Stack *stackData = Read_Stack(Get_String_Arg(const_cast<char*>("input")));
  /*
  Stack *out = Downsample_Stack(stackData, 1, 1, 0);
  Kill_Stack(stackData);
*/
  Stack *out = stackData;

  if (Is_Arg_Matched(const_cast<char*>("--rmborder"))) {
    printf("Remove borders ...\n");
    //stackData = Stack_Region_Expand(out, 8, 1, NULL);
    Stack_Not(out, out);
    Stack* solid = Stack_Majority_Filter(out, NULL, 8);
    Kill_Stack(out);

    Stack_Not(solid, solid);
    stackData = solid;
  }

  if (Is_Arg_Matched(const_cast<char*>("--interpolate"))) {
    printf("Interpolating ...\n");
    out = Stack_Bwinterp(stackData, NULL);
    Kill_Stack(stackData);
    stackData = out;
  }

  printf("Label objects ...\n");
  int nobj = Stack_Label_Objects_N(stackData, NULL, 1, 2, 26);
  Swc_Tree *tree = New_Swc_Tree();
  tree->root = Make_Virtual_Swc_Tree_Node();

  for (int objIndex = 0; objIndex < nobj; objIndex++) {
    printf("Skeletonizing object %d/%d\n", objIndex + 1, nobj);
    Swc_Tree *subtree = New_Swc_Tree();
    subtree->root = Make_Virtual_Swc_Tree_Node();

    Stack *objstack = Copy_Stack(stackData);
    Stack_Level_Mask(objstack, 2 + objIndex);

    printf("Build distance map ...\n");
    Stack *tmpdist = Stack_Bwdist_L_U16P(objstack, NULL, 0);
    Kill_Stack(objstack);

    printf("Shortest path grow ...\n");
    Sp_Grow_Workspace *sgw = New_Sp_Grow_Workspace();
    sgw->wf = Stack_Voxel_Weight_I;
    int max_index;
    Stack_Max(tmpdist, &max_index);

    Stack *mask = Make_Stack(GREY, Stack_Width(tmpdist),
        Stack_Height(tmpdist),
        Stack_Depth(tmpdist));
    Zero_Stack(mask);

    size_t nvoxel = Stack_Voxel_Number(stackData);
    size_t i;
    for (i = 0; i < nvoxel; i++) {
      if (stackData->array[i] == 0) {
        mask->array[i] = SP_GROW_BARRIER;
      }
    }

    mask->array[max_index] = SP_GROW_SOURCE;
    Sp_Grow_Workspace_Set_Mask(sgw, mask->array);
    Stack_Sp_Grow(tmpdist, NULL, 0, NULL, 0, sgw);

    ZSpGrowParser parser(sgw);

    int lengthThreshold = 25;
    if (Is_Arg_Matched(const_cast<char*>("--length_threshold"))) {
      lengthThreshold = Get_Int_Arg(const_cast<char*>("--length_threshold"));
    }
    cout << "Length threshold: " << lengthThreshold << endl;

    std::vector<ZVoxelArray> pathArray =
      parser.extractAllPath(lengthThreshold, tmpdist);

    for (std::vector<ZVoxelArray>::iterator iter = pathArray.begin();
        iter != pathArray.end(); ++iter) {
      (*iter).sample(tmpdist, sqrt);
      (*iter).labelStack(stackData, 255.0);

      Swc_Tree *branch = (*iter).toSwcTree();
      Swc_Tree_Connect_Branch(subtree, branch->root);
      branch->root = NULL;
      Kill_Swc_Tree(branch);
    }

    if (Swc_Tree_Has_Branch(subtree) == TRUE) {
      printf("%d nodes added\n", Swc_Tree_Node_Fsize(subtree->root));
      Swc_Tree_Merge(tree, subtree);
    }

    Kill_Swc_Tree(subtree);
    Kill_Stack(mask);
    Kill_Stack(tmpdist);
  }

  if (Swc_Tree_Has_Branch(tree)) {
    ZSwcTree treeObject;
    treeObject.setData(tree);
    ZSwcForest *forest = treeObject.toSwcTreeArray();
    forest->buildConnectionGraph(true);
    ZSwcTree *wholeTree = forest->merge();
    wholeTree->resortId();

    wholeTree->save(Get_String_Arg(const_cast<char*>("-o")));

    cout << Get_String_Arg(const_cast<char*>("-o")) << " saved" << endl;
  } else {
    std::cout << "Empty tree. No file saved";
    return 1;
  }

  return 0;
}
