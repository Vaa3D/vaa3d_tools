#include <string.h>
#include <utilities.h>
#include <math.h>
#include "tz_math.h"
#include "tz_workspace.h"
#include "tz_stack_io.h"
#include "tz_neurofield.h"
#include "tz_local_neuroseg.h"
#include "tz_local_rpi_neuroseg.h"
#include "tz_r2_rect.h"
#include "tz_r2_ellipse.h"

Locseg_Label_Workspace* New_Locseg_Label_Workspace()
{
  Locseg_Label_Workspace *ws = (Locseg_Label_Workspace*) 
    Guarded_Malloc(sizeof(Locseg_Label_Workspace), 
		   "New_Locseg_Label_Workspace");

  Default_Locseg_Label_Workspace(ws);

  return ws;
}

void Default_Locseg_Label_Workspace(Locseg_Label_Workspace *ws)
{
  ws->signal = NULL;
  ws->option = 1;
  ws->flag = -1;
  ws->color[0] = 255;
  ws->color[1] = 0;
  ws->color[2] = 0;
  ws->value = 0;
  ws->sratio = 1.0;
  ws->sdiff = 0.0;
  ws->slimit = 3.0;
  ws->buffer_mask = NULL;
  int i;
  for (i = 0; i < 6; i++) {
    ws->range[i] = -1;
  }
}

void Delete_Locseg_Label_Workspace(Locseg_Label_Workspace *ws)
{
  free(ws);
}

void Clean_Locseg_Label_Workspace(Locseg_Label_Workspace *ws)
{
  if (ws->signal != NULL) {
    Kill_Stack(ws->signal);
    ws->signal = NULL;
  }

  if (ws->buffer_mask != NULL) {
    Kill_Stack(ws->buffer_mask);
    ws->buffer_mask = NULL;
  }
}

void Kill_Locseg_Label_Workspace(Locseg_Label_Workspace *ws)
{
  Clean_Locseg_Label_Workspace(ws);
  Delete_Locseg_Label_Workspace(ws);
}

Receptor_Score_Workspace* New_Receptor_Score_Workspace()
{
  Receptor_Score_Workspace *ws = (Receptor_Score_Workspace*) 
    Guarded_Malloc(sizeof(Receptor_Score_Workspace), 
		   "New_Locseg_Score_Workspace");
  Default_Receptor_Score_Workspace(ws);

  return ws;
}

void Print_Locseg_Label_Workspace(const Locseg_Label_Workspace *ws)
{
  printf("Signal stack: ");
  Print_Stack_Info(ws->signal);
  printf("Option: %d\n", ws->option);
  printf("Flag: %d; Color: %u %u %u; Value %d\n", ws->flag,
	 ws->color[0], ws->color[1], ws->color[2], ws->value);
  printf("Swell: r * %g + %g (limit: %g)\n", ws->sratio, ws->sdiff, ws->slimit);
  printf("Range: [%d, %d], [%d, %d], [%d, %d]\n", ws->range[0], ws->range[3],
	 ws->range[1], ws->range[4], ws->range[2], ws->range[5]);
}

void Default_Receptor_Score_Workspace(Receptor_Score_Workspace *ws)
{
  ws->fs.n = 1;
  ws->fs.options[0] = STACK_FIT_DOT;
  ws->field_func = NULL;
  ws->mask = NULL;
}

void Delete_Receptor_Score_Workspace(Receptor_Score_Workspace *ws)
{
  free(ws);
}

void Clean_Receptor_Score_Workspace(Receptor_Score_Workspace *ws)
{
  if (ws->mask != NULL) {
    Kill_Stack(ws->mask);
    ws->mask = NULL;
  }
}

void Kill_Receptor_Score_Workspace(Receptor_Score_Workspace *ws)
{
  Clean_Receptor_Score_Workspace(ws);
  Delete_Receptor_Score_Workspace(ws);
}

