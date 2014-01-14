/**@file tz_neuron_component.h
 * @brief building blocks for a neuron
 * @author Ting Zhao
 * @date 29-Aug-2008
 */

#ifndef _TZ_NEURON_COMPONENT_H_
#define _TZ_NEURON_COMPONENT_H_

#include "tz_cdefs.h"
#include "tz_stack_draw.h"

__BEGIN_DECLS

/**@addtogroup neuron_component_ Neuron components
 * @{
 */

/* For a data structure to be neuron component, it must have the following 
 * operations (repace Neurocomp with the data structure name):
 *   Neurocomp_Translate
 *   Neurocomp_Rotate
 *   Neurocomp_Fit_Stack
 *   Neurocomp_Draw_Stack
 *   Neurocomp_Connection_Test
 */

enum {
  NEUROCOMP_TYPE_UNIDENTIFIED = 0,
  NEUROCOMP_TYPE_LOCSEG_CHAIN = 1,
  NEUROCOMP_TYPE_LOCAL_NEUROSEG,
  NEUROCOMP_TYPE_GEO3D_BALL,
  NEUROCOMP_TYPE_GEO3D_CIRCLE,
  NEUROCOMP_TYPE_GEO3D_ELLIPSE
};

/*@brief Neuron component
 *
 * Each neuron component has a type, which is defined in the enum. For each type,
 * there might be a macro to convert a neuron component to that type directly.
 * For example, for the type NEUROCOMP_TYPE_LOCSEG_CHAIN, NEUROCOMP_LOCSEG_CHAIN
 * is the macro to get a pointer of Locseg_Chain. If the type is not matched,
 * the macro should return NULL.
 */
typedef struct _Neuron_Component {
  int type;          /* data type id */
  void *data;        /* data         */
} Neuron_Component;

Neuron_Component *New_Neuron_Component();
void Delete_Neuron_Component(Neuron_Component *nc);

void Clean_Neuron_Component(Neuron_Component *nc);
void Kill_Neuron_Component(Neuron_Component *nc);

void Reset_Neuron_Component(Neuron_Component *nc);
void Set_Neuron_Component(Neuron_Component *nc, int type, void *data);

void Print_Neuron_Component(const Neuron_Component *nc);

void Neuron_Component_Draw_Stack(const Neuron_Component *nc, Stack *stack,
				 const Stack_Draw_Workspace *ws);

/**@}*/

Neuron_Component* Make_Neuron_Component_Array(int n);
void Clean_Neuron_Component_Array(Neuron_Component *nc, int n);

__END_DECLS

#endif
