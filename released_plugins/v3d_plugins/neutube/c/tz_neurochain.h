/**@file tz_neurochain.h
 * @brief *neuron chain (obsolete)
 * @author Ting Zhao
 * @date 23-Nov-2007
 */

#ifndef _TZ_NEUROCHAIN_H_
#define _TZ_NEUROCHAIN_H_

#include "tz_cdefs.h"
#include "tz_local_neuroseg.h"
#include "tz_bitmask.h"
#include "tz_int_arraylist.h"
#include "tz_graph.h"
#include "tz_locseg_chain.h"
#include "tz_vrml_io.h"

__BEGIN_DECLS

typedef enum {FORWARD, BACKWARD, BOTH, LONG, STRONG} Direction_e; 

typedef enum {
  TRACE_HIST_FIT_SCORE_MASK = 1,
  TRACE_HIST_FIT_STAT_MASK,
  TRACE_HIST_FIT_CORR_MASK,
  TRACE_HIST_HIT_REGION_MASK,
  TRACE_HIST_INDEX_MASK,
  TRACE_HIST_IS_REFIT_MASK,
  TRACE_HIST_DIRECTION_MASK
} Trace_History_Mask_e;

/* Tracing history definition and routines */
typedef struct _Trace_History {
  Bitmask_t mask; /* mask to indicate which field has been assigned */
                  /* This should always be the first field in case the structure
		   * is changed in some future version */
  double fit_score;
  double fit_stat;
  double fit_corr;
  int hit_region;
  int index;
  BOOL is_refit;
  Direction_e direction;
} Trace_History;

Trace_History *New_Trace_History();
void Free_Trace_History(Trace_History *th);
void Trace_History_Copy(const Trace_History *src, Trace_History *des);
void Trace_History_Set_Score(Trace_History *th, const Neuroseg_Fit_Score *fs);
void Trace_History_Set_Hit_Region(Trace_History *th, int hit_region);
void Trace_History_Set_Index(Trace_History *th, int index);
void Trace_History_Set_Is_Refit(Trace_History *th, BOOL is_refit);
void Trace_History_Set_Direction(Trace_History *th, Direction_e direction);
BOOL Trace_History_Is_Refit(Trace_History *th);
void Fprint_Trace_History(FILE *fp, const Trace_History *th);
void Print_Trace_History(const Trace_History *th);
/******************************************/

typedef struct tagNeurochain {
  Local_Neuroseg locseg;        /* neuron segment */
  Trace_History *hist;           /* tracing information */
  struct tagNeurochain *prev;   /* previous node */
  struct tagNeurochain *next;   /* next node */
} Neurochain;

typedef enum {
  NEUROCHAIN_HEAD, NEUROCHAIN_TAIL, NEUROCHAIN_BOTH_END
} Neurochain_End_e;

/*
 * New_Neurochain() returns a new neuron chain with one node. Free_Neurochain()
 * frees all dynamic memories associated with <chain>. 
 */
Neurochain* New_Neurochain();
void Free_Neurochain(Neurochain *chain);

/*
 * Neurochain_Head() and Neurochain_Tail returns the head and tail of a neuron
 * chain respectively. They return NULL for NULL inputs.
 */
Neurochain *Neurochain_Head(const Neurochain *chain);
Neurochain *Neurochain_Tail(const Neurochain *chain);

/*
 * Neurochain_Next() returns the next node of <chain> at the direction <d>,
 * which should be FORWARD or BACKWARD.
 */
Neurochain *Neurochain_Next(const Neurochain *chain, Direction_e d);

Neurochain *Neurochain_Remove_Head(Neurochain *chain);
Neurochain *Neurochain_Remove_Tail(Neurochain *chain);

/*
 * Neurochain_Length() returns the length of a neuron chain. It could be 
 * counted forward (<d> is FORWARD) or backward (<d> is BACKWARD) from the 
 * current position, or returns the total length of the chain (<d> is BOTH).
 */
int Neurochain_Length(const Neurochain *chain, Direction_e d);