void Print_Receptor_Score_Workspace(const Receptor_Score_Workspace *ws)
{
  Print_Stack_Fit_Score(&(ws->fs));
  printf("Field function: ");
  if (ws->field_func == NULL) {
    printf("NULL\n");
  } else if (ws->field_func == Neurofield_S1) {
    printf("Neurofield_S1: negative * 2\n");
  } else if (ws->field_func == Neurofield_S2) {
    printf("Neurofield_S2: exp(-t) - exp(-1)\n");
  } else if (ws->field_func == Neurofield_S3) {
    printf("Neurofield_S3: -1/1\n");
  } else {
    printf("Unknown field function.");
  }
}

void Receptor_Score_Workspace_Copy(Receptor_Score_Workspace *dst, 
				   const Receptor_Score_Workspace *src)
{
  memcpy(dst, src, sizeof(Receptor_Score_Workspace));
}


void Clean_Receptor_Fit_Workspace(Receptor_Fit_Workspace *ws)
{
  if (ws->sws != NULL) {
    Kill_Receptor_Score_Workspace(ws->sws);
    ws->sws = NULL;
  }
}
/*
void Delete_Receptor_Fit_Workspace(Receptor_Fit_Workspace *ws)
{
  free(ws);
}

void Kill_Receptor_Fit_Workspace(Receptor_Fit_Workspace *ws)
{
  Clean_Receptor_Fit_Workspace(ws);
  Delete_Receptor_Fit_Workspace(ws);  
}
*/

void Default_Receptor_Fit_Workspace(Receptor_Fit_Workspace *ws)
{
  ws->nvar = 0;
  ws->pos_adjust = 0;
  ws->sws = NULL;
}

DEFINE_ZOBJECT_INTERFACE(Receptor_Fit_Workspace)

void Receptor_Fit_Workspace_Copy(Receptor_Fit_Workspace *dst, 
			       const Receptor_Fit_Workspace *src)
{
  memcpy(dst, src, sizeof(Receptor_Fit_Workspace));
}

Locseg_Fit_Workspace* New_Locseg_Fit_Workspace()
{
  Locseg_Fit_Workspace *ws = (Locseg_Fit_Workspace*) 
    Guarded_Malloc(sizeof(Locseg_Fit_Workspace), 
		   "New_Locseg_Fit_Workspace");
  ws->sws = NULL;
  Default_Locseg_Fit_Workspace(ws);

  return ws;  
}

void Default_Locseg_Fit_Workspace(Locseg_Fit_Workspace *ws)
{
  ws->nvar = 
    Local_Neuroseg_Var_Mask_To_Index(NEUROSEG_VAR_MASK_R |
				     NEUROSEG_VAR_MASK_ORIENTATION |
				     NEUROSEG_VAR_MASK_SCALE,
				     NEUROPOS_VAR_MASK_NONE,
				     ws->var_index);

  ws->var_link = NULL;
  DECLARE_LOCAL_NEUROSEG_VAR_MIN(var_min);
  DECLARE_LOCAL_NEUROSEG_VAR_MAX(var_max);
  memcpy(ws->var_min, var_min, sizeof(var_min));
  memcpy(ws->var_max, var_max, sizeof(var_max));

  if (ws->sws != NULL) {
    Default_Locseg_Score_Workspace(ws->sws);
  } else {
    ws->sws = New_Locseg_Score_Workspace();
  }
  ws->pos_adjust = 1;
}

void Default_Rpi_Locseg_Fit_Workspace(Receptor_Fit_Workspace *ws)
{
  ws->nvar = 
    Local_Rpi_Neuroseg_Var_Mask_To_Index(RPI_NEUROSEG_VAR_MASK_OFFSET |
	RPI_NEUROSEG_VAR_MASK_ORIENTATION, NEUROPOS_VAR_MASK_NONE,
       	ws->var_index);

  ws->var_link = NULL;
  DECLARE_LOCAL_RPI_NEUROSEG_VAR_MIN(var_min);
  DECLARE_LOCAL_RPI_NEUROSEG_VAR_MAX(var_max);
  memcpy(ws->var_min, var_min, sizeof(var_min));
  memcpy(ws->var_max, var_max, sizeof(var_max));

  if (ws->sws != NULL) {
    Default_Receptor_Score_Workspace(ws->sws);
  } else {
    ws->sws = New_Receptor_Score_Workspace();
  }
  ws->pos_adjust = 1;
}

