/**@file tz_locseg_chain.h
 * @brief neuron chain
 * @author Ting Zhao
 * @date 04-Jun-2008
 */

#ifndef _TZ_LOCSEG_CHAIN_H_
#define _TZ_LOCSEG_CHAIN_H_

#include "tz_trace_defs.h"
#include "tz_locseg_chain_com.h"
#include "tz_locnp_chain.h"
#include "tz_locne_chain.h"
#include "tz_locseg_chain_knot.h"
#include "tz_stack_graph.h"
#include "tz_sp_grow.h"

__BEGIN_DECLS

/**@addtogroup locseg_chain Locseg chain
 *
 * @{
 */

#define NEUROCOMP_LOCSEG_CHAIN(nc) (((nc)->type == NEUROCOMP_TYPE_LOCSEG_CHAIN) ? ((Locseg_Chain*) ((nc)->data)) : NULL)

/**@brief Copy a chain.
 *
 * Copy_Locseg_Chain() return a copy of <chain>. The two chains do not share any
 * memory.
 */
Locseg_Chain* Copy_Locseg_Chain(Locseg_Chain *chain);

/**@brief Test if two chains are identical.
 *
 * Locseg_Chain_Is_Identical() returns TRUE if <chain1> and <chain2> have 
 * identical content; otherwise it returns FALSE.
 */
BOOL Locseg_Chain_Is_Identical(Locseg_Chain *chain1, Locseg_Chain *chain2);

/**@brief Appends a locnp chain to a locseg chain.
 * 
 * Locseg_Chain_Append_Locnp() appends a locnp chain to a locseg chain. <end>
 * specifies where to append: DL_TAIL for the tail of <chain1> and DL_HEAD
 * for the head of <chain1>.
 */
void Locseg_Chain_Append_Locnp(Locseg_Chain *chain1, Locnp_Chain *chain2,
			       Dlist_End_e end);

/**@brief Convert a locnp chain to a locseg chain.
 */
Locseg_Chain *Locseg_Chain_From_Locnp_Chain(Locnp_Chain *chain);

/**@brief Convert a locseg chain array to an ellipse array.
 */
Geo3d_Ellipse* 
Locseg_Chain_To_Ellipse_Array_Z(Locseg_Chain *chain, double z_scale,
				Geo3d_Ellipse *locne, int *n);

/**@brief Load chain files under a directory.
 *
 * Dir_Locseg_Chain_Nc() returns a an array of neuron components, which are
 * loaded from all files matching the regular expression <pattern> under the 
 * directory <dir_name>.
 * <n> stores the number of chains that are loaded. If <file_num> is not NULL,  
 * it stores the number suffix of each file. 
 */
Neuron_Component *Dir_Locseg_Chain_Nc(const char *dir_name, const char *pattern,
				      int *n, int *file_num);

/**@brief Build a neuron structure from locseg chain components.
 *
 * Locseg_Chain_Comp_Neurostruct() returns a neuron structure constructed from
 * the component array <comp>, which has <n> locseg chains as its elements. 
 * <stack> is the signal guide of chain connections if it is not NULL. <ws> is
 * the place to hold additional parameters.
 */
Neuron_Structure* 
Locseg_Chain_Comp_Neurostruct(Neuron_Component *comp, int n, 
			      const Stack *stack, double z_scale,
			      void *ws);

/**@brief Neuron reconstruction.
 *
 * Locseg_Chain_Comp_Neurostrut_W() reconstructs a neuron structure based on
 * the draft structure <ns>. 
 */
Neuron_Structure *
Locseg_Chain_Comp_Neurostruct_W(Neuron_Structure *ns, const Stack *stack,
				double z_scale, Connection_Test_Workspace *ws);


/**@brief Trace multiple seeds.
 *
 * Trace_Locseg_S() traces a total of <nseed> seeds and returns the result, 
 * which is an array of Locseg_Chain. The ith chain in the result array 
 * corresponds to the ith seed in the array <locseg>. <scores> can be NULL. But 
 * if it is not NULL, it should be the scores of the seeds and will be sorted 
 * during the funciton call. The scores are used to determine which seed is 
 * traced first. <tw> is the tracing workspace. The number of chains is recorded
 * in <nchain>.
 */
Locseg_Chain** Trace_Locseg_S(const Stack *signal, double z_scale, 
			      Local_Neuroseg *locseg, double *scores, 
			      int nseed, Trace_Workspace *tw, int *nchain);

