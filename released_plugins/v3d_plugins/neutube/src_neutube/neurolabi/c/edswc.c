/**@file edswc.c
 * @brief >> modify swc file
 * @author Ting Zhao
 * @date 01-Aug-2009
 */

#include <string.h>
#include "utilities.h"
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include "tz_swc_tree.h"
#include "tz_utilities.h"
#include "tz_error.h"
#include "tz_math.h"
#include "tz_darray.h"
#include "tz_u8array.h"
#include "tz_iarray.h"
#include "tz_xml_utils.h"
#include "tz_random.h"
#include "tz_geo3d_utils.h"
#include "tz_stack_graph.h"
#include "tz_stack_threshold.h"
#include "tz_stack_attribute.h"
#include "tz_stack_utils.h"
#include "tz_stack_io.h"
#include "tz_stack_stat.h"
#include "tz_int_histogram.h"
#include "tz_apo.h"
#include "tz_geo3d_vector.h"

#include "private/tz_main.c"
#include "private/edswc.c"


FILE *open_file(const char *path, const char *mode);

static int skip_argument(char *argv[])
{
  if (eqstr(argv[0], "-o")) {
    return 2;
  }

  if (eqstr(argv[0], "-root")) {
    return 4;
  }

  if (eqstr(argv[0], "-clean_root")) {
    return 1;
  }

  if (eqstr(argv[0], "-tune_fork")) {
    return 1;
  }

  if (eqstr(argv[0], "-type")) {
    return 2;
  }

  if (eqstr(argv[0], "-remove_overshoot")) {
    return 1;
  }

  if (eqstr(argv[0], "-mt")) {
    return 2;
  }

  if (eqstr(argv[0], "-soma")) {
    return 2;
  }

  if (eqstr(argv[0], "-decompose")) {
    return 1;
  }

  if (eqstr(argv[0], "-resize")) {
    return 4;
  }

  if (eqstr(argv[0], "-merge")) {
    return 2;
  }

  if (eqstr(argv[0], "-cut")) {
    return 2;
  }

  if (eqstr(argv[0], "-info")) {
    return 2;
  }

  if (eqstr(argv[0], "-apo")) {
    return 2;
  }

  if (eqstr(argv[0], "-translate")) {
    return 4;
  }

  if (eqstr(argv[0], "-glue")) {
    return 1;
  }

  if (eqstr(argv[0], "-search")) {
    return 4;
  }

  if (eqstr(argv[0], "-shuffle")) {
    return 1;
  }

  if (eqstr(argv[0], "-pcount")) {
    return 1;
  }

  if (eqstr(argv[0], "-select")) {
    return 3;
  }

  if (eqstr(argv[0], "-color")) {
    return 2;
  }

  if (eqstr(argv[0], "-feat_file")) {
    return 2;
  }

  if (eqstr(argv[0], "-signal")) {
    return 2;
  }

  if (eqstr(argv[0], "-clean_small")) {
    return 2;
  }

  if (eqstr(argv[0], "-sigmargin")) {
    return 2;
  }

  if (eqstr(argv[0], "-cross_talk")) {
    return 1;
  }

  if (eqstr(argv[0], "-prune_small")) {
    return 2;
  }

  if (eqstr(argv[0], "-prune_short")) {
    return 2;
  }

  if (eqstr(argv[0], "-threshold")) {
    return 2;
  }

  if (eqstr(argv[0], "-rotate")) {
    return 6;
  }

  if (eqstr(argv[0], "-yzswitch")) {
    return 1;
  }
  /*
  if (strcmp(arg[0],
    "[-pyramidal] [-analysis] [-feat_file <string>] [-signal <string>] [-bluechannel <int>]",
    "[-clean_small <double>] [-sigmargin <double>]",
    */
  return 1;
}

