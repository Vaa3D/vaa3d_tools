/**@file parsediadem.c
 * @brief >> Parse diadem results
 * @author Ting Zhao
 * @date 03-Aug-2009
 */

#include <utilities.h>
#include "tz_xml_utils.h"
#include "tz_geo3d_scalar_field.h"
#include "tz_string.h"
#include "tz_swc_tree.h"
#include "tz_darray.h"
#include "tz_u8array.h"
#include "tz_geo3d_utils.h"

static Geo3d_Scalar_Field* read_node_file(char *file_path)
{
  FILE *fp = fopen(file_path, "r");
  int n = 0;
  double value[4];

  String_Workspace *sw = New_String_Workspace();

  while (Read_Line(fp, sw)) {
    n++;
  }

  rewind(fp);

  Geo3d_Scalar_Field *field = Make_Geo3d_Scalar_Field(n);
  field->size = 0;

  char *line = NULL;
  while ((line = Read_Line(fp, sw)) != NULL) {
    String_To_Double_Array(line, value, &n);
    if (n == 4) {
      field->points[field->size][0] = value[0];
      field->points[field->size][1] = value[1];
      field->points[field->size][2] = value[2];
      field->values[field->size++] = value[3];
    } else if (n == 3) {
      field->points[field->size][0] = value[0];
      field->points[field->size][1] = value[1];
      field->points[field->size][2] = value[2];
      field->values[field->size++] = 1;
    }
  }

  fclose(fp);

  return field;
}

Swc_Tree_Node *Find_Closest_Branch_Point(Swc_Tree *tree, double *pos, double *dist_xy, double *dist_z){

	double tn_pos[3];
	double dxy, dz;
	Swc_Tree_Iterator_Start(tree, 2, FALSE);

	Swc_Tree_Node *tn = NULL;
	Swc_Tree_Node *close_tn = NULL;

	double mindist = -1.0;
	while ((tn = Swc_Tree_Next(tree)) != NULL) {
		if (Swc_Tree_Node_Is_Regular(tn) &&  Swc_Tree_Node_Is_Branch_Point(tn)) {
			Swc_Tree_Node_Pos(tn, tn_pos);
			if (mindist < 0) {
				close_tn = tn;
				mindist = Geo3d_Dist_Sqr(pos[0], pos[1], pos[2], tn_pos[0], tn_pos[1], tn_pos[2]);
			} else {
				double d = Geo3d_Dist_Sqr(pos[0], pos[1], pos[2],  tn_pos[0], tn_pos[1], tn_pos[2]);
				if (d < mindist) {
					mindist = d;
					close_tn = tn;
					dxy = sqrt((pos[0]-tn_pos[0])*(pos[0]-tn_pos[0])+(pos[1]-tn_pos[1])*(pos[1]-tn_pos[1]));
					dz = abs(pos[2] - tn_pos[2]);
				}
			}
		}
	}

	*dist_xy = dxy;
	*dist_z = dz;
	return close_tn;
}

Swc_Tree_Node *Find_Closest_Leaf_Point(Swc_Tree *tree, double *pos, double *dist_xy, double *dist_z){

	double tn_pos[3];
	double dxy, dz;
	Swc_Tree_Iterator_Start(tree, 2, FALSE);

	Swc_Tree_Node *tn = NULL;
	Swc_Tree_Node *close_tn = NULL;

	double mindist = -1.0;
	while ((tn = Swc_Tree_Next(tree)) != NULL) {
		if (Swc_Tree_Node_Is_Regular(tn) &&  Swc_Tree_Node_Is_Leaf(tn)) {
			Swc_Tree_Node_Pos(tn, tn_pos);
			if (mindist < 0) {
				close_tn = tn;
				mindist = Geo3d_Dist_Sqr(pos[0], pos[1], pos[2], tn_pos[0], tn_pos[1], tn_pos[2]);
				dxy = sqrt((pos[0]-tn_pos[0])*(pos[0]-tn_pos[0])+(pos[1]-tn_pos[1])*(pos[1]-tn_pos[1]));
				dz = fabs(pos[2] - tn_pos[2]);
			}else {
				double d = Geo3d_Dist_Sqr(pos[0], pos[1], pos[2],  tn_pos[0], tn_pos[1], tn_pos[2]);
				if (d < mindist) {
					mindist = d;
					close_tn = tn;
					dxy = sqrt((pos[0]-tn_pos[0])*(pos[0]-tn_pos[0])+(pos[1]-tn_pos[1])*(pos[1]-tn_pos[1]));
					dz = fabs(pos[2] - tn_pos[2]);
				}
			}
		}
	}

	*dist_xy = dxy;
	*dist_z = dz;
	return close_tn;
}