/**@brief Number of knots of a chain.
 *
 * Locseg_Chain_Knot_Number() returns the number of knots in <chain>.
 */
int Locseg_Chain_Knot_Number(Locseg_Chain *chain);

/**@brief Get the knot array of a locseg chain.
 */
Locseg_Chain_Knot_Array* 
Locseg_Chain_To_Knot_Array(Locseg_Chain *chain, Locseg_Chain_Knot_Array *ka);

/**@brief Create an neuron component array from a locseg chain.
 *
 * Locseg_Chain_To_Neuron_Component() returns the neuron component array
 * constructed from <chain>. The type of the components is specified by <type>,
 * which should be NEUROCOMP_TYPE_LOCAL_NEUROSEG or NEUROCOMP_TYPE_GEO3D_CIRCLE.
 * The number of components is stored in <n>. Note that the memory associated
 * with component array is independent on that associated with <chain>. If <nc>
 * is not NULL, it is also the returned pointer and must have enough space to 
 * hold the result.
 */
Neuron_Component* 
Locseg_Chain_To_Neuron_Component(Locseg_Chain *chain, int type, 
				 Neuron_Component *nc, int *n);

/**@brief Create an neuron component array from a locseg chain with scaling.
 *
 * Locseg_Chain_To_Neuron_Component_S() is similar to 
 * Locseg_Chain_To_Neuron_Component(). But every segment in <chain> will be 
 * scaled with <xy_scale> and <z_scale> before producing a component in the 
 * array. The scaling will not change any segment property of <chain>.
 *
 * Note: In the current version Locseg_Chain_To_Neuron_Component() is not simply
 * Locseg_Chain_To_Neuron_Component_S() with scale 1. They have different
 * implementation schemes. Locseg_Chain_To_Neuron_Component_S() is more
 * recommended.
 */
Neuron_Component* 
Locseg_Chain_To_Neuron_Component_S(Locseg_Chain *chain, int type, 
				   Neuron_Component *nc, int *n,
				   double xy_scale, double z_scale);

/**@brief Calculate intensity profile of a chain.
 *
 * Locseg_Chain_Intensity_Profile() returns the intensity profile of <chain>.
 * Each element of the the returned array corresponds to the average intensity
 * of a segment of <chain> in the same order. <z_scale> is the z scale between
 * <chain> and <stack>.
 */
double* Locseg_Chain_Intensity_Profile(Locseg_Chain *chain, const Stack *stack,
				       double z_scale, double *value);

/**@brief The refit times in a chain.
 *
 * Locseg_Chain_Refit_Number() returns the number of refittings in <chain>
 */
int Locseg_Chain_Refit_Number(Locseg_Chain *chain);

/**@brief Break a chain at refitting segments.
 *
 * Locseg_Chain_Refit_Break() breaks <chain> at refitting segments and returns
 * the resulted chains in an array. The number of sub-chains is stored in <*n>.
 * If <chain_array> is not NULL, it is the same as the returned array.
 */
Locseg_Chain* Locseg_Chain_Refit_Break(Locseg_Chain *chain, 
				       Locseg_Chain *chain_array, int *n);

/**@brief Scale a locseg chain.
 *
 * Locseg_Chain_Scale_Z() applies inverse z-scaling on <chain>.
 */
void Locseg_Chain_Scale_Z(Locseg_Chain *chain, double z_scale);

/* xy-scaling */
void Locseg_Chain_Scale_XY(Locseg_Chain *chain, double xy_scale);

/**@brief Connection test based on planar distance.
 *
 * Locseg_Chain_Connection_Test_P() tests if <chain1> connects to <chain2> based
 * on planar distance. The result is stored in <conn> and the returned value is
 * the minimal planar distance.
 */
double Locseg_Chain_Connection_Test_P(Locseg_Chain *chain1,
				      Locseg_Chain *chain2,
				      Neurocomp_Conn *conn);

/**@brief Print a locseg chain to a swc file.
 *
 * Locseg_Chain_Swc_Fprint_Z() prints <chain> with inverse z-scaling 
 * (<z_scale>) in <fp> with the SWC format. It returns the number of SWC nodes
 * that are printed.
 */
int Locseg_Chain_Swc_Fprint_Z(FILE *fp, Locseg_Chain *chain, int type,
			      int start_id, int parent_id, 
			      Dlist_Direction_e direction, double z_scale);

