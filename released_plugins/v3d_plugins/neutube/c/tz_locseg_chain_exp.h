#ifndef _TZ_LOCSEG_CHAIN_EXP_H_
#define _TZ_LOCSEG_CHAIN_EXP_H_

#include "tz_stack.h"
#include "tz_locseg_chain.h"

__BEGIN_DECLS

typedef struct _Pixel_Feature
{
  int loc[3];
  size_t indice;
  double value;
  double eigen_values[3];   /*max to min*/
  double eigen_vector1[3];
  double eigen_vector2[3];
  double eigen_vector3[3];
} Pixel_Feature;

Pixel_Feature *Get_Pixel_Feature(double pos[], const Stack *stack, const Trace_Workspace *tw);

void Local_Neuroseg_Pixel_Feature(Local_Neuroseg *ln, const Stack *stack, const Trace_Workspace *tw);

void Reset_Pixel_Feature(Pixel_Feature *pf);

void Kill_Trace_Workspace_Exp(Trace_Workspace *tw);

Trace_Workspace*
Locseg_Chain_Default_Trace_Workspace_Exp(Trace_Workspace *tw,
           const Stack *stack);


__END_DECLS

#endif