static int help(int argc, char *argv[], char *spec[])
{
  if (argc == 2) {
    if (strcmp(argv[1], "--help") == 0) {
      printf("\nedswc ");
      Print_Argument_Spec(spec);

      printf("\nDetails\n");
      printf("input: input swc file.\n");
      printf("-o: output swc or dot file.\n");
      printf("-root: coordinate of the root.\n");
      printf("-clean_root: cut off branches of the root to make sure it has only one child.\n");
      printf("-tune_fork: tune branch points to make it more reasonable (not guaranteed to work).\n");
      printf("-type: change the type of the tree\n");
      printf("-remove_overshoot: find and remove overshoots");
      printf("-mt: label the main trunk with a certain type.");
      printf("-decompose: decompose the tree into individual branches and each branch is saved as a separate SWC file. When this argument is specified, the -o option will be taken as the prefix of the output files.\n");
      printf("-translate: translate the input tree. It is always done before resizing.\n");
      printf("-single: extract the biggest tree.\n");
      printf("-resize: rescale the input tree.\n");
      printf("-merge: reduce a tree by merging close nodes.\n");
      printf("-cut: cut nodes with specified IDs.\n");
      printf("-apo: supply apo files while producing dendrogram.\n");
      printf("-glue: glue trees.\n");
      printf("-search: search a tree at a given position.\n");
      printf("-info: print information of a tree.\n");
      printf("-shuffle: shuffle the punctas.\n");
      printf("-pcount: show counting in svg file.\n");
      printf("-subtract: subtract a tree from another.\n");
      printf("-pyramidal: process the input swc tree as a pyramidal cell.\n");
      printf("-length: show length in svg.\n");
      printf("-analysis: turn an swc file and an apo file to files for analysis.\n");
      printf("-cross_talk: identify cross talk and label the possible cross talk branches as a center value.\n");
      printf("-prune_small: prune node with small sizes (<= the specified value.)");
      printf("-rotate: rotate an swc tree (theta, psi, cx, cy, cz)");
      printf("\n");

      return 1;
    }
  }

  return 0;
}


#define SVG_VIEW_MAX_X 800
#define SVG_VIEW_MAX_Y 600

void Swc_Tree_Merge_Root_Group_Type(Swc_Tree *tree, int label)
{
  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_BREADTH_FIRST, FALSE);

  Swc_Tree_Node *tn = tree->begin;
  Swc_Tree_Node *next = NULL;
  while (tn != NULL) {
    next = tn->next;
    if (Swc_Tree_Node_Is_Regular_Root(tn->parent) == TRUE) {
      if ((Swc_Tree_Node_Type(tn->parent) == label) &&
    (Swc_Tree_Node_Type(tn) == label)) {
  Swc_Tree_Node_Merge_To_Parent(tn);
      }
    }
    tn = next;
  }
}