/**@brief Print a locseg chain to a swc file with a certain offset.
 *
 * Locseg_Chain_Swc_Fprint_Z() prints <chain> with inverse z-scaling 
 * (<z_scale>) in <fp> with the SWC format. The position of the chain is
 * trasnlated with <offset>. It returns the number of SWC nodes
 * that are printed.
 */
int Locseg_Chain_Swc_Fprint_T(FILE *fp, Locseg_Chain *chain, int type,
			      int start_id, int parent_id, 
			      Dlist_Direction_e direction, double z_scale,
			      const double *offset);

/**@brief Write an SWC file from a locseg chain.
 *
 * Locseg_Chain_To_Swc_File() save the SWC representation of <chain> into an 
 * SWC file with the path <filepath> 
 */
void Locseg_Chain_To_Swc_File(Locseg_Chain *chain, const char *filepath);

/**@brief Create a circle array from a chain.
 *
 * Locseg_Chain_To_Geo3d_Circle_Array_Z() returns an array of circles that are
 * approximate cross sections of <chain> with inverse z-scaling with the 
 * parameter <z_scale>. The resulted number of circles is stored in <n>.
 * If <circle> is not NULL, it is the same as the returned pointer.
 */
Geo3d_Circle* Locseg_Chain_To_Geo3d_Circle_Array_Z(Locseg_Chain *chain,
						   double z_scale,
						   Geo3d_Circle *circle, 
						   int *n);
/**@brief Turn a locseg chain to a locne chain.
 */
Locne_Chain* Locseg_Chain_To_Locne_Chain(Locseg_Chain *chain);

/**@brief Push a local neuroseg.
 *
 * Local_Neuroseg_Push() returns a chain traced from <locseg>. The tracing 
 * follows the signal in <stack> but ignores any signal that is masked. The 
 * masked region is defined in <mask>, in which a non-zero value of a voxel 
 * means the voxel is masked. <mask> can be NULL, which means no voxel is 
 * masked. <option> specifies which information is recorded and should be one 
 * of the option values to fit stack. The tracing information is stored in 
 * <record>. The number of values is stored <trial>. Both <record> and <trial> 
 * can be NULL to skip information recording.
 */
Locseg_Chain* Local_Neuroseg_Push(Local_Neuroseg *locseg, const Stack *stack, 
				  double z_scale, const Stack *mask,
				  int option, double *record, 
				  int *trial);

/**@brief Connection features of two chains.
 *
 * Locseg_Chain_Conn_Feature() returns the features that describe how <chain1>
 * connects to <chain2>.
 */
double* Locseg_Chain_Conn_Feature(Locseg_Chain *chain1, Locseg_Chain *chain2,
				  const Stack *stack, const double *res, 
				  double *feat, int *n);

/**@brief Print a lcoseg chain to a VRML file.
 *
 * Locseg_Chain_Vrml_Fprint_Z() prints <chain> with inverse z-scaling 
 * (<z_scale>) in <fp> with the VRML format.
 */
void Locseg_Chain_Vrml_Fprint_Z(FILE *fp, Locseg_Chain *chain, double z_scale,
				const Vrml_Material *material, int indent);

/**@brief Calculate the average score of a locseg chain.
 *
 * Locseg_Chain_Average_Score() returns the average fitting score between
 * <chain> with z-scaling (<z_scale>) and <stack>. <option> specifies which
 * kind of score to calculate.
 */
double Locseg_Chain_Average_Score(Locseg_Chain *chain, const Stack *stack,
				  double z_scale, int option);

/**@brief Calculate the median score of a locseg chain.
 *
 * Locseg_Chain_Median_Score() returns the median fitting score between
 * <chain> with z-scaling (<z_scale>) and <stack>. <option> specifies which
 * kind of score to calculate.
 */
double Locseg_Chain_Median_Score(Locseg_Chain *chain, const Stack *stack,
				 double z_scale, int option);

/**@brief Calculate the Minimal score of a locseg chain.
 *
 * Locseg_Chain_Min_Seg_Score() returns the minimal fitting score between
 * <chain> with z-scaling (<z_scale>) and <stack>. <option> specifies which
 * kind of score to calculate.
 */
double Locseg_Chain_Min_Seg_Score(Locseg_Chain *chain, const Stack *stack,
				  double z_scale, int option);

