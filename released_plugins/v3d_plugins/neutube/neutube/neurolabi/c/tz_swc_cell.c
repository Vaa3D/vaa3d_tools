/* @file tz_swc_cell.c
 * @author Ting Zhao
 * @date 12-Sep-2008
 */

#include <stdlib.h>
#include <utilities.h>
#include "tz_error.h"
#include "tz_swc_cell.h"
#include "tz_iarray.h"
#include "tz_string.h"

const static char *Swc_Color_Code[] = {
  "white",
  "black",
  "red",
  "blue",
  "magenta",
  "cyan",
  "yellow",
  "green",
  "brown",
  "olivedrab",
  "indianred",
  "lightskyblue",
  "royalblue"
};

const char* Swc_Node_Color_Code(const Swc_Node *sc)
{
  return Swc_Color_Code[sc->type];
}

Swc_Node* New_Swc_Node()
{
  Swc_Node *node = (Swc_Node*) Guarded_Malloc(sizeof(Swc_Node), 
					      "New_Swc_Node");
  Default_Swc_Node(node);
  return node;
}

void Kill_Swc_Node(Swc_Node *node)
{
  free(node);
}

void Default_Swc_Node(Swc_Node *node)
{
  node->id = 0;
  node->type = 2;
  node->d = 1;
  node->x = 0;
  node->y = 0;
  node->z = 0;
  node->parent_id = -1;
  node->label = 0;
}

Swc_Node* Copy_Swc_Node(const Swc_Node *node)
{
  Swc_Node *new_node = New_Swc_Node();

  *new_node = *node;

  return new_node;
}

void Swc_Node_Copy(Swc_Node *dst, const Swc_Node *src)
{
  *dst = *src;
}

void Swc_Node_Fprint(FILE *fp, const Swc_Node *cell)
{
  ASSERT(fp, "NULL file pointer.");
  if (cell == NULL) {
    return;
  }

  fprintf(fp, "%d %d %g %g %g %g %d\n", 
	  cell->id, cell->type, cell->x, cell->y,
	  cell->z, cell->d, cell->parent_id);
}

void Print_Swc_Node(const Swc_Node *cell)
{
  if (cell->label > 0) {
    printf("swc node (%d): ", cell->label);
  } else {
    printf("swc node: ");
  }

  Swc_Node_Fprint(stdout, cell);
}

int Swc_Node_Fscan(FILE *fp, Swc_Node *cell)
{
  String_Workspace *sw = New_String_Workspace();

  char *line = Read_Line(fp, sw);
  int succ = 1;

  if (line != NULL) {
    strtrim(line);
    
    if (line[0] == '#') {
      succ = 0;
    } else {
      if (sscanf(line, "%d %d %lf %lf %lf %lf %d", &(cell->id), &(cell->type),
		 &(cell->x), &(cell->y), &(cell->z), &(cell->d),
		 &(cell->parent_id)) != 7) {
	succ = 0;
      }
    }
  } else {
    succ = 0;
  }

  Kill_String_Workspace(sw);

  return succ;
}

int Swc_File_Record_Number(const char *file_path)
{
  FILE *fp = Guarded_Fopen((char *) file_path, "r", "Swc_File_Record_Number");
  
  int n = 0;
  Swc_Node record;
  while (!feof(fp)) {
    if (Swc_Node_Fscan(fp, &record) == 1) {
      n++;
    }
  }

  fclose(fp);
  
  return n;
}

int Swc_Fp_Max_Id(FILE *fp)
{
  int n = -1;
  int max_pid = -1;

  Swc_Node record;
  while (!feof(fp)) {
    if (Swc_Node_Fscan(fp, &record) == 1) {
      if (n < record.id) {
	n = record.id;
      }
      if (max_pid < record.parent_id) {
	max_pid = record.parent_id;
      }
    }  
  }
  
  TZ_ASSERT(n >= max_pid, "Invalid parent id.");

  return n;
}

