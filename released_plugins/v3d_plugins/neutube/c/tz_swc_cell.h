/**@file tz_swc_cell.h
 * @brief swc routines
 * @author Ting Zhao
 * @date 06-Aug-2008
 */

#ifndef _TZ_SWC_CELL_H_
#define _TZ_SWC_CELL_H_

#include <stdio.h>
#include "tz_cdefs.h"

#include "tz_geo3d_ball.h"
#include "tz_arrayqueue.h"

__BEGIN_DECLS

enum {
  SWC_SOMA = 1, SWC_AXON, SWC_BASAL_DENDRITE, SWC_APICAL_DENDRITE,
  SWC_MAIN_TRUNK = 5,
  SWC_DENDRITE = 13, SWC_NEURITE = 12, SWC_UNKNOWN = 100
};

/**@struct _Swc_Node tz_swc_cell.h
 *
 * Swc node structure.
 */
typedef struct _Swc_Node {
  int id;   /**< id */
  int type; /**< neurite type */
  double d; /**< radius */
  double x; /**< x */
  double y; /**< y */
  double z; /**< z */
  int parent_id; /**< parent id */
  int label; /**< additional labeling field */
} Swc_Node;

typedef Swc_Node Swc_Cell; /* for old definition */

Swc_Node* New_Swc_Node();
void Kill_Swc_Node(Swc_Node *node);

void Default_Swc_Node(Swc_Node *node);

Swc_Node* Copy_Swc_Node(const Swc_Node *node);

void Swc_Node_Copy(Swc_Node *dst, const Swc_Node *src);

void Swc_Node_Fprint(FILE *fp, const Swc_Node *cell);
void Print_Swc_Node(const Swc_Node *cell);

int Swc_Node_Fscan(FILE *fp, Swc_Node *cell);

Geo3d_Ball* Swc_Node_To_Geo3d_Ball(const Swc_Node *sc, Geo3d_Ball *ball);

/**@brief SWC node color code.
 *
 * Swc_Node_Color_Code() returns the name of the V3D color of <sc>. This
 * function is especillay for V3D display and SVG correspondence.
 */
const char* Swc_Node_Color_Code(const Swc_Node *sc);

int Swc_File_Record_Number(const char *file_path);
int Swc_Fp_Max_Id(FILE *fp);
int Swc_File_Max_Id(const char *file_path);

/**@brief Read an SWC file.
 *
 * Read_Swc_File() returns an array of SWC nodes in the file <file_path>. The
 * number of nodes is store in <n>.
 */
Swc_Node* Read_Swc_File(const char *file_path, int *n);

/**@brief Sort swc nodes.
 * Swc_Sort() sorts <n> nodes in the array <sc> in the increasing order of
 * their IDs.
 */
void Swc_Sort(Swc_Node *sc, int n);

/**@brief normalize a swc tree
 *
 * Swc_Normalize() normalizes a swc tree so that the node id is within the range
 * of 1 to n, where n is the number of nodes in <sc>.
 */
void Swc_Normalize(Swc_Node *sc, int n);

/**@brief Concatenate two swc files.
 *
 * Swc_File_Cat() concatenates <path1> and <path2> and saves the result in
 * <out>. The IDs of the swc files will be automatically adjusted for producing
 * a valid swc file.
 */
void Swc_File_Cat(const char *path1, const char *path2, 
		  const char *out);

/**@brief Concatenate multiple swc files.
 *
 * Swc_File_Cat_M() concatenates <n> files specified by <path> and saves the
 * reuslt in <out>.
 */
void Swc_File_Cat_M(char** const path, int n, const char *out);

__END_DECLS

#endif
