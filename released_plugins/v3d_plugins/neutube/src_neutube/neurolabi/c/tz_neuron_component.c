/* tz_neuron_component.c
 *
 * 29-Aug-2008 Initial write: Ting Zhao
 */

#include <stdio.h>
#include <stdlib.h>
#include <utilities.h>
#include "tz_error.h"
#include "tz_neuron_component.h"
#include "tz_locseg_chain.h"
#include "tz_local_neuroseg.h"
#include "tz_geo3d_ball.h"

Neuron_Component *New_Neuron_Component()
{
  Neuron_Component *nc = (Neuron_Component *) 
    Guarded_Malloc(sizeof(Neuron_Component), "New_Neuron_Component");
  Reset_Neuron_Component(nc);

  return nc;
}

void Delete_Neuron_Component(Neuron_Component *nc)
{
  free(nc);
}

void Clean_Neuron_Component(Neuron_Component *nc)
{
  switch (nc->type) {
  case NEUROCOMP_TYPE_LOCSEG_CHAIN:
    Clean_Locseg_Chain(NEUROCOMP_LOCSEG_CHAIN(nc));
    break;
  case NEUROCOMP_TYPE_LOCAL_NEUROSEG:
    break;
  case NEUROCOMP_TYPE_GEO3D_CIRCLE:
    break;
  default:
    if (nc->data != NULL) {
      TZ_ERROR(ERROR_DATA_TYPE);
    }
  }
  free(nc->data);

  Reset_Neuron_Component(nc);
}

void Kill_Neuron_Component(Neuron_Component *nc)
{
  switch (nc->type) {
  case NEUROCOMP_TYPE_LOCSEG_CHAIN:
    Kill_Locseg_Chain(NEUROCOMP_LOCSEG_CHAIN(nc));
    break;
  case NEUROCOMP_TYPE_LOCAL_NEUROSEG:
    Delete_Local_Neuroseg(NEUROCOMP_LOCAL_NEUROSEG(nc));
    break;
  case NEUROCOMP_TYPE_GEO3D_CIRCLE:
    break;
  default:
    TZ_ERROR(ERROR_DATA_TYPE);
  }  
  free(nc);
}

void Reset_Neuron_Component(Neuron_Component *nc)
{
  nc->type = NEUROCOMP_TYPE_UNIDENTIFIED;
  nc->data = NULL;  
}

void Set_Neuron_Component(Neuron_Component *nc, int type, void *data)
{
  nc->type = type;
  nc->data = data;
}

void Print_Neuron_Component(const Neuron_Component *nc)
{
  switch (nc->type) {
  case NEUROCOMP_TYPE_LOCSEG_CHAIN:
    printf("Locseg chain:\n");
    Print_Locseg_Chain(NEUROCOMP_LOCSEG_CHAIN(nc));
    break;
  case NEUROCOMP_TYPE_LOCAL_NEUROSEG:
    printf("Local neuroseg:\n");
    Print_Local_Neuroseg(NEUROCOMP_LOCAL_NEUROSEG(nc));
    break;
  case NEUROCOMP_TYPE_GEO3D_CIRCLE:
    printf("3D circle:\n");
    Print_Geo3d_Circle(NEUROCOMP_GEO3D_CIRCLE(nc));
    break;
  default:
    TZ_ERROR(ERROR_DATA_TYPE);
  }
}

void Neuron_Component_Draw_Stack(const Neuron_Component *nc, Stack *stack,
			  const Stack_Draw_Workspace *ws)
{
  switch (nc->type) {
  case NEUROCOMP_TYPE_LOCSEG_CHAIN:
    Locseg_Chain_Draw_Stack(NEUROCOMP_LOCSEG_CHAIN(nc), stack, ws);
    break;
  case NEUROCOMP_TYPE_LOCAL_NEUROSEG:
    Local_Neuroseg_Draw_Stack(NEUROCOMP_LOCAL_NEUROSEG(nc), stack, ws);
    break;
  case NEUROCOMP_TYPE_GEO3D_BALL:
    Geo3d_Ball_Draw_Stack(NEUROCOMP_GEO3D_BALL(nc), stack, ws);
    break;
  default:
    TZ_ERROR(ERROR_DATA_TYPE);
  }
}

void Clean_Neuron_Component_Array(Neuron_Component *nc, int n)
{
  int i;
  for (i = 0; i < n; i++) {
    Clean_Neuron_Component(nc + i);
  }
}

Neuron_Component* Make_Neuron_Component_Array(int n)
{
  Neuron_Component* nc = (Neuron_Component *) 
    Guarded_Malloc(sizeof(Neuron_Component) * n, "New_Neuron_Component");
  int i;
  for (i = 0; i < n; i++) {
    Reset_Neuron_Component(nc + i);
  }

  return nc;
}