int Swc_File_Max_Id(const char *file_path)
{
  FILE *fp = Guarded_Fopen((char *) file_path, "r", "Swc_File_Record_Number");
  
  int n = Swc_Fp_Max_Id(fp);

  fclose(fp);
  
  return n;
}

Swc_Node* Read_Swc_File(const char *file_path, int *n)
{
  *n =  Swc_File_Record_Number(file_path);
  Swc_Node *neuron = (Swc_Node *) 
    Guarded_Malloc(sizeof(Swc_Node) * (*n), "Read_Swc_File");

  FILE *fp = Guarded_Fopen((char *) file_path, "r", "Read_Swc_File");
  
  int i;
  for (i = 0; i < *n; i++) {
    while (Swc_Node_Fscan(fp, neuron + i) != 1);
  }

  fclose(fp);

  return neuron;
}


Geo3d_Ball* Swc_Node_To_Geo3d_Ball(const Swc_Node *sc, Geo3d_Ball *ball)
{
  if (ball == NULL) {
    ball = New_Geo3d_Ball();
  }
  ball->r = sc->d / 2.0;
  ball->center[0] = sc->x;
  ball->center[1] = sc->y;
  ball->center[2] = sc->z;

  return ball;
}

static int swc_cell_compar(const void *sc1, const void *sc2)
{
  return ((Swc_Node*) sc1)->id - ((Swc_Node*) sc2)->id;
}

void Swc_Sort(Swc_Node *sc, int n)
{
  qsort(sc, n, sizeof(Swc_Node), swc_cell_compar);
}

void Swc_Normalize(Swc_Node *sc, int n)
{
  int max_id = 0;
  int i;
  for (i = 0; i < n; i++) {
    if (max_id < sc[i].id) {
      max_id = sc[i].id;
    }
  }

  int *id_map = iarray_malloc(max_id + 1);
  for (i = 0; i < n; i++) {
    id_map[sc[i].id] = i + 1;
  }

  for (i = 0; i < n; i++) {
    sc[i].id = id_map[sc[i].id];
    if (sc[i].parent_id >= 0) {
      sc[i].parent_id = id_map[sc[i].parent_id];
    }
  }

  free(id_map);
}

void Swc_File_Cat(const char *path1, const char *path2, 
		  const char *out)
{
  FILE *fp1 = GUARDED_FOPEN(path1, "r");
  FILE *fp2 = GUARDED_FOPEN(path2, "r");
  FILE *out_fp = GUARDED_FOPEN(out, "w");

  int max_id = -1;
  Swc_Node record;
  while (!feof(fp1)) {
    if (Swc_Node_Fscan(fp1, &record) == 1) {
      if (max_id < record.id) {
	max_id = record.id;
      }
      Swc_Node_Fprint(out_fp, &record);
    }  
  }
    
  int start_id = max_id + 1;

  while (!feof(fp2)) {
    if (Swc_Node_Fscan(fp2, &record) == 1) {
      record.id += start_id;
      if (record.parent_id >= 0) {
	record.parent_id += start_id;
      }
      Swc_Node_Fprint(out_fp, &record);
    }  
  }
  
  fclose(fp1);
  fclose(fp2);
  fclose(out_fp);
}

void Swc_File_Cat_M(char** const path, int n, const char *out)
{
  FILE *out_fp = GUARDED_FOPEN(out, "w");

  int max_id = -1;
  int start_id = 0;
  
  int i;
  for (i = 0; i < n; i++) {
    FILE *fp = GUARDED_FOPEN(path[i], "r");

    Swc_Node record;
    while (!feof(fp)) {
      if (Swc_Node_Fscan(fp, &record) == 1) {
	record.id += start_id;
	if (record.parent_id >= 0) {
	  record.parent_id += start_id;
	}
	if (max_id < record.id) {
	  max_id = record.id;
	}
	Swc_Node_Fprint(out_fp, &record);
      }  
    }

    start_id = max_id + 1;
    fclose(fp);
  }
  
  fclose(out_fp);
}