/**@brief Average signal intensity of a locseg chain.
 *
 * Locseg_Chain_Average_Score() returns the average intensity of signal within
 * <chain> with z-scaling (<z_scale>).
 */
double Locseg_Chain_Average_Signal(Locseg_Chain *chain, const Stack *signal,
				   double z_scale);

/**@brief Minimal signal intensity of a locseg chain.
 *
 * Locseg_Chain_Average_Score() returns the minimal average segment intensity 
 * of signal within <chain> with z-scaling (<z_scale>).
 */
double Locseg_Chain_Min_Seg_Signal(Locseg_Chain *chain, const Stack *signal,
				   double z_scale);

/**@brief Average bottom radius of a locseg chain.
 *
 * Locseg_Chain_Average_Bottom_Radius() returns the average bottom radius of 
 * <chain>.
 */
double Locseg_Chain_Average_Bottom_Radius(Locseg_Chain *chain);

/**@brief Test if a locseg chain contains a certain stack value.
 *
 * Locseg_Chain_Has_Stack_Value() returns TRUE if inside <chain>, a voxel 
 * from <stack> has the intensity <value>; Otherwise it returns FALSE.
 */
BOOL Locseg_Chain_Has_Stack_Value(Locseg_Chain *chain, Stack *stack,
				  double z_scale, double value);

/**@brief Label a stack with a locseg chain.
 *
 * Locseg_Chain_Label_W() labels <stack> by <chain> with z-scaling (z_scale).
 * <start> and <end> specify which part of <chain> is for labeling. See the
 * definition of <Locseg_Label_Workspace> for more options.
 */
void Locseg_Chain_Label_W(Locseg_Chain *chain, Stack *stack, double z_scale,
			  int begin, int end, Locseg_Label_Workspace *ws);

/**@brief Estimate the trace record size in a file. */
int Locseg_Chain_File_Trace_Record_Size(const char *file);

/**@}*/

/**@brief Downsample a locseg chain.
 *
 * Locseg_Chain_Down_Sample() removes some segments in <chain> to make <chain>
 * sparser.
 */
void Locseg_Chain_Down_Sample(Locseg_Chain *chain);

/**@brief Tune an end of a chain.
 *
 * Locseg_Chain_Tune_End() tunes one of the ends of <chain> to make it have a 
 * better fit to <stack>.
 */
void Locseg_Chain_Tune_End(Locseg_Chain *chain, Stack *stack, double z_scale,
			   Stack *mask, Dlist_End_e end);

/**@brief brief Refine the ends of a chain.
 *
 * Locseg_Chain_Refine_End() refines both ends of <chain>.
 */ 
void Locseg_Chain_Refine_End(Locseg_Chain *chain, Stack *signal, double z_scale,
    Trace_Workspace *tw);

/**@brief Connect a local neuroseg to a chain.
 *
 * Locseg_Chain_Connect_Seg() connects <locseg> to a <chain> with an effort to
 * make the new chain smooth. The pointer <locseg> will become a part of 
 * <chain> after the function all. It returns the index of <locseg> in the new
 * chain.
 */
int Locseg_Chain_Connect_Seg(Locseg_Chain *chain, Local_Neuroseg *locseg,
			     Trace_Record *tr);

/**@brief Merge two chains.
 *
 * Locseg_Chain_Merge() merges <chain1> and <chain2>. <end1> and <end2> specify
 * where they are merged. <chain1> becomes the merged chain and <chain2> 
 * becomes empty after the function call.
 */
void Locseg_Chain_Merge(Locseg_Chain *chain1, Locseg_Chain *chain2, 
			Dlist_End_e end1, Dlist_End_e end2);

/**@brief Flip a chain.
 *
 * Locseg_Chain_Flip() flips <chain>. After flipping, the head of <chain> 
 * becomes its tail and the orietation of each local neuroseg is flipped.
 */
void Locseg_Chain_Flip(Locseg_Chain *chain);

/**@brief Fix a connection.
 *
 * Locseg_Chain_Conn_Fix() remodel <chain2> to make the connection between
 * <chain1> and <chain2> better.
 */
void Locseg_Chain_Conn_Fix(Locseg_Chain *chain1, Locseg_Chain *chain2,
			   Neurocomp_Conn *conn);
/*
void Locseg_Chain_Remove_Redundant(Locseg_Chain *chain);
*/