void Default_R2_Rect_Fit_Workspace(Receptor_Fit_Workspace *ws)
{
  ws->nvar = 
    Local_R2_Rect_Var_Mask_To_Index(RECEPTOR_TRANSFORM_VAR_MASK_R | RECEPTOR_TRANSFORM_VAR_MASK_THETA, 
        ws->var_index);

  ws->var_link = NULL;
  DECLARE_LOCAL_R2_RECT_VAR_MIN(var_min);
  DECLARE_LOCAL_R2_RECT_VAR_MAX(var_max);
  memcpy(ws->var_min, var_min, sizeof(var_min));
  memcpy(ws->var_max, var_max, sizeof(var_max));

  if (ws->sws != NULL) {
    Default_Receptor_Score_Workspace(ws->sws);
  } else {
    ws->sws = New_Receptor_Score_Workspace();
  }
  ws->pos_adjust = 1;
}

void Default_R2_Ellipse_Fit_Workspace(Receptor_Fit_Workspace *ws)
{
  ws->nvar = 
    Local_R2_Ellipse_Var_Mask_To_Index(RECEPTOR_TRANSFORM_ELLIPSE_2D_VAR_MASK_OFFSET| RECEPTOR_TRANSFORM_ELLIPSE_2D_VAR_MASK_ALPHA, 
        ws->var_index);

  ws->var_link = NULL;
  DECLARE_LOCAL_R2_ELLIPSE_VAR_MIN(var_min);
  DECLARE_LOCAL_R2_ELLIPSE_VAR_MAX(var_max);
  memcpy(ws->var_min, var_min, sizeof(var_min));
  memcpy(ws->var_max, var_max, sizeof(var_max));

  if (ws->sws != NULL) {
    Default_Receptor_Score_Workspace(ws->sws);
  } else {
    ws->sws = New_Receptor_Score_Workspace();
  }
  ws->pos_adjust = 1;
}

void Locseg_Fit_Workspace_Enable_Cone(Locseg_Fit_Workspace *ws)
{
  Bitmask_t param_mask = Index_To_Bitmask(ws->var_index, ws->nvar);
  
  ws->nvar = 
    Local_Neuroseg_Var_Mask_To_Index(param_mask | NEUROSEG_VAR_MASK_C,
				     NEUROPOS_VAR_MASK_NONE,
				     ws->var_index);
}

void Locseg_Fit_Workspace_Disable_Cone(Locseg_Fit_Workspace *ws)
{
  Bitmask_t param_mask = Index_To_Bitmask(ws->var_index, ws->nvar);
  
  ws->nvar = 
    Local_Neuroseg_Var_Mask_To_Index(param_mask & (~NEUROSEG_VAR_MASK_C),
				     NEUROPOS_VAR_MASK_NONE,
				     ws->var_index);
}

DECLARE_LOCAL_NEUROSEG_VAR_NAME

void Print_Locseg_Fit_Workspace(Locseg_Fit_Workspace *ws)
{
  if (ws != NULL) {
    int i;
    for (i = 0; i < ws->nvar; i++) {
      printf("%d %s [%g %g]\n", ws->var_index[i], 
	     Local_Neuroseg_Var_Name[ws->var_index[i]],
	     ws->var_min[ws->var_index[i]], ws->var_max[ws->var_index[i]]);
    }
    
    if (ws->var_link != NULL) {
      printf("Linked variables: %d -- %d", ws->var_link[0], ws->var_link[1]);
    }
    
    Print_Locseg_Score_Workspace(ws->sws);
  } else {
    printf("Null locseg fit workspace.\n");
  }
}