int main(int argc, char *argv[])
{
  if (Show_Version(argc, argv, "1.48") == 1) {
    return 0;
  }

  static char *Spec[] = {"<input:string> ... [-o <string>]",
    "[-root <double> <double> <double>]",
    "[-clean_root] [-tune_fork] [-type <int>] [-single]",
    "[-select <int> <int>]",
    "[-remove_overshoot] [-mt <int>] [-soma <int>]",
    "[-translate <double> <double> <double>]",
    "[-decompose] [-resize <double> <double> <double>]",
    "[-glue | -compose]", "[-search <double> <double> <double>]",
    "[-merge <double>] [-cut <int>] ...",
    "[-info <string>]", "[-apo <string>] ...",
    "[-shuffle]", "[-pcount] [-length]", "[-subtract]", "[-color <string>]",
    "[-pyramidal] [-analysis] [-feat_file <string>] [-signal <string>] [-bluechannel <int>]",
    "[-clean_small <double>] [-sigmargin <double>]",
    "[-cross_talk] [-threshold <double>]",
    "[-removeSmallLeaf <int(1)>]", "[-prune_small <double>]",
    "[-rotate <double> <double> <double> <double> <double>]",
    "[-yzswitch]",
    "[-prune_short <double>]",
    NULL};

  if (help(argc, argv, Spec) == 1) {
    return 0;
  }


  int new_argc = argc;
  if (argc >= 25) {
    new_argc = rearrange_arguments(argc, argv);
  }

  Process_Arguments(new_argc, argv, Spec, 1);

  int nfile = 0;

  if (new_argc != argc) {
    nfile = argc - new_argc + 1;
  } else {
    nfile = Get_Repeat_Count("input");
  }

  char **filepath = (char**) malloc(sizeof(char*) * nfile);

  int k;
  if (new_argc != argc) {
    for (k = 0; k < nfile; k++) {
      filepath[k] = argv[new_argc + k - 1];
    }
  } else {
    for (k = 0; k < nfile; k++) {
      filepath[k] = Get_String_Arg("input", k);
    }
  }

  /* convert apo file to swc or marker file */
  if (fhasext(filepath[0], "apo")) {
    Geo3d_Scalar_Field *field = Geo3d_Scalar_Field_Import_Apo_E(filepath[0], 4);
    if (field != NULL) {
      if (fhasext(Get_String_Arg("-o"), "marker")) {
        geo3d_scalar_field_export_marker(field, Get_String_Arg("-o"));
      } else {
        geo3d_scalar_field_export_swc(field, Get_String_Arg("-o"));
      }
    } else {
      return 1;
    }
    return 0;
  }
  /*********************************/

  Swc_Tree *tree = Read_Swc_Tree(filepath[0]);

  if (Is_Arg_Matched("-prune_small")) {
    Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);
    Swc_Tree_Node *tn = Swc_Tree_Next(tree);
    while (tn != NULL) {
      Swc_Tree_Node *next = tn->next;
      if (Swc_Tree_Node_Is_Regular(tn)) {
        if (Swc_Tree_Node_Radius(tn) <= Get_Double_Arg("-prune_small")) {
          Swc_Tree_Node_Merge_To_Parent(tn);
        }
      }
      tn = next;
    }
  }

  if (Is_Arg_Matched("-prune_short")) {
    Swc_Tree_Remove_Terminal_Branch(tree, Get_Double_Arg("-prune_short"));
  }

  if (Is_Arg_Matched("-subtract")) {
    swc_tree_subtract(tree, filepath, nfile);
    Write_Swc_Tree(Get_String_Arg("-o"), tree);
    return 0;
  }

  if (Is_Arg_Matched("-cross_talk")) {
    Swc_Tree *source_tree = Read_Swc_Tree(filepath[1]);

    int n;
    double *score = Swc_Tree_Cross_Talk_Score(tree, source_tree, &n);

    //darray_print2(score, n, 1);

    Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);

    Swc_Tree_Node *tn = NULL;
    double threshold = Get_Double_Arg("-threshold");
    while ((tn = Swc_Tree_Next(tree)) != NULL) {
      if (score[Swc_Tree_Node_Label(tn) - 1] > threshold) {
        tn->node.label = 1;
        if (Swc_Tree_Node_Is_Regular_Root(tn->parent)) {
          Swc_Tree_Node_Set_Label(tn->parent, 1);
        }
      } else {
        tn->node.label = 0;
      }
    }

    Swc_Tree_Remove_Labeled(tree, 1);

    Swc_Tree_Resort_Id(tree);

    Write_Swc_Tree(Get_String_Arg("-o"), tree);

    free(score);

    return 0;
  }

  double search_pos[3];
  double mindist;
  int minindex = 0;
  if (Is_Arg_Matched("-search")) {
    search_pos[0] = Get_Double_Arg("-search", 1);
    search_pos[1] = Get_Double_Arg("-search", 2);
    search_pos[2] = Get_Double_Arg("-search", 3);
    mindist = Swc_Tree_Point_Dist(tree, search_pos[0], search_pos[1],
				  search_pos[2], NULL, NULL);
  }

  int i;
  BOOL node_changed = FALSE;

  int tree_type = 0;

  for (i = 1; i < nfile; i++) {
    Swc_Tree *tree2 = Read_Swc_Tree(filepath[i]);

    if (Is_Arg_Matched("-search")) {
      double dist = Swc_Tree_Point_Dist(tree2, search_pos[0], search_pos[1],
          search_pos[2], NULL, NULL);
      if (dist < mindist) {
        mindist = dist;
        minindex = i;
      }
    } else {
      if (Is_Arg_Matched("-glue")) {
        Swc_Tree_Glue(tree, Swc_Tree_Regular_Root(tree2));
        Swc_Tree_Merge_Close_Node(tree, 0.01);
      } else {
        if (Is_Arg_Matched("-compose")) {
          Swc_Tree_Set_Type(tree2, tree_type % 13);
          tree_type++;
        }
        Swc_Tree_Merge(tree, tree2);
      }
    }

    Kill_Swc_Tree(tree2);
    node_changed = TRUE;
  }

  if (Is_Arg_Matched("-search")) {
    printf("Search result: %s\n", filepath[minindex]);
  }
#ifdef _DEBUG_2
  Swc_Tree_To_Dot_File(tree, "../data/test.dot");