/**@brief Build a bridge between two lcoal neurosegs.
 *
 * Locseg_Chain_Bridge() returns a chain that connects <locseg1> and <locseg2>.
 */
Locseg_Chain* Locseg_Chain_Bridge(const Local_Neuroseg *locseg1, 
				  const Local_Neuroseg *locseg2);

/**@brief Build a bridge between two lcoal neurosegs.
 *
 * Local_Chain_Bridge_Sp() returns a chain that connects <source> and <target>.
 * If <fw> is not NULL, the thickness of the bridge chain is determined by 
 * fitting intensity field of <signal>. The fitting uses information in <fw>.
 * If <fw> is NULL, the thickness of the bridge is the same as that of <source>.
 */
Locseg_Chain* Locseg_Chain_Bridge_Sp(const Local_Neuroseg *source, 
				     Locseg_Chain *target, 
				     const Stack *signal, double z_scale,
				     Stack_Graph_Workspace *sgw,
				     Locseg_Fit_Workspace *fw);

/**brief Turn a skeleton into a locseg chain.
 *
 * Locseg_Chain_From_Skel() returns an locseg chain based on the supplied
 * skeleton <skel>, which has <n> elements. The value of each element of <skel>
 * represents the index of a point in <signal>. <sr> and <ss> give the size hint
 * of the start by specifying the radius and scale respectively. 
 * Fitting to <signal> is performed on the final chain based upon settings in 
 * <fw> if neither <sr> nor <ss> is 0.
 */
Locseg_Chain* Locseg_Chain_From_Skel(int *skel, int n, double sr, double ss,
				     const Stack *signal, double z_scale,
				     Locseg_Fit_Workspace *fw);

/**brief Turn a ball array into a locseg chain.
 *
 * Locseg_Chain_From_Skel() returns an locseg chain based on the supplied
 * ball array <skel>, which has <n> elements.  
 * Fitting to <signal> is performed on the final chain based upon settings in 
 * <fw>.
 */
Locseg_Chain* Locseg_Chain_From_Ball_Array(Geo3d_Ball *skel, int n,
					   const Stack *signal, double z_scale,
					   Locseg_Fit_Workspace *fw);

/**@brief Geometric length of a chain.
 *
 * Locseg_Chain_Geolen_Z() returns the inversely z-scaled geometric length of 
 * <chain>.
 */
double Locseg_Chain_Geolen_Z(Locseg_Chain *chain, double z_scale);

/**@brief The coordinates of one of the two ends of a chain.
 *
 * Locseg_Chain_End_Pos() finds the <end> of <chain> and stores the coordinates
 * in <pos>.
 */
void Locseg_Chain_End_Pos(Locseg_Chain *chain, Dlist_End_e end, double *pos);

/**@brief The distance from a point to a chain.
 *
 * Locseg_Chain_Point_Dist() returns the distance between the point <pos> and 
 * the chain <chain>. The index of the segment that is closest to <pos> is saved
 * in <seg_index> if it is not NULL.
 */
double Locseg_Chain_Point_Dist(Locseg_Chain *chain, const double *pos, 
			       int *seg_index, double *skel_pos);

/**@brief The distance from a point to the knots of a chain.
 *
 * Locseg_Chain_Point_Dist_K() returns the distance between <pos> and the 
 * neareast knot of <chain>. The center of the nearest knot is stored in
 * <knot_pos> if <knot_pos> is not NULL.
 */
double Locseg_Chain_Point_Dist_K(Locseg_Chain *chain, const double *pos, 
				 double *knot_pos);

/**@brief The distance between two chains.
 *
 * Locseg_Chain_Dist() returns the distance between the <end> of <chain1> and 
 * <chain2>.
 */
double Locseg_Chain_Dist(Locseg_Chain *chain1, Locseg_Chain *chain2, 
			 Dlist_End_e end);

/**@brief The bright end of a chain.
 *
 * Locseg_Chain_Bright_End() finds a bright point at the end of <chain>. <end>
 * must be DL_HEAD or DL_TAIL. <z_scale> is the scale for inverse z-scaling of 
 * <chain>. It checks pixels from the end to the half way of the last segment.
 */
void Locseg_Chain_Bright_End(Locseg_Chain *chain, Dlist_End_e end,
			     const Stack *signal, double z_scale, 
			     double *pos);	 

/**@brief Update the workspace for stack graph.
 *
 * Locseg_Chain_Update_Stack_Graph_Workspace() sets up <sgw> based on the 
 * specified chain and stack information. It assumes the graph is for hooking
 * <source> to <target>.
 */