void Locseg_Fit_Workspace_Set_Var(Locseg_Fit_Workspace *ws,
				  Bitmask_t neuroseg_mask, 
				  Bitmask_t neuropos_mask)
{
  ws->nvar = Local_Neuroseg_Var_Mask_To_Index(neuroseg_mask, neuropos_mask,
					      ws->var_index);
}

Locseg_Ellipse_Fit_Workspace* New_Locseg_Ellipse_Fit_Workspace()
{
  Locseg_Ellipse_Fit_Workspace *ws = (Locseg_Ellipse_Fit_Workspace*) 
    Guarded_Malloc(sizeof(Locseg_Ellipse_Fit_Workspace), 
		   "New_Locseg_Ellipse_Fit_Workspace");
  ws->sws = NULL;
  Default_Locseg_Ellipse_Fit_Workspace(ws);

  return ws;  
}

void Default_Locseg_Ellipse_Fit_Workspace(Locseg_Ellipse_Fit_Workspace *ws)
{
  ws->nvar = 
    Local_Neuroseg_Ellipse_Var_Mask_To_Index(NEUROSEG_ELLIPSE_VAR_MASK_R |
					     NEUROSEG_ELLIPSE_VAR_MASK_OFFSET |
					     NEUROSEG_ELLIPSE_VAR_MASK_ALPHA,
					     NEUROPOS_VAR_MASK_NONE,
					     ws->var_index);
  
  ws->var_link = NULL;
  DECLARE_LOCAL_NEUROSEG_ELLIPSE_VAR_MIN(var_min);
  DECLARE_LOCAL_NEUROSEG_ELLIPSE_VAR_MAX(var_max);
  memcpy(ws->var_min, var_min, sizeof(var_min));
  memcpy(ws->var_max, var_max, sizeof(var_max));

  if (ws->sws != NULL) {
    Default_Locseg_Ellipse_Score_Workspace(ws->sws);
  } else {
    ws->sws = New_Locseg_Ellipse_Score_Workspace();
  }
  ws->pos_adjust = 1;
}

void Default_Locseg_Chain_Skel_Workspace(Locseg_Chain_Skel_Workspace *ws)
{
  ws->fw = NULL;
  ws->sr = 0.0;
  ws->ss = 0.0;
  ws->er = 0.0;
  ws->es = 0.0;
}

void Clean_Locseg_Chain_Skel_Workspace(Locseg_Chain_Skel_Workspace *ws)
{
  Kill_Locseg_Fit_Workspace(ws->fw);
  Default_Locseg_Chain_Skel_Workspace(ws);
}

void Print_Locseg_Chain_Skel_Workspace(Locseg_Chain_Skel_Workspace *ws)
{
  printf("start: %g, %g; end: %g, %g", ws->sr, ws->ss, ws->er, ws->es);
}

DEFINE_ZOBJECT_INTERFACE(Locseg_Chain_Skel_Workspace)


void Default_Swc_Tree_Svg_Workspace(Swc_Tree_Svg_Workspace *ws)
{
  ws->puncta = NULL;
  ws->on_root = NULL;
  ws->puncta_type = NULL;
  ws->shuffling = FALSE;
  ws->showing_count = FALSE;
  ws->showing_length = FALSE;
  ws->color_code = SWC_SVG_COLOR_CODE_NONE;
  ws->max_vx = 800;
  ws->max_vy = 600;
  ws->weight_option = SWC_SVG_LENGTH_WEIGHT;
}

void Clean_Swc_Tree_Svg_Workspace(Swc_Tree_Svg_Workspace *ws)
{
  if (ws->puncta == NULL) {
    Kill_Geo3d_Scalar_Field(ws->puncta);
  }
  if (ws->on_root != NULL) {
    free(ws->on_root);
  }
  if (ws->puncta_type != NULL) {
    free(ws->puncta_type);
  }

  Default_Swc_Tree_Svg_Workspace(ws);
}

void Print_Swc_Tree_Svg_Workspace(Swc_Tree_Svg_Workspace *ws)
{
  printf("Workspace for swc->svg\n");
}

DEFINE_ZOBJECT_INTERFACE(Swc_Tree_Svg_Workspace)