#endif

  int ncut = Get_Repeat_Count("-cut");

  for (i = 0; i < ncut; i++) {
    Swc_Tree_Iterator_Start(tree, 2, TRUE);
    Swc_Tree_Node *tn = NULL;
    while ((tn = Swc_Tree_Next(tree)) != NULL) {
      if (Swc_Tree_Node_Id(tn) == Get_Int_Arg("-cut", i)) {
        Swc_Tree_Node_Kill_Subtree(tn);
        node_changed = TRUE;
        break;
      }
    }
  }

  if (Is_Arg_Matched("-root")) {
    double root[3];
    root[0] = Get_Double_Arg("-root", 1);
    root[1] = Get_Double_Arg("-root", 2);
    root[2] = Get_Double_Arg("-root", 3);
    Swc_Tree_Node *tn = Swc_Tree_Closest_Node(tree, root);
    Swc_Tree_Node_Set_Root(tn);
    node_changed = TRUE;
  }

  if (Is_Arg_Matched("-clean_root")) {
    Swc_Tree_Clean_Root(tree);
    node_changed = TRUE;
  }

  if (Is_Arg_Matched("-remove_overshoot")) {
    Swc_Tree_Remove_Overshoot(tree);
  }

  if (Is_Arg_Matched("-tune_fork")) {
    Swc_Tree_Tune_Fork(tree);
    node_changed = TRUE;
  }

  if (Is_Arg_Matched("-type")) {
    int type = Get_Int_Arg("-type");

    Swc_Tree_Iterator_Start(tree, 1, FALSE);

    /* select a branch */
    if (Is_Arg_Matched("-select")) {
      Swc_Tree_Node *begin = Swc_Tree_Query_Node(tree,
          Get_Int_Arg("-select", 1), SWC_TREE_ITERATOR_NO_UPDATE);
      Swc_Tree_Node *end = Swc_Tree_Query_Node(tree, Get_Int_Arg("-select", 2),
          SWC_TREE_ITERATOR_NO_UPDATE);

      if ((begin == NULL) || (end == NULL)) {
        fprintf(stderr, "Invalid selection ID.\n");
        return 1;
      }

      Swc_Tree_Iterator_Path(tree, begin, end);
    }

    Swc_Tree_Node *tn;
    if (type >= 0) {
      while ((tn = Swc_Tree_Next(tree)) != NULL) {
        Swc_Tree_Node_Data(tn)->type = type;
      }
    } else if (type == -1) {
      type = 0;
      while ((tn = Swc_Tree_Next(tree)) != NULL) {
        Swc_Tree_Node_Data(tn)->type = type + 2;
        if (!(Swc_Tree_Node_Is_Continuation(tn) ||
              Swc_Tree_Node_Is_Root(tn))) {
          type++;
          type = type % 11;
        }
      }
    }
  }

  if (Is_Arg_Matched("-yzswitch")) {
    Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);
    Swc_Tree_Node *tn = NULL;
    while ((tn = Swc_Tree_Next(tree)) != NULL) {
      double tmpY = tn->node.y;
      tn->node.y = tn->node.z;
      tn->node.z = tmpY;
    }
  }

  if (Is_Arg_Matched("-rotate")) {
    double theta = Get_Double_Arg("-rotate", 1) * TZ_PI / 180.0;
    double psi = Get_Double_Arg("-rotate", 2) * TZ_PI / 180.0;
    double cx = Get_Double_Arg("-rotate", 3);
    double cy = Get_Double_Arg("-rotate", 4);
    double cz = Get_Double_Arg("-rotate", 5);
    Swc_Tree_Rotate(tree, theta, psi, cx, cy, cz);
  }

  if (Is_Arg_Matched("-translate")) {
    double x = Get_Double_Arg("-translate", 1);
    double y = Get_Double_Arg("-translate", 2);
    double z = Get_Double_Arg("-translate", 3);
    swc_tree_translate(tree, x, y, z);
  }

  if (Is_Arg_Matched("-clean_small")) {
    double threshold = Get_Double_Arg("-clean_small");
    swc_tree_clean_small(tree, threshold);
    printf("%g\n", Swc_Tree_Overall_Length(tree));
    node_changed = TRUE;
  }

  if (Is_Arg_Matched("-resize")) {
    double x_scale = Get_Double_Arg("-resize", 1);
    double y_scale = Get_Double_Arg("-resize", 2);
    double z_scale = Get_Double_Arg("-resize", 3);
    swc_tree_resize(tree, x_scale, y_scale, z_scale);
  }

  if (Is_Arg_Matched("-merge")) {
    Swc_Tree_Merge_Close_Node(tree, Get_Double_Arg("-merge"));
    node_changed = TRUE;
  }

  if (Is_Arg_Matched("-single")) {
    if (Swc_Tree_Number(tree) > 1) {
      Swc_Tree_Node *new_root = tree->root->first_child;
      int max_size = Swc_Tree_Node_Fsize(new_root);
      Swc_Tree_Node *tmp_tn = new_root->next_sibling;
      while (tmp_tn != NULL) {
        int tmp_size = Swc_Tree_Node_Fsize(tmp_tn);
        if (tmp_size > max_size) {
          max_size = tmp_size;
          new_root = tmp_tn;
        }
        tmp_tn = tmp_tn->next_sibling;
      }
      Swc_Tree_Node_Detach_Parent(new_root);
      Clean_Swc_Tree(tree);
      tree->root = new_root;
    }
  }

  if (Is_Arg_Matched("-mt")) {
    Swc_Tree_Label_Main_Trunk_L(tree, 5, 300.0, 1600.0);
    Swc_Tree_Iterator_Start(tree, 1, FALSE);
    Swc_Tree_Node *tn;
    while ((tn = Swc_Tree_Next(tree)) != NULL) {
      if (Swc_Tree_Node_Data(tn)->label == 5) {
        Swc_Tree_Node_Data(tn)->type = Get_Int_Arg("-mt");
      }
    }
  }


  if (Is_Arg_Matched("-soma")) {
    /* under development */
    //Swc_Tree_Label_Soma(tree, 6, 300.0, 1600.0);
    Swc_Tree_Iterator_Start(tree, 1, FALSE);
    Swc_Tree_Node *tn;
    while ((tn = Swc_Tree_Next(tree)) != NULL) {
      if (Swc_Tree_Node_Data(tn)->label == 6) {
        Swc_Tree_Node_Data(tn)->type = Get_Int_Arg("-soma");
      }
    }
  }

  Swc_Tree *soma_tree = NULL;
  Swc_Tree *out_tree = NULL;
  if (Is_Arg_Matched("-pyramidal")) {
    Swc_Tree_Grow_Soma(tree, 1);
    soma_tree = Copy_Swc_Tree(tree);
    out_tree = Copy_Swc_Tree(tree);
    Swc_Tree_Resort_Pyramidal(tree, FALSE, TRUE);
    Swc_Tree_Resort_Pyramidal(out_tree, FALSE, FALSE);
    Swc_Tree_Set_Type_As_Label(tree);
    Swc_Tree_Set_Type_As_Label(out_tree);
    Swc_Tree_Resort_Id(tree);
    Swc_Tree_Resort_Id(out_tree);
  } else {
    soma_tree = Copy_Swc_Tree(tree);
  }


  if (node_changed == TRUE) {
    Swc_Tree_Resort_Id(tree);
  }

  if (Is_Arg_Matched("-o")) {
    if (Is_Arg_Matched("-decompose")) {
      swc_tree_decompose(tree, Get_String_Arg("-o"));
    } else {
      if (fhasext(Get_String_Arg("-o"), "dot")) {
        Swc_Tree_To_Dot_File(tree, Get_String_Arg("-o"));
      } else if (fhasext(Get_String_Arg("-o"), "svg")) {
        Swc_Tree_Svg_Workspace *ws = New_Swc_Tree_Svg_Workspace();
        ws->max_vx = SVG_VIEW_MAX_X;
        ws->max_vy = SVG_VIEW_MAX_Y;
        if (Is_Arg_Matched("-length")) {
          ws->showing_length = TRUE;
        }
        int napo = Get_Repeat_Count("-apo");
        int *puncta_number = iarray_malloc(napo);
        if (napo > 0) {
          ws->puncta = Geo3d_Scalar_Field_Import_Apo(Get_String_Arg("-apo", 0));
          puncta_number[0] = ws->puncta->size;
          for (i = 1; i < napo; i++) {
            Geo3d_Scalar_Field *puncta =
              Geo3d_Scalar_Field_Import_Apo(Get_String_Arg("-apo", i));
            puncta_number[i] = puncta_number[i-1] + puncta->size;
            Geo3d_Scalar_Field_Merge(ws->puncta, puncta, ws->puncta);
            Kill_Geo3d_Scalar_Field(puncta);
          }

          ws->puncta_type = iarray_calloc(ws->puncta->size);

          int cur_type = 0;
          for (i = 0; i < ws->puncta->size; i++) {
            if (i >= puncta_number[cur_type]) {
              cur_type++;
            }
            ws->puncta_type[i] = cur_type;
          }

          if (Is_Arg_Matched("-shuffle")) {
            ws->shuffling = TRUE;
          }

          if (Is_Arg_Matched("-pcount")) {
            ws->showing_count = TRUE;
          }

          if ((soma_tree != NULL) && (ws->puncta != NULL)) {
            if (ws->puncta->size > 0) {
              GUARDED_CALLOC_ARRAY(ws->on_root, ws->puncta->size, BOOL);
              Swc_Tree_Identify_Puncta(soma_tree, ws->puncta, 1, ws->on_root);
            }
          }
        }

        if (Is_Arg_Matched("-color")) {
          if (strcmp(Get_String_Arg("-color"), "depth") == 0) {
            ws->color_code = SWC_SVG_COLOR_CODE_Z;
            Swc_Tree_Z_Feature(tree);
          } else if (strcmp(Get_String_Arg("-color"), "surface") == 0) {
            ws->color_code = SWC_SVG_COLOR_CODE_SURFAREA;
            Swc_Tree_Perimeter_Feature(tree);
          } else if (fhasext(Get_String_Arg("-color"), "tif")) {
            ws->color_code = SWC_SVG_COLOR_CODE_INTENSITY;
            Stack *swc_signal = Read_Stack(Get_String_Arg("-color"));
            Swc_Tree_Intensity_Feature(tree, swc_signal, NULL);
            Kill_Stack(swc_signal);
          } else {
            fprintf(stderr, "Invalid color option: %s. "
                "Color coding is ignored.\n", Get_String_Arg("-color"));
          }
        }
        Swc_Tree_To_Svg_File_W(tree, Get_String_Arg("-o"), ws);
      } else {
        int napo = Get_Repeat_Count("-apo");
        if (napo > 0) {
          Geo3d_Scalar_Field *puncta = New_Geo3d_Scalar_Field();
          puncta = Geo3d_Scalar_Field_Import_Apo(Get_String_Arg("-apo", 0));
          Swc_Tree_Puncta_Feature(tree, puncta);
          Kill_Geo3d_Scalar_Field(puncta);
          Swc_Tree_Iterator_Start(tree, 2, FALSE);
          int color_map[] = { 1, 3, 5, 7, 6, 8, 2 };
          Swc_Tree_Node *tn = NULL;
          while ((tn = Swc_Tree_Next(tree)) != NULL) {
            int new_type = iround(tn->feature);
            if (new_type > 6) {
              new_type = 6;
            }
            Swc_Tree_Node_Data(tn)->type = color_map[new_type];
          }
        }
        if (Is_Arg_Matched("-signal")) {
          Stack *signal = Read_Sc_Stack(Get_String_Arg("-signal"), Get_Int_Arg("-bluechannel")-1);
          if (signal != NULL) {
          double margin = 0.0;
          if (Is_Arg_Matched("-sigmargin")) {
            margin = Get_Double_Arg("-sigmargin");
          }
          double fmin, fmax;
          Swc_Tree_Feature_Range(tree, &fmin, &fmax);
          Swc_Tree_Iterator_Start(tree, 2, FALSE);
          int color_map[] = { 1, 3, 5, 7, 6, 8, 2 };
          Swc_Tree_Node *tn = NULL;
          while ((tn = Swc_Tree_Next(tree)) != NULL) {
            int new_type = iround((tn->feature - fmin) / (fmax - fmin) * 6.0);
            if (new_type > 6) {
              new_type = 6;
            }
            Swc_Tree_Node_Data(tn)->type = color_map[new_type];
            if (Is_Arg_Matched("-o")) {
              if (fhasext(Get_String_Arg("-o"), "swc")) {
                Swc_Tree_Node_Data(tn)->d += 5.0;
              }
            }
          }

          Kill_Stack(signal);
          }
        }

        if (out_tree != NULL) {
          Write_Swc_Tree(Get_String_Arg("-o"), out_tree);
        } else {
          Write_Swc_Tree(Get_String_Arg("-o"), tree);
        }
      }
    }
  } else {   // get non color coded blueness
    if (Is_Arg_Matched("-signal")) {
      Stack *signal = Read_Sc_Stack(Get_String_Arg("-signal"), Get_Int_Arg("-bluechannel")-1);
      if (signal != NULL) {
        double margin = 0.0;
        if (Is_Arg_Matched("-sigmargin")) {
          margin = Get_Double_Arg("-sigmargin");
        }
        Swc_Tree_Intensity_Feature_E(tree, signal, NULL, margin);
        double fmin, fmax;
        Swc_Tree_Feature_Range(tree, &fmin, &fmax);
        Swc_Tree_Iterator_Start(tree, 2, FALSE);
        int color_map[] = { 1, 3, 5, 7, 6, 8, 2 };
        Swc_Tree_Node *tn = NULL;
        while ((tn = Swc_Tree_Next(tree)) != NULL) {
          int new_type = iround((tn->feature - fmin) / (fmax - fmin) * 6.0);
          if (new_type > 6) {
            new_type = 6;
          }
          Swc_Tree_Node_Data(tn)->type = color_map[new_type];
          Swc_Tree_Node_Data(tn)->d += 5.0;
        }

        Kill_Stack(signal);
      }
    }
  }

  if (Is_Arg_Matched("-info")) {
    char *info = Get_String_Arg("-info");
    if ((strcmp(info, "length") == 0) || (strcmp(info, "all") == 0)) {
      printf("Overall length: %g\n", Swc_Tree_Overall_Length(tree));
    }

    if ((strcmp(info, "leaf") == 0) || (strcmp(info, "all") == 0)) {
      printf("Number of tips: %d\n", Swc_Tree_Leaf_Number(tree));
    }

    if ((strcmp(info, "branch_number") == 0) || strcmp(info, "all") == 0) {
      printf("Number of branches: %d\n", Swc_Tree_Branch_Number(tree));
    }

    if ((strcmp(info, "surface") == 0) || (strcmp(info, "all") == 0)) {
      printf("Surface area: %g\n", Swc_Tree_Surface_Area(tree));
    }
  }

  if (Is_Arg_Matched("-analysis")) {
    Geo3d_Scalar_Field *puncta =
      Geo3d_Scalar_Field_Import_Apo_E(Get_String_Arg("-apo", 0), 4);
    char branch_file[500];
    char puncta_file[500];
    char folder[500];
    sprintf(folder, "%s", Get_String_Arg("input",0));
    folder[strlen(folder)-4] = '\0';

    sprintf(branch_file, "%s/%s_branch.txt", folder, Get_String_Arg("input", 0));
    sprintf(puncta_file, "%s/%s_puncta.txt", folder, Get_String_Arg("-apo", 0));

    Swc_Tree_To_Analysis_File(tree, puncta, branch_file, puncta_file);
  }

  if (Is_Arg_Matched("-feat_file")) {
    char *feat_option = Get_String_Arg("-feat_file");
    if (strcmp(feat_option, "branch")) {
      Swc_Tree_To_Branch_Feature_File(tree, Get_String_Arg("-o"));
    }

  }