void Locseg_Chain_Update_Stack_Graph_Workspace(const Local_Neuroseg *source,
					       Locseg_Chain *target,
					       const Stack *signal,
					       double z_scale,
					       Stack_Graph_Workspace *sgw);

/**@brief Shortest path between a point and a chain.
 *
 * Locseg_Chain_Shortest_Path_Pt() returns the index array of the shortest path 
 * between <pt> and the subchain from <start_index> to <end_index> of <target>.
 */
Int_Arraylist *Locseg_Chain_Shortest_Path_Pt(double *pos,
					     Locseg_Chain *target,
					     int start_index,
					     int end_index,
					     const Stack *signal, 
					     double z_scale,
					     Stack_Graph_Workspace *sgw);

/**@brief Shortest path between a locseg and a chain.
 *
 * Locseg_Chain_Shortest_Path_Seg() retunrs the index array of the shortest path
 * between <source> and <target>.
 */
Int_Arraylist *Locseg_Chain_Shortest_Path_Seg(const Local_Neuroseg *source,
					      Locseg_Chain *target,
					      const Stack *signal, 
					      double z_scale,
					      Stack_Graph_Workspace *sgw);

/**@brief Shortest path between two chains.
 *
 * Locseg_Chain_Shortest_Path_Seg() retunrs the index array of the shortest path
 * between <source> and <target>.
 */
Int_Arraylist *
Locseg_Chain_Shortest_Path(Locseg_Chain *source, 
			   Locseg_Chain *target,
			   const Stack *signal, double z_scale, 
			   Stack_Graph_Workspace *sgw);

Int_Arraylist* Locseg_Chain_Skel_Shortest_Path_Pt(double *pos,
						  Locseg_Chain *target,
						  int start_index,
						  int end_index,
						  const Stack *signal, 
						  double z_scale,
						  Stack_Graph_Workspace *sgw);

Int_Arraylist*
Locseg_Chain_Skel_Shortest_Path_Seg(const Local_Neuroseg *source,
				    Locseg_Chain *target,
				    const Stack *signal, 
				    double z_scale,
				    Stack_Graph_Workspace *sgw);

Int_Arraylist *Locseg_Chain_Skel_Shortest_Path(Locseg_Chain *source, 
					       Locseg_Chain *target,
					       const Stack *signal, 
					       double z_scale,
					       Stack_Graph_Workspace *sgw);

/**@brief Interploate a chain based on given position and orientation.
 */
int Locseg_Chain_Interpolate_L(Locseg_Chain *chain, const double *pos, 
			       const double *ort, double *new_pos);

void Locseg_Chain_Knot_Normalize(Locseg_Chain *chain);

/**@brief Translate a chain.
 *
 * Locseg_Chain_Translate() translates <chain> by <offset>.
 */
void Locseg_Chain_Translate(Locseg_Chain *chain, const double *offset);

/**@brief Break a node in a chain.
 *
 * Locseg_Chain_Break_Node() breaks a segment at <index> in <chain>. The 
 * segment was cut at the regular parameter <t> and becomes two nodes. It 
 * returns TRUE if the cut succeeds. When the segment was too short (<=1.5) or
 * <t> is not an internal point, it does nothing and returns FALSE.
 */
BOOL Locseg_Chain_Break_Node(Locseg_Chain *chain, int index, double t);

/**@brief Remove tracing seeds from a chain.
 *
 * Locseg_Chain_Remove_Seed() removes the tracing seeds from <chain>. A tracing
 * seed is a segment with DL_BOTHDIR. It returns the number of removed nodes.
 */
int Locseg_Chain_Remove_Seed(Locseg_Chain *chain);

/**@brief Adjust tracing seeds.
 *
 * Locseg_Chain_Adjust_Seed() adjusts the bottom or top of the tracing seeds at
 * the ends of <chain> so that they do not penetrate into other segments.
 */
void Locseg_Chain_Adjust_Seed(Locseg_Chain *chain);

BOOL Locseg_Chain_Form_Loop(Locseg_Chain *chain, Local_Neuroseg *locseg,
			    Dlist_Direction_e direction);

Local_Neuroseg *Locseg_Chain_Peek_Seg_Nearby(Locseg_Chain *chain, 
					     const double *pos);

