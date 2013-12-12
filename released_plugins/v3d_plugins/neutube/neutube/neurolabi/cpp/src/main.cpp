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

using namespace std;

int main(int argc, char *argv[])
{
  if (Show_Version(argc, argv, "0.1") == 1) {
    return 0;
  }

  static char const *Spec[] = {"<input:string> [-o <string>]",
    "--body_id <int>", "[--length_threshold <int>]",
    "--work_dir <string>", "[--ds <int>]", "[--plane_id <int> <int>]",
    NULL};

  Process_Arguments(argc, argv, const_cast<char**>(Spec), 1);

  char *input = Get_String_Arg(const_cast<char*>("input"));
  string inputDir(input);

  if (Is_Arg_Matched(const_cast<char*>("--work_dir"))) {
    ZSegmentMapArray segmentMapArray;
    segmentMapArray.load(inputDir + "/segment_to_body_map.txt");
    int bodyId = Get_Int_Arg(const_cast<char*>("--body_id"));
    vector<int> segments = segmentMapArray.bodyToSegment(bodyId);

    for (size_t i = 0; i < segments.size(); i++) {
      cout << segments[i] << std::endl;
    }

    if (segments.size() > 1) {
      std::ostringstream outDirStream;
      outDirStream << Get_String_Arg(const_cast<char*>("--work_dir")) << "/" 
        << bodyId;

      ostringstream cmdStream;
      cmdStream << "mkdir " << outDirStream.str();
      system(cmdStream.str().c_str()); 

      /* Generate XML file*/
      ofstream xmlStream((outDirStream.str() + ".xml").c_str());
      xmlStream << "<xml>" << endl << "<data>" << endl <<
        "<image type=\"dir\">" << endl <<
        "<url>" << outDirStream.str() << "</url>" << endl <<
        "<ext>tif</ext>" << endl << "</image>" << endl <<
        "</data>" << endl << "</xml>" << endl;
      xmlStream.close();

      int planeIdStart = -1;
      int planeIdEnd = -1;
      
      if (Is_Arg_Matched(const_cast<char*>("--plane_id"))) {
        planeIdStart = Get_Int_Arg(const_cast<char*>("--plane_id"), 1);
        planeIdEnd = Get_Int_Arg(const_cast<char*>("--plane_id"), 2);
      } else {
        char filePath[500];
        sprintf(filePath, "%s/superpixel_maps", input);
        dir_fnum_pair(filePath, ".*\\.png", &planeIdStart, &planeIdEnd);
        printf("planes: %d, %d\n", planeIdStart, planeIdEnd);
      }
      
      if ((planeIdStart < 0) || (planeIdEnd < 0)) {
        printf("Invalid plane ID\n");
        return 1;
      }

      for (int planeId = planeIdStart; planeId <= planeIdEnd; planeId++) {
        cout << "Plane ID: " << planeId << endl;

        ZSuperpixelMapArray superpixelMapArray;
        superpixelMapArray.load(inputDir + "/superpixel_to_segment_map.txt",
            planeId);

        vector<int> superpixel =
          superpixelMapArray.segmentToSuperpixel(segments);

        cout << "Superpixels:" << endl;
        for (size_t i = 0; i < superpixel.size(); i++) {
          cout << superpixel[i] << endl;
        }

        if (superpixel.size() > 0) {
          char filePath[500];
          sprintf(filePath, "%s/superpixel_maps/sp_map.%05d.png", input, 
              planeId);
          cout << filePath << endl;
          Stack *stack = Read_Stack_U(filePath);
          Stack *mask = Make_Stack(GREY, stack->width, stack->height,
              stack->depth);
          Zero_Stack(mask);

          /*
          for (size_t i = 0; i < superpixel.size(); i++) {
            uint16_t *array16 = (uint16_t*) stack->array;
            size_t nvoxel = Stack_Voxel_Number(stack);
            for (size_t k = 0; k < nvoxel; k++) {
              if (array16[k] == superpixel[i]) {
                mask->array[k] = 1;
              }
            }
          }
          */
          
          for (size_t i = 0; i < superpixel.size(); i++) {
            size_t nvoxel = Stack_Voxel_Number(stack);
            switch (stack->kind) {
              case GREY16:
                {
                  uint16_t *array16 = (uint16_t*) stack->array;
                  for (size_t k = 0; k < nvoxel; k++) {
                    if (array16[k] == superpixel[i]) {
                      mask->array[k] = 1;
                    }
                  }
                }
                break;
              case COLOR:
                {
                  color_t *arrayc = (color_t*) stack->array;
                  for (size_t k = 0; k < nvoxel; k++) {
                    int value = arrayc[k][0];
                    value *= 255;
                    value += arrayc[k][1];
                    value *= 255;
                    value += arrayc[k][2]; 
                    if (value == superpixel[i]) {
                      mask->array[k] = 1;
                    }
                  }
                }
                break;
              default:
                break;
            }
          }

          Kill_Stack(stack);

          ostringstream filePathStream2;
          filePathStream2 << outDirStream.str() << "/neuron.00" << planeId 
            << ".tif";

          cout << filePathStream2.str() << endl;

          Write_Stack((char*) filePathStream2.str().c_str(), mask);
          Kill_Stack(mask);
        }
      }

      Stack_Document *doc = 
        Xml_Read_Stack_Document((outDirStream.str() + ".xml").c_str());
      File_List *list = (File_List*) doc->ci;
      Print_File_List(list);

      Stack *stack = Read_Image_List_Bounded(list);
      Stack *out = Stack_Region_Expand(stack, 8, 1, NULL);
      Kill_Stack(stack);
      int dsRate = 5;
      if (Is_Arg_Matched(const_cast<char*>("--ds"))) {
        dsRate = Get_Int_Arg(const_cast<char*>("--ds"));
      }
      stack = Downsample_Stack(out, dsRate - 1, dsRate - 1, 0);
      Write_Stack((char*) (outDirStream.str() + "_ds.tif").c_str(), stack);

      /* Skeletonization */
      Stack *stackData = stack;

      int nobj = Stack_Label_Objects_N(stackData, NULL, 1, 2, 26);
      Swc_Tree *tree = New_Swc_Tree();
      tree->root = Make_Virtual_Swc_Tree_Node();

      for (int objIndex = 0; objIndex < nobj; objIndex++) {
        Swc_Tree *subtree = New_Swc_Tree();
        subtree->root = Make_Virtual_Swc_Tree_Node();

        Stack *objstack = Copy_Stack(stackData);
        Stack_Level_Mask(objstack, 2 + objIndex);

        Stack *tmpdist = Stack_Bwdist_L_U16P(objstack, NULL, 0);
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

        Swc_Tree_Merge(tree, subtree);
        Kill_Swc_Tree(subtree);
        Kill_Stack(mask);
        Kill_Stack(tmpdist);
      }

      ZSwcTree treeObject;
      treeObject.setData(tree);
      ZSwcForest *forest = treeObject.toSwcTreeArray();
      forest->buildConnectionGraph(true);
      ZSwcTree *wholeTree = forest->merge();
      wholeTree->resortId();

      wholeTree->save((outDirStream.str() + ".swc").c_str());
    }
  }

  return 0;
}