/*
 * Locate_Neurochain() locates a certain node relative to <chain> based on the
 * offset position <p>, which could be positive or negative, corresponding to
 * forward or backward.
 */
Neurochain *Locate_Neurochain(const Neurochain *chain, int p);

/*
 * Init_Neurochain() initializes a node of a neuron chain with neuron segment 
 * <seg> and position <pos>. Either or both of them could be NULL, which means
 * no effect.
 */
Neurochain* Init_Neurochain(Neurochain *chain, const Local_Neuroseg *seg);

/*
 * Append_Neurochain() appends <chain2> to the end of <chain1> and returns the
 * head of the new chain.
 */
Neurochain* Append_Neurochain(Neurochain *chain1, Neurochain *chain2);

/*
 * Neurochain_Remove_Last() removes the last n nodes from a neuron chain if
 * there are more than n nodes following the current node. Otherwise it removes
 * all nodes following the current node only. It returns the actual number of
 * removed nodes.
 *
 * Neurochain_Cut_Last() does a similar job but it returns a chain of the 
 * removed nodes without freeing them.
 *
 * Neurochain_Cut_At() cuts a chain at the next nth node to the current node.
 * It will not do anything if n is less than 1. It returns the cut-off chain.
 *
 * Neurochain_Cut() cuts at the current node and returns the tail of the
 * remained chain. The current node becomes the head of the cut-off chain.
 */
int Neurochain_Remove_Last(Neurochain *chain, int n);
Neurochain* Neurochain_Cut_Last(Neurochain *chain, int n);
Neurochain* Neurochain_Cut_At(Neurochain *chain, int n);
Neurochain* Neurochain_Cut(Neurochain *chain);

/*
 * Elongate_Neurochain() elongates a chain with one node from the head or/and 
 * tail. Its behavior also depends on the value of <d>: FORWARD for tail,
 * BACKWARD for head and BOTH for tail and head. It returns the new node of the
 * chain or the new head  if <d> is BOTH. Elongate_Neurochain_Spec() elonages 
 * a chain with the copy of <seg>. The new node is remained unintialized if
 * <seg> is NULL.
 *
 * Extend_Neurochain() add one node to the tail of the chain and returns the new
 * node. Extend_Neurochain_Spec() add a copy of <seg> to the tail of the 
 * chain and returns the new node. The new node is remained unintialized if
 * <seg> is NULL.
 */
Neurochain* Elongate_Neurochain(Neurochain *chain, Direction_e d);
Neurochain* Extend_Neurochain(Neurochain *chain);
Neurochain* Elongate_Neurochain_Spec(Neurochain *chain, Direction_e d,
				     const Local_Neuroseg *seg,
				     const Trace_History *hist);
Neurochain* Extend_Neurochain_Spec(Neurochain *chain, 
				   const Local_Neuroseg *seg,
				   const Trace_History *hist);

/*
 * Shrink_Neurochain() shrinks a chain with one node from the head or/and tail.
 * It returns the removed node. If both the head and tail node are removed and
 * they are different, the returned chain will have two node.
 */
Neurochain* Shrink_Neurochain(Neurochain **pchain, Neurochain_End_e ne);

/* 
 * Cross_Neurochain() cross to chains so that the previous node of <chain1>
 * is linked to <chain2> and the previous node of <chain2> is linked to 
 * <chain1>. 
 */
void Cross_Neurochain(Neurochain *chain1, Neurochain *chain2);

/*
 * Print_Neurochain() prints a neuron chain to the standard output.
 * Print_Neurochain_Info() prints the brief information of a neuron chain to
 * the standard output.
 */
void Fprint_Neurochain(FILE *fp, const Neurochain *chain);
void Print_Neurochain(const Neurochain *chain);
void Print_Neurochain_Info(const Neurochain *chain);