BOOL Is_Existing_In_Field(Geo3d_Scalar_Field *field, double *pos){
	BOOL existing = FALSE;
	int i;

	for(i=0; i<field->size; i++){
		if(Geo3d_Dist_Sqr(pos[0], pos[1], pos[2], field->points[i][0], field->points[i][1], field->points[i][2])==0)
			existing = TRUE;
		break;
	}

	return existing;
}

int main(int argc, char *argv[])
{

  static char *Spec[] = {"<dataset:string> | -D <string>", NULL};
  Process_Arguments(argc, argv, Spec, 1);

  char file_path[100];
  if (Is_Arg_Matched("-D")) {
    sprintf(file_path, "%s/error.xml", Get_String_Arg("-D"));
  } else {
    sprintf(file_path, "../data/diadem_%s/error.xml",
	    Get_String_Arg("dataset"));
  }

  xmlDocPtr doc;
  xmlNodePtr cur;

  doc = xmlParseFile(file_path);
  if (doc == NULL) {
    fprintf(stderr, "XML parsing failed.\n");
    return 1;
  }

  cur = xmlDocGetRootElement(doc);
  if (cur == NULL) {
    fprintf(stderr, "empty document\n");
    xmlFreeDoc(doc);
    return 1;
  }

  if (xmlStrcmp(cur->name, (const xmlChar*) "diadem_metric")) {
    fprintf(stderr, "document of wrong type\n");
    xmlFreeDoc(doc);
    return 1;
  }

  cur = cur->xmlChildrenNode;
  char *miss_file = NULL;
  char *golden_file = NULL;
  char *extra_file = NULL;
  char *nearby_file = NULL;
  char *test_file = NULL;
  char *miss_swc_file = NULL;
  char *extra_swc_file = NULL;
  char *nearby_swc_file = NULL;
  char *miss_vlm_file = NULL;
  char *extra_vlm_file = NULL;
  char *nearby_vlm_file = NULL;
  char *miss_score_file = NULL;
  char *extra_score_file = NULL;

  double EuDistThre_xy = 0.0;
  double EuDistThre_z = 0.0;
  double score;

  while (cur != NULL) {
    if (Xml_Node_Is_Element(cur, "miss") == TRUE) {
      miss_file = Xml_Node_String_Value(doc, cur);
    } else if (Xml_Node_Is_Element(cur, "extra") == TRUE) {
      extra_file = Xml_Node_String_Value(doc, cur);
    } else if (Xml_Node_Is_Element(cur, "score") == TRUE) {
      score = Xml_Node_Double_Value(doc, cur);
    } else if (Xml_Node_Is_Element(cur, "golden") == TRUE) {
      golden_file = Xml_Node_String_Value(doc, cur);
    } else if (Xml_Node_Is_Element(cur, "nearby") == TRUE) {
      nearby_file = Xml_Node_String_Value(doc, cur);
    } else if (Xml_Node_Is_Element(cur, "test") == TRUE) {
      test_file = Xml_Node_String_Value(doc, cur);
    } else if (Xml_Node_Is_Element(cur, "miss_swc") == TRUE) {
      miss_swc_file = Xml_Node_String_Value(doc, cur);
    } else if (Xml_Node_Is_Element(cur, "extra_swc") == TRUE) {
      extra_swc_file = Xml_Node_String_Value(doc, cur);
    } else if (Xml_Node_Is_Element(cur, "nearby_swc") == TRUE) {
      nearby_swc_file = Xml_Node_String_Value(doc, cur);
    } else if (Xml_Node_Is_Element(cur, "miss_vlm") == TRUE) {
      miss_vlm_file = Xml_Node_String_Value(doc, cur);
    } else if (Xml_Node_Is_Element(cur, "extra_vlm") == TRUE) {
      extra_vlm_file = Xml_Node_String_Value(doc, cur);
    } else if (Xml_Node_Is_Element(cur, "nearby_vlm") == TRUE) {
      nearby_vlm_file = Xml_Node_String_Value(doc, cur);
    } else if (Xml_Node_Is_Element(cur, "miss_score") == TRUE) {
      miss_score_file = Xml_Node_String_Value(doc, cur);
    } else if (Xml_Node_Is_Element(cur, "extra_score") == TRUE) {
      extra_score_file = Xml_Node_String_Value(doc, cur);
    }else if(Xml_Node_Is_Element(cur, "xyth") == TRUE) {
      EuDistThre_xy = Xml_Node_Double_Value(doc, cur);
    }else if(Xml_Node_Is_Element(cur, "zth") == TRUE) {
      EuDistThre_z = Xml_Node_Double_Value(doc, cur);
    }

    cur = cur->next;
  }

  if ((EuDistThre_xy <= 0.0) || (EuDistThre_z <= 0.0)) {
    fprintf(stderr, "Invalid threshold value\n");
    xmlFreeDoc(doc);
    return 1;
  }

  Swc_Tree *tree = Read_Swc_Tree(golden_file);

  double bound[6];
  Swc_Tree_Bound_Box(tree, bound);
  double marker_ratio = dmax3(bound[3] - bound[0], bound[4] - bound[1],
			      bound[5] - bound[2]) / 512.0;

  Geo3d_Scalar_Field *miss_field = read_node_file(miss_file);
  printf("The missed nodes:\n");
  Print_Geo3d_Scalar_Field(miss_field);

  Geo3d_Scalar_Field *extra_field = read_node_file(extra_file);
  printf("The extra nodes:\n");
  Print_Geo3d_Scalar_Field(extra_field);

  Geo3d_Scalar_Field *nearby_field = read_node_file(nearby_file);
  //    Print_Geo3d_Scalar_Field(nearby_field);

  double wm = darray_sum(miss_field->values, miss_field->size);
  double we = darray_sum(extra_field->values, extra_field->size);
  double wg = round((score * we + wm) / (1 - score));

  //    printf("%g %g %g\n", wm, we, wg);

  Swc_Tree_Iterator_Start(tree, 2, FALSE);
  int max_id = 0;
  Swc_Tree_Node *tn = NULL;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    Swc_Tree_Node_Data(tn)->type = 3;
    if (max_id < Swc_Tree_Node_Data(tn)->id) {
      max_id = Swc_Tree_Node_Data(tn)->id;
    }
  }

  uint8 *nearby_mask = u8array_calloc(max_id + 1);

  /* process nearby nodes */
  FILE *fp = NULL;
  FILE *fp3 = NULL;

  if (nearby_swc_file != NULL) {
    fp = fopen(nearby_swc_file, "w");
  }

  if (nearby_vlm_file != NULL) {
    fp3 = fopen(nearby_vlm_file, "w");
  }

  //    printf("Nearby list\n");
  int i;
  for (i = 0; i < nearby_field->size; i++) {
    Swc_Tree_Node *tn = Swc_Tree_Closest_Node(tree, nearby_field->points[i]);

    nearby_mask[Swc_Tree_Node_Data(tn)->id] = 1;

    double w = (1.0+3.0/(1+exp(-(nearby_field->values[i]*2.0-2.0))))
      * marker_ratio;

    if (fp != NULL) {
      fprintf(fp, "%d %d %g %g %g %g %d\n", i + 1, 7,
	      Swc_Tree_Node_Data(tn)->x,
	      Swc_Tree_Node_Data(tn)->y, Swc_Tree_Node_Data(tn)->z,w, -1);
    }

    if (fp3 != NULL) {
      fprintf(fp3, "%g,%g,%g,%g,1,%g\n", Swc_Tree_Node_Data(tn)->x,
	      Swc_Tree_Node_Data(tn)->y, Swc_Tree_Node_Data(tn)->z,
	      w, nearby_field->values[i]);
    }
  }
  if (fp != NULL) {
    fclose(fp);
  }
  if (fp3 != NULL) {
    fclose(fp3);
  }

  /* process missing nodes */
  fp = NULL;
  FILE *fp2 = fopen(miss_score_file, "w");
  fp3 = NULL;

  if (miss_swc_file != NULL) {
    fp = fopen(miss_swc_file, "w");
  }

  if (miss_vlm_file != NULL) {
    fp3 = fopen(miss_vlm_file, "w");
  }

  Swc_Tree *test_tree = Read_Swc_Tree(test_file);

  //    printf("Missing list\n");
  int n_leaves = 0;
  int n_branchings = 0;
  int n_leaves_distance = 0;
  int n_leaves_path = 0;
  int n_branchings_distance = 0;
  int n_branchings_path = 0;

  double score_leaves = 0.0;
  double score_branchings = 0.0;
  double score_leaves_distance = 0.0;
  double score_leaves_path = 0.0;
  double score_branching_distance = 0.0;
  double score_branching_path = 0.0;

  BOOL error_type; // 'true' indicates distance-error, 'false' indicates path-error

  for (i = 0; i < miss_field->size; i++) {
    Swc_Tree_Node *tn = Swc_Tree_Closest_Node(tree, miss_field->points[i]);
    double cur_score = miss_field->values[i] / (wg + we);
    fprintf(fp2, "%4d | %5.2f %5.2f %5.2f | %g | %g\n", Swc_Tree_Node_Data(tn)->id,
	    Swc_Tree_Node_Data(tn)->x, Swc_Tree_Node_Data(tn)->y,
	    Swc_Tree_Node_Data(tn)->z, miss_field->values[i],
	    cur_score);

    Swc_Tree_Node_Data(tn)->type = 2;

    double w = (1.0+3.0/(1+exp(-(miss_field->values[i]*2.0-2.0))))
      * marker_ratio;

    if (fp != NULL) {
      int type = 0;
      if (nearby_mask[Swc_Tree_Node_Data(tn)->id] == 1) {
	type = 1;
      }
      fprintf(fp, "%d %d %g %g %g %g %d\n", i + 1, type,
	      Swc_Tree_Node_Data(tn)->x,
	      Swc_Tree_Node_Data(tn)->y, Swc_Tree_Node_Data(tn)->z, w, -1);
    }

    if (fp3 != NULL) {
      double xy_dist, z_dist;
      Swc_Tree_Node *test_tn;
      double pos[3];
      Swc_Tree_Node_Pos(tn, pos);
      if(Swc_Tree_Node_Is_Branch_Point(tn))
	test_tn = Find_Closest_Branch_Point(test_tree, pos, &xy_dist, &z_dist);
      else
	test_tn = Find_Closest_Leaf_Point(test_tree, pos, &xy_dist, &z_dist);

      if(xy_dist >= EuDistThre_xy || z_dist >= EuDistThre_z){
	fprintf(fp3, "%g,%g,%g,%g,1,%g,[xy-%g z-%g | distance_error], 255, 255, 0\n",Swc_Tree_Node_Data(tn)->x + 1.0,
		Swc_Tree_Node_Data(tn)->y + 1.0, Swc_Tree_Node_Data(tn)->z + 1.0,
		w, miss_field->values[i], xy_dist, z_dist);
	error_type = TRUE;
      }else{
	fprintf(fp3, "%g,%g,%g,%g,1,%g,[xy-%g z-%g | path_error], 255, 0, 255\n",Swc_Tree_Node_Data(tn)->x + 1.0,
		Swc_Tree_Node_Data(tn)->y + 1.0, Swc_Tree_Node_Data(tn)->z + 1.0,
		w, miss_field->values[i], xy_dist, z_dist);
	error_type = FALSE;
      }
    }

    if(miss_field->values[i]==1){
      n_leaves ++;
      score_leaves += cur_score;
      if(error_type){ // distance error
	n_leaves_distance ++;
	score_leaves_distance += cur_score;
      }else{
	n_leaves_path ++;
	score_leaves_path += cur_score;
      }
    }else{
      n_branchings ++;
      score_branchings += cur_score;
      if(error_type){ // distance error
	n_branchings_distance ++;
	score_branching_distance += cur_score;
      }else{
	n_branchings_path ++;
	score_branching_path += cur_score;
      }
    }
  }

  if(fp2 != NULL){
    fprintf(fp2, "\nleaf errors:  \t %d | loss score: %g(%2.1f%%)\n", n_leaves, score_leaves, 100*score_leaves/(1 - score));
    fprintf(fp2, "leaf errors (dist): %d | loss score: %g(%2.1f%%) \n", n_leaves_distance, score_leaves_distance, 100*score_leaves_distance/(1 - score));
    fprintf(fp2, "leaf errors (path): %d | loss score: %g(%2.1f%%) \n", n_leaves_path, score_leaves_path, 100*score_leaves_path/(1 - score));
    fprintf(fp2, "\nbranching errors: \t %d | score loss: %g(%2.1f%%) \n", n_branchings, score_branchings, 100*score_branchings/(1 - score));
    fprintf(fp2, "branching error (dist):  %d | score loss: %g(%2.1f%%) \n", n_branchings_distance, score_branching_distance, 100*score_branching_distance/(1 - score));
    fprintf(fp2, "branching error (path):  %d | score loss: %g(%2.1f%%) \n", n_branchings_path, score_branching_path, 100*score_branching_path/(1 - score));
  }
  if (fp != NULL) {
    fclose(fp);
  }
  fclose(fp2);
  if (fp3 != NULL) {
    fclose(fp3);
  }

  /*
    sprintf(file_path, "../data/diadem_%s/error.swc", Get_String_Arg("dataset"));
    Write_Swc_Tree(file_path, tree);
  */

  tree = test_tree;

  /* process extra nodes*/
  fp = NULL;
  fp2 = fopen(extra_score_file, "w");
  fp3 = NULL;

  if (extra_swc_file != NULL) {
    fp = fopen(extra_swc_file, "w");
  }

  if (extra_vlm_file != NULL) {
    fp3 = fopen(extra_vlm_file, "w");
  }

  //    printf("Extra list\n");
  n_leaves = 0;
  n_branchings = 0;

  score_leaves = 0.0;
  score_branchings = 0.0;


  for (i = 0; i < extra_field->size; i++) {
    Swc_Tree_Node *tn = Swc_Tree_Closest_Node(tree, extra_field->points[i]);
    double cur_score = extra_field->values[i] * (wg - wm) / (wg + we) / (wg + we - extra_field->values[i]);

    fprintf(fp2, "%4d | %5.2f %5.2f %5.2f | %g | %g\n", Swc_Tree_Node_Data(tn)->id,
	    Swc_Tree_Node_Data(tn)->x, Swc_Tree_Node_Data(tn)->y,
	    Swc_Tree_Node_Data(tn)->z, extra_field->values[i], cur_score);

    double w = (1.0+3.0/(1+exp(-(extra_field->values[i]*2.0-2.0))))
      * marker_ratio;

    if (fp != NULL) {
      fprintf(fp, "%d %d %g %g %g %g %d\n", i + 1, 1,
	      Swc_Tree_Node_Data(tn)->x,
	      Swc_Tree_Node_Data(tn)->y, Swc_Tree_Node_Data(tn)->z, w, -1);
    }

    if (fp3 != NULL) {
      fprintf(fp3, "%g,%g,%g,%g,1,%g, error, 255, 255, 0\n", Swc_Tree_Node_Data(tn)->x + 1.0,
	      Swc_Tree_Node_Data(tn)->y + 1.0, Swc_Tree_Node_Data(tn)->z + 1.0,
	      w, extra_field->values[i]);
    }

    if(extra_field->values[i]==1){
      n_leaves ++;
      score_leaves += cur_score;
    }else{
      n_branchings ++;
      score_branchings += cur_score;
    }
  }

  if(fp2 != NULL){
    fprintf(fp2, "\nleaf errors:   %d | loss score: %g(%2.1f%%)\n", n_leaves, score_leaves, 100*score_leaves/(1 - score));
    fprintf(fp2, "branching errors:  %d | score loss: %g(%2.1f%%) \n", n_branchings, score_branchings, 100*score_branchings/(1 - score));
  }

  if (fp != NULL) {
    fclose(fp);
  }
  fclose(fp2);
  if (fp3 != NULL) {
    fclose(fp3);
  }

  xmlFreeDoc(doc);

  printf("Score = %g\n", score);

  return 0;
}
