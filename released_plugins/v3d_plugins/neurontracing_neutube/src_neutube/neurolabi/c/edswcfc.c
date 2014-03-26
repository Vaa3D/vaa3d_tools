/**@file edswcfc.c
 * @brief >> decompose swc file based on FC
 * @author Linqing Feng
 * @date 15-July-2011
 */

#include <string.h>
#include <utilities.h>
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

#include "private/tz_main.c"

FILE *open_file(const char *path, const char *mode);

static int skip_argument(char *argv[])
{
  if (strcmp(argv[0], "-o")) {
    return 2;
  }

  if (strcmp(argv[0], "-decompose")) {
    return 1;
  }

  if (strcmp(argv[0], "-mergesoma")) {
    return 1;
  }

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
      printf("-decompose: decompose the tree into individual FC branches and each branch is saved as a separate SWC file. When this argument is specified, "
             "if -o option is not folder (e.g. -o aaa/bb/c), it will be taken as the prefix of the output files and folders."
             "if -o option is folder (e.g. -o aaa/bb/), it will be taken as the output folder and input file name will be used as prefix.\n");
      printf("-mergesoma: merge soma while building pyramidal swc if the input file is not pyramidal swc");
      printf("\n");

      return 1;
    }
  }

  return 0;
}


int main(int argc, char *argv[])
{
  if (Show_Version(argc, argv, "0.01") == 1) {
    return 0;
  }

  static char *Spec[] = {"<input:string> ... [-o <string>]",
    "[-decompose] [-mergesoma]",
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

  /* convert apo file to swc file */
  if (fhasext(filepath[0], "apo")) {
    Geo3d_Scalar_Field *field = Geo3d_Scalar_Field_Import_Apo_E(filepath[0], 4);
    if (field != NULL) {
      int i;
      Swc_Node tmp_node;
      FILE *fp = fopen(Get_String_Arg("-o"), "w");
      for (i = 0; i < field->size; i++) {
        tmp_node.x = field->points[i][0];
        tmp_node.y = field->points[i][1];
        tmp_node.z = field->points[i][2];
        tmp_node.id = i + 1;
        tmp_node.parent_id = -1;
        tmp_node.d = Cube_Root(0.75 / TZ_PI * field->values[i]);
        Swc_Node_Fprint(fp, &tmp_node);
      }
      fclose(fp);
    } else {
      return 1;
    }
    return 0;
  }

  Swc_Tree *tree = Read_Swc_Tree(filepath[0]);

  /*test if it's already pyramidal, if not, convert to pyramidal*/
  Swc_Tree_Node *tn = tree->root;
  BOOL isPyramidal = TRUE;
  while (tn != NULL) {
    if (tn->node.type != 1 && tn->node.type != 3 && tn->node.type != 4 && tn->node.id > 0) {
      isPyramidal = FALSE;
      break;
    }
    tn = Swc_Tree_Node_Next(tn);
  }
  if (isPyramidal) {
    printf("Already pyramidal swc.\n");
  } else {
    printf("Not pyramidal swc yet, building pyramidal swc...\n");
    Swc_Tree_Grow_Soma(tree, 1);
    if (Is_Arg_Matched("-mergesoma")) {
      Swc_Tree_Resort_Pyramidal(tree, FALSE, TRUE);   /*merge soma to one node*/
    } else {
      Swc_Tree_Resort_Pyramidal(tree, FALSE, FALSE);
    }
    Swc_Tree_Set_Type_As_Label(tree);
    Swc_Tree_Resort_Id(tree);
  }

  /*assign color, hide in index...*/
  Swc_Tree_Iterator_Start(tree, 1, FALSE);
  int type = 0;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
      tn->index = type + 2;
      if (!(Swc_Tree_Node_Is_Continuation(tn) || 
            Swc_Tree_Node_Is_Root(tn))) {
        type++;
        type = type % 11;
      }
  }

  Swc_Tree_Merge_Root_Group(tree, SWC_SOMA);
  if (Swc_Tree_Node_Is_Virtual(tree->root)) {
    Swc_Tree_Node_Merge_To_Parent(tree->root->first_child);
  } else {
    Swc_Tree_Node_To_Virtual(tree->root);
  }

  /* Total branch number of the tree. */
  int branch_number = Swc_Tree_Label_Branch_All(tree) + 1;

  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);
  tn = Swc_Tree_Next(tree);
  //FILE *fp = fopen(branch_file, "w");
  //fprintf(fp, "# branch id, type, x, y, z, radius\n");
  int linenumber = 2;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (Swc_Tree_Node_Is_Branch_Point(tn->parent)) {
      linenumber++;
    }
    tn->node.label = linenumber++;
  }
  //fclose(fp);


  if (Is_Arg_Matched("-o")) {
    if (Is_Arg_Matched("-decompose")) {

      Swc_Tree_Iterator_Leaf(tree);
      tn = tree->begin;
      int basal_index = 0;
      int apical_index = 0;
      char outfilepath[500];
      char outbasalpath[500];
      char outapicalpath[500];
      char outbasalandapicalpath[500];
      char prefix[300];
      BOOL start = TRUE;
      int length = 0;
      FILE *fp = NULL;

      /*get prefix*/
      strcpy(prefix, Get_String_Arg("-o"));
      if (prefix[strlen(prefix)-1]=='/') {
        /*add input file name*/
        char inputfilepath[300];
        strcpy(inputfilepath, filepath[0]);
        char *end = strrchr(inputfilepath, '.');
        if (end != NULL) {
          *end = '\0';
        }
        else {
          printf("something stange happens..\n");
        }
        char *start = strrchr(inputfilepath, '/');
        if (start != NULL) {
          start++;
        }
        else {
          start = inputfilepath;
        }
        strcat(prefix, start);
      }


      sprintf(outbasalpath, "%s_basal.txt", prefix);
      sprintf(outapicalpath, "%s_apical.txt", prefix);
      sprintf(outbasalandapicalpath, "%s_basal_and_apical.txt",prefix);
      FILE *basalfp = open_file(outbasalpath, "w");
      FILE *apicalfp = open_file(outapicalpath, "w"); 
      FILE *basalandapicalfp = open_file(outbasalandapicalpath, "w");

      while (tn != NULL) {
        Swc_Tree_Node *tmptn = tn;
        if (fp != NULL) {
          fclose(fp);
        }
        

        if (tmptn->node.type == 4) {
          /*apical*/
          sprintf(outfilepath, "%s_apical/apical_%0*d.swc", prefix,  4, apical_index);
          fprintf(apicalfp, "apical_%0*d.swc", 4, apical_index);
          fprintf(basalandapicalfp, "apical_%0*d.swc", 4, apical_index);
          apical_index++;
        }
        else if (tmptn->node.type == 3) {
          /*basal*/
          sprintf(outfilepath, "%s_basal/basal_%0*d.swc", prefix,  4, basal_index);
          fprintf(basalfp, "basal_%0*d.swc", 4, basal_index);
          fprintf(basalandapicalfp, "basal_%0*d.swc", 4, basal_index);
          basal_index++;
        }
        
        fp = open_file(outfilepath, "w");
     
        int nodetypecopy = tmptn->node.type;
        int lastbranchid = -100;    // use to remove duplicated branch id
        while (! Swc_Tree_Node_Is_Root(tmptn) && tmptn->node.type != 1) {  /*without soma area*/
          if (nodetypecopy == 4 && lastbranchid != tmptn->node.label) {
            lastbranchid = tmptn->node.label;
            fprintf(apicalfp, ",%d", tmptn->node.label);
            fprintf(basalandapicalfp, ",%d", tmptn->node.label);
          }
          else if (nodetypecopy == 3 && lastbranchid != tmptn->node.label) {
            lastbranchid = tmptn->node.label;
            fprintf(basalfp, ",%d", tmptn->node.label);
            fprintf(basalandapicalfp, ",%d", tmptn->node.label);
          }

          tmptn->node.type = tmptn->index;
          Swc_Node_Fprint(fp, &(tmptn->node));
          tmptn = tmptn->parent;
        }
        if (nodetypecopy == 4) {
          fprintf(apicalfp, "\n");
          fprintf(basalandapicalfp, "\n");
        }
        else if (nodetypecopy == 3) {
          fprintf(basalfp, "\n");
          fprintf(basalandapicalfp, "\n");
        }
       
        tn = tn->next;
      }
      fclose(apicalfp);
      fclose(basalfp);

      /*soma*/
      sprintf(outfilepath, "%s_soma.swc", prefix);
      fp = open_file(outfilepath, "w");
      tn = tree->root;
      while (tn != NULL) {
        if (tn->node.type == 1) {
          Swc_Node_Fprint(fp, &(tn->node));
        }
        tn = Swc_Tree_Node_Next(tn);
      }
      fclose(fp);

      /*colored total swc*/
/*
      sprintf(outfilepath, "%s_total_colored.swc", prefix);
      fp = open_file(outfilepath, "w");                    
      tn = tree->root;                                     
      while (tn != NULL) {                                 
        if (Swc_Tree_Node_Is_Virtual(tn)) {                
          tn = Swc_Tree_Node_Next(tn);                     
        } else {                                           
          Swc_Node_Fprint(fp, &(tn->node));                
          tn = Swc_Tree_Node_Next(tn);                     
        }                                                  
      }                                                    
      fclose(fp);                                          
*/
      sprintf(outfilepath, "%s_total_colored.swc", prefix);
      Write_Swc_Tree(outfilepath, tree);


      printf("%d leaf number\n", Swc_Tree_Leaf_Number(tree));
      printf("%d basal dendrite\n", basal_index);
      printf("%d apical dendrite\n", apical_index);
    } 
  }

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