/*
 * Write_Neurochain() writes <chain> to the file named <file_path> by iterating
 * forward. The file will be created if it does not exist, otherwise it will
 * be overwritten. The file will be empty if chain is NULL.
 *
 * Read_Neurochain() returns the neuron chain that is read from the file named
 * <file_path>. It returns NULL if nothing is read, otherwise the returned
 * pointer is a new object and should be freed later.
 *
 * Dir_Neurochain() reads neuron chains in a directory and return them as an
 * array of neuron chains. The number of the chains is stored in <n>. All files
 * with the extension <ext> will be considered as neuron chain files.
 *
 * Dir_Neurochain_P() also reads neuron chains from a directory, but only those
 * matching the regular expression patttern are read.
 * 
 * Dir_Neurochain_D() is able to store full paths of the files that are read
 * in <file>.
 */
void Write_Neurochain(const char *file_path, const Neurochain *chain);
Neurochain* Read_Neurochain(const char *file_path);
Neurochain** Dir_Neurochain(const char *dir, const char *ext, int *n);
Neurochain** Dir_Neurochain_P(const char *dir, const char *pattern, int *n);
Neurochain** Dir_Neurochain_D(const char *dir, const char *pattern, int *n,
			      char *file[]);
Neurochain** Dir_Neurochain_N(const char *dir_name, const char *pattern, int *n,
			      int *file_num);

/*
 * Neurochain_Fwrite() writes <chain> to the file stream <fp> by iterating 
 * forward. Nothing will be written if chain is NULL.
 *
 * Neurochain_Fread() reads a neuron chain from <fp> to <chain>. The next node
 * of <chain> should be NULL while being input, otherwise there will be memory
 * leak.
 */
void Neurochain_Fwrite(const Neurochain *chain, FILE *fp);
void Neurochain_Fread(Neurochain *chain, FILE *fp);

/*
 * Neurochain_Fprint_Vrml() prints a neuron chain to <fp> as a VRML format.
 * Nothing will be printed if <chain> is NULL. The chain is iterated forward.
 */
void Neurochain_Fprint_Vrml(const Neurochain *chain, FILE *fp);

/*
 * Write_Neurochain_Vrml() creates a VRML file that contains the information
 * of <chain>. The written is done by "w" mode. The chain is iterated forward.
 */
void Write_Neurochain_Vrml( const char *file_path, const Neurochain *chain);

/*
 * Neurochain_Label() draws a neuron chain in a stack, which must be COLOR
 * kind. Neurochain_Erase() erases signal where the chain hits in a stack, which
 * must be GREY kind. 
 *
 * Neurochain_Lable_E() and Neurochain_Erase_E() do similar things. But they
 * allow users to change the range of labeling and thickness of the segments.
 * The segments from <begin> to <end> will be activated and their thickness
 * is changed to r * <ratio> + <diff>.
 */
void Neurochain_Label(Stack *stack, const Neurochain *chain, double z_scale);
void Neurochain_Erase(Stack *stack, const Neurochain *chain, double z_scale);

void Neurochain_Label_E(Stack *stack, const Neurochain *chain, double z_scale,
			int begin, int end, double ratio, double diff);
void Neurochain_Erase_E(Stack *stack, const Neurochain *chain, double z_scale,
			int begin, int end, double ratio, double diff);

void Neurochain_Label_G(Stack *stack, const Neurochain *chain, double z_scale,
			int begin, int end, double ratio, double diff, 
			int value);

/*
 * Neurochain_Translate() translates a chain in a 3D space. <x>, <y> and <z>
 * are offsets for X, Y and Z axis.
 */
void Neurochain_Translate(Neurochain *chain, double x, double y, double z);

/*
 * Neurochain_Score() calculates the score of a chain. It is the sum of the
 * scores of all neuron segments in the chain. The score is 0 for a NULL chain.
 * <fs> determines which score to calculate. Only fs->options[0] is effective.
 * If <fs> is NULL, the default one is calculated.
 */
double Neurochain_Score(const Neurochain *chain, const Stack *stack,
			double z_scale, Neuroseg_Fit_Score *fs);

double Neurochain_Geolen(const Neurochain *chain);