Locseg_Chain* Locseg_Chain_From_Sp_Grow(Stack *stack, double z_scale,
					Sp_Grow_Workspace *sgw);

Locseg_Chain *Locseg_Chain_Bridge_Sp_G(Locseg_Chain *source, 
				       Locseg_Chain *target,
				       Stack *stack, double z_scale,
				       Sp_Grow_Workspace *sgw);


/**@brief Remove 0 size segments.
 * 
 * Locseg_Chain_Regularize() removes all segments that have size 0 from <chain>.
 */
void Locseg_Chain_Regularize(Locseg_Chain *chain);

/**@brief Import of a list of chains.
 *
 * Locseg_Chain_Import_List() returns an array of locseg chain objects that are
 * specified in <file_path>. Each row of <file_path> corresponds to the file of
 * a chain. The number of chains is stored in <n>.
 */
Locseg_Chain** Locseg_Chain_Import_List(const char *file_path, int *n);

/**@brief Break a chain at a certain index.
 *
 * Locseg_Chain_Break_At() breaks <chain> at the segment at <index>. It returns
 * the tail part, which starts from the segment next to <index>. It returns NULL
 * if <chain> is not broken.
 */
Locseg_Chain* Locseg_Chain_Break_At(Locseg_Chain *chain, int index);

/**@brief Break a chain.
 *
 * Locseg_Chain_Break_Between() breaks <chain> into two chains and returns the 
 * second one. <chain> itself keeps the first part. The break is done in the 
 * following way:
 * - If <index1> and <index2> is the same, it breaks between <index1>-1 and 
 *   <index2> if <index1> is not 0. If <index1> is 0, it breaks between <index1>
 *   and <index2>+1.
 * - If |<index1> - <index2>| == 1, it breaks the link between <index1> and 
 *   <index2>.
 * - If |<index1> - <index2>| >= 2 or <= 3, it removes all segments between 
 *   <index1>  and <index2>.
 * - If |<index1> - <index2>| > 3, it finds the maximum turn between <index1> 
 *   and <index2> and removes the two segments forming the turn.
 */
Locseg_Chain* Locseg_Chain_Break_Between(Locseg_Chain *chain, 
					 int index1, int index2);

/**@brief Reconstruct a neuron via shortest path grow.
 *
 * Locseg_Chain_Sp_Grow_Reconstruct() connects <n> chains in <chain_array> via
 * shortest path grow in <stack>. <sgw> is the workspace for shortest path grow.
 */
Neuron_Structure*
Locseg_Chain_Sp_Grow_Reconstruct(Locseg_Chain **chain_array,
				 int n, Stack *stack, double z_scale,
				 Sp_Grow_Workspace *sgw);

/**@brief Estimate how much a chain hits another.
 *
 * Locseg_Chain_Hit_Ratio() returns a value that indicates how much <source>
 * hits <target>. The value is in [0, 1] and larger value means more hit.
 */
double Locseg_Chain_Hit_Ratio(Locseg_Chain *source, Locseg_Chain *target);

/**@brief Estimate how much a chain hits an SWC file.
 *
 * Locseg_Chain_Hit_Ratio() returns a value that indicates how much <source>
 * hits <target>. The value is in [0, 1] and larger value means more hit.
 */
double Locseg_Chain_Hit_Ratio_Swc(Locseg_Chain *source, Swc_Tree *target);

/**@brief Extend a locseg chain to a certain position.
 */
void Locseg_Chain_Extend(Locseg_Chain *chain, Dlist_End_e end, 
    const double *pos);

/**@brief Calculate the turning angle between two chains.
 */
double Locseg_Chain_Turning_Angle(Locseg_Chain *chain1, 
    Locseg_Chain *chain2, int end1, int end2, double *res);

/**@brief Screen locseg chains.
 */
void Locseg_Chain_Array_Screen_Z(Locseg_Chain **chain, int n, double thre);

/**@brief Merge locseg chains.
 */
void Locseg_Chain_Array_Force_Merge(Locseg_Chain **chain, int n, 
    const Connection_Test_Workspace *ws);

/**@brief Break jumps.
 */
Locseg_Chain** Locseg_Chain_Array_Break_Jump(Locseg_Chain **chain, int n, 
    double dist_thre, int *n2);

/**@brief Correct ends of locseg chains.
 */
void Locseg_Chain_Correct_Ends(Locseg_Chain *chain);

__END_DECLS

#endif