#ifdef _DEBUG_2
  Swc_Tree_To_Dot_File(tree, "../data/test2.dot");
#endif

  return 0;
}

/* same as fopen, but create folder when needed */
FILE *open_file(const char *path, const char *mode)
{
  FILE *fp = fopen(path, mode);
  if (fp == NULL) {
    int errsv = errno;
    /*perror("fopen");*/
    if (errsv == ENOENT) {
      /*printf("The directory does not exist. Creating new directory ...\n");*/
      int pos;
      char *pathcopy = malloc(strlen(path)+1);
      strcpy(pathcopy, path);
      for (pos=1; pos<strlen(pathcopy)-1; pos++) {
        if (pathcopy[pos]=='/') {
          pathcopy[pos] = '\0';
          struct stat sb;
          int e = stat(pathcopy, &sb);
          if (e != 0 && errno == ENOENT) {
            printf("Creating directory %s\n", pathcopy);
      #if defined(_WIN64) || defined(_WIN32)
      e = mkdir(pathcopy);
      #else
            e = mkdir(pathcopy, S_IRWXU);
      #endif
            if (e != 0) {
              perror("mkdir error");
              exit(EXIT_FAILURE);
            }
          }
          pathcopy[pos] = '/';
        }
      }
      free(pathcopy);
      /*try again*/
      fp = fopen(path, mode);
      if (fp == NULL) {
        perror("second try fopen");
        exit(EXIT_FAILURE);
      }
      return fp;
    }

    exit(EXIT_FAILURE);
  }
  return fp;
}