/*
 * Neurochain_Thickness_Mean() calculates the average thickness of a chain.
 * Neurochain_Thickness_Var() returns the variance of the thickness of a chain.
 * Neurochain_Thickness_Change_Mean() calculates the avearge change rate of a
 * chain.
 */
double Neurochain_Thickness_Mean(const Neurochain *chain);
double Neurochain_Thickness_Var(const Neurochain *chain);
double Neurochain_Thickness_Change_Mean(const Neurochain *chain);

/*
 * Neurochain_Angle_Mean() returns the mean of the angles of a chain.
 */
double Neurochain_Angle_Mean(const Neurochain *chain);

/*
 * Neurochain_Score_Mean() returns the mean tracing score of a chain.
 */
double Neurochain_Score_Mean(const Neurochain *chain, int option);

/*
 * Neurochain_Hit_Test() returns the index (1-indexing) of the neuron segment
 * if the point (<x>, <y>, <z>) hits the chain, otherwise it returns 0.
 * The index is counted from the current node at the direction <d>. 
 * The segments with indices no greater than <nseg> are tested only. <d> 
 * should be FORWARD or BACKWARD. To iterate all the nodes FORWARD or BACKWARD,
 * set <nseg> to 0.
 */
int Neurochain_Hit_Test(const Neurochain *chain,  Direction_e d, int nseg,
			double x, double y, double z);

int Neurochain_Hit_Test_Seg(const Neurochain *chain, Direction_e d, int nseg,
			    const Local_Neuroseg *locseg);

void Neurochain_Hit_Stack(const Neurochain *chain, Stack *region, int hit[]);

BOOL Neurochain_Hit_Bound(const Neurochain *chain, int width, int height,
			  int depth, int margin);

/*
 * Neurochain_Flip_Link() flips the link of the chain, i.e. the tail of the
 * chain becomes head but the neighbors of the nodes are kept the same.
 * It iterates from the current node in one direction only. The direction is
 * specified by <d>, which could be FORWARD or BACKWARD.
 * It returns the end of the new chain.
 *
 * FORWARD example:
 * ->node1->node2->node3->node4 ==> ->node4->node3->node2->node1
 *
 * BACKWARD example:
 * node1->node2->node3->node4-> ==> node4->node3->node2->node1->
 */
Neurochain* Neurochain_Flip_Link(Neurochain *chain, Direction_e d);

/*
 * Neurochain_Get_Locseg() retrieves a local segment from the current node. The
 * segment will be flipped if the tracing direction is BACKWARD.
 */
void Neurochain_Get_Locseg(const Neurochain *chain, Local_Neuroseg *locseg);

int Neurochain_End_Points(const Neurochain *chain, int n, 
			  coordinate_3d_t pts[]);

Neurochain* Neurochain_Remove_Overlap_Segs(Neurochain *chain);
Neurochain* Neurochain_Remove_Overlap_Ends(Neurochain *chain);
Neurochain* Neurochain_Remove_Turn_Ends(Neurochain *chain, double max_angle);

void Neurochain_Normalize_Direction(Neurochain *chain, Direction_e d);

Graph* Neurochain_Graph(Neurochain **chain, int n, Int_Arraylist *hit_spots);


Locseg_Chain* Neurochain_To_Locseg_Chain(Neurochain *chain);
Neurochain* Neurochain_From_Locseg_Chain(Locseg_Chain *chain);


double *Neurochain_Average_Signal(Neurochain *chain, const Stack *stack,
				  double z_scale, double *value);

void Vrml_Neurochain_Get_Locseg(const Neurochain *chain,
				Local_Neuroseg *locseg);
void Vrml_Neurochain_Fprint(FILE *fp, const Neurochain *chain, int indent);
void Vrml_Neurochain_Fprint_M(FILE *fp, const Neurochain *chain, 
			      const Vrml_Material *material, int indent);
void Vrml_Neurochain_Fprint_C(FILE *fp, const Neurochain *chain, 
			      const Vrml_Material *material, int indent);

__END_DECLS

#endif
