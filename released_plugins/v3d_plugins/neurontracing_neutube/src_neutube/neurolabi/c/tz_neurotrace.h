/**@file tz_neurotrace.h
 * @brief *neuron tracing routines (obsolete)
 * @author Ting Zhao
 * @date 31-Oct-2007
 */

#ifndef _TZ_NEUROTRACE_H_
#define _TZ_NEUROTRACE_H_

#include "tz_cdefs.h"
#include "tz_image_lib.h"
#include "tz_object_3d.h"
#include "tz_neuropos.h"
#include "tz_neuroseg.h"
#include "tz_neurochain.h"
#include "tz_locne_chain.h"

__BEGIN_DECLS

/*
 * Set_Neuroseg_Max_Radius() sets the maximum possible value of the radii of
 * the neuron segment, i.e. both the bottom and top radius of a valie neuron 
 * segment should not be greater than <r>. It returns the old value of the
 * maximum radius.
 */
double Set_Neuroseg_Max_Radius(double r);

/*
 * For the following routines which require <z_scale> as an argument, <z_scale>
 * has the same meaning: it is the scale of Z axis. z * <z_scale> transforms z 
 * from the filter space to the image space, where z is supposed to be the Z 
 * coordinate of a point in the filter space.
 *
 * Note: It might be more intuitive to define <z_scale> as the scale 
 * factor from the image space to the filter space. But since the routines has
 * already been written in the other way, I'll just keep it.
 */

/*
 * Initialize_Tracing() initializes neurochain tracing by setting the current
 * node and finding its best fit with <stack>. <locseg> could be
 * NULL, which means the function will use the current values as the start; 
 * otherwise <chain> is initialized by <locseg> first.
 * 
 * The function returns the score of fitting.
 */
double Initialize_Tracing(const Stack *stack, Neurochain *chain,
			  const Local_Neuroseg *locseg,
			  double z_scale);

/*
 * Trace_Neuron2() traces a neurite in <stack> and stores the result in <chain>,
 * whose current node is the start of tracing. d is the tracing direction,
 * which can be FORWARD, BACKWARD or BOTH. <mask> is a binary stack indicating
 * which voxels have already been labeled (0 means labeled and 1 means not).
 * <nseg> is the maximum number of segments that <chain> can have, so the
 * tracing will stop when <chain> has <nseg> segments. 
 */
Neurochain *Trace_Neuron2(const Stack *stack, Neurochain *chain, Direction_e d,
			  const Stack *mask, double z_scale, int nseg);

Neurochain *Object_To_Neurochain(Object_3d *obj, Neurochain *chain, 
				 double z_scale, int nseg);

void Trace_Neuron_All(const Stack *stack, Neurochain *chain[],
		      const Stack *mask, double z_scale);

/*
 * Neuroseg_Hit_Traced() returns TRUE when <locseg> hits a voxel that has 
 * value 0 in <mask>. A segment hits a voxel if and only if the voxel is the
 * nearest neighbor of its top center.
 *
 * Neurseg_Hit_At() returns the offset of the voxel that hit by <locseg> in
 * <mask>.
 *
 * <z_scale> is the Z-axis scale of the stack.
 */
BOOL Neuroseg_Hit_Traced(const Local_Neuroseg *locseg, const Stack *mask, 
			 double z_scale);
int Neuroseg_Hit_At(const Local_Neuroseg *locseg, const Stack *mask,
		    double z_scale);

void Read_Neurochain_Dir(const char *dir);
void Process_Neurochain(Neurochain *chain[], int n, Stack *region);

typedef struct _Soma_Trace_Workspace {
  Trace_Workspace tw;
  double curv_thre; /* curvature threshold */
} Soma_Trace_Workspace;

Soma_Trace_Workspace* New_Soma_Trace_Workspace();
void Delete_Soma_Trace_Workspace(Soma_Trace_Workspace *stw);

Locne_Chain* Trace_Soma(const Stack *stack, double z_scale, 
			Local_Neuroseg_Ellipse *seed, 
			Soma_Trace_Workspace *stw);
				   
double Soma_Score(Locne_Chain *soma);
void Soma_Stack_Mask(Locne_Chain *soma, Stack *stack, double z_scale,
		     double v);

__END_DECLS

#endif
