/**@file tz_workspace.h
 * @author Ting Zhao
 * @date 24-Apr-2009
 */

#ifndef _TZ_WORKSPACE_H_
#define _TZ_WORKSPACE_H_

#include "tz_cdefs.h"
#include "tz_image_lib_defs.h"
#include "tz_geo3d_scalar_field.h"
#include "tz_zobject.h"
#include "tz_bitmask.h"

__BEGIN_DECLS

typedef struct _Receptor_Score_Workspace {
  Stack_Fit_Score fs;
  double (*field_func) (double, double);
  Stack *mask;
} Receptor_Score_Workspace;

#define RECEPTOR_MAX_VAR_NUMBER 20
typedef struct _Receptor_Fit_Workspace {
  int var_index[RECEPTOR_MAX_VAR_NUMBER];
  int nvar;
  int *var_link;
  double var_min[RECEPTOR_MAX_VAR_NUMBER];
  double var_max[RECEPTOR_MAX_VAR_NUMBER];
  int pos_adjust;
  Receptor_Score_Workspace *sws;
} Receptor_Fit_Workspace;

#define Locseg_Score_Workspace Receptor_Score_Workspace
#define Locseg_Fit_Workspace Receptor_Fit_Workspace

#define Locseg_Ellipse_Score_Workspace Receptor_Score_Workspace
#define Locseg_Ellipse_Fit_Workspace Receptor_Fit_Workspace

enum { LOCSEG_LABEL_OPTION_GREY = 1, LOCSEG_LABEL_OPTION_WEAK, 
  LOCSEG_LABEL_OPTION_WEAK_MEAN, LOCSEG_LABEL_OPTION_WEAK_MIN, 
  LOCSEG_LABEL_OPTION_ADD = 6, LOCSEG_LABEL_OPTION_SUB,
  LOCSEG_LABEL_OPTION_SKEL = 10, LOCSEG_LABEL_OPTION_DRAW };

/* workspace for labeling a local neuroseg 
 * option: 1 - grey stack labeling (single value)
 *         2 - weak signal labeling for grey stack
 *         3 - weak signal labeling, comparing to neighbor average
 *         4 - weak signal labeling, comapring to neighbor minimum
 *
 *         6 - gray stack accumulation
 *         7 - gray stack subtraction
 *
 *         10 - skeleton labeling
 *         11 - stack drawing (receptor field)
 *
 */
typedef struct _Locseg_Label_Workspace {
  Stack *signal;
  int option;
  int flag; /* Only those with the value <flag> will be labeled if <flag> is
	       non-negative. Otherwise there is no value constraint. */
  color_t color;
  int value;
  double sratio;
  double sdiff;
  double slimit;
  Stack *buffer_mask; /* A buffer mask for adding and subtraction */
  int range[6]; /* [sx, sy, sz, ex, ey, ez]*/
} Locseg_Label_Workspace;

Locseg_Label_Workspace* New_Locseg_Label_Workspace();
void Default_Locseg_Label_Workspace(Locseg_Label_Workspace *ws);

void Delete_Locseg_Label_Workspace(Locseg_Label_Workspace *ws);
void Clean_Locseg_Label_Workspace(Locseg_Label_Workspace *ws);
void Kill_Locseg_Label_Workspace(Locseg_Label_Workspace *ws);

void Print_Locseg_Label_Workspace(const Locseg_Label_Workspace *ws);

Receptor_Score_Workspace* New_Receptor_Score_Workspace();
void Default_Receptor_Score_Workspace(Receptor_Score_Workspace *ws);

void Delete_Receptor_Score_Workspace(Receptor_Score_Workspace *ws);
void Clean_Receptor_Score_Workspace(Receptor_Score_Workspace *ws);
void Kill_Receptor_Score_Workspace(Receptor_Score_Workspace *ws);

void Receptor_Score_Workspace_Copy(Locseg_Score_Workspace *dst, 
				   const Receptor_Score_Workspace *src);

#define New_Locseg_Score_Workspace New_Receptor_Score_Workspace
#define Default_Locseg_Score_Workspace Default_Receptor_Score_Workspace
#define Delete_Locseg_Score_Workspace Delete_Receptor_Score_Workspace
#define Clean_Locseg_Score_Workspace Clean_Receptor_Score_Workspace
#define Kill_Locseg_Score_Workspace Kill_Receptor_Score_Workspace
#define Locseg_Score_Workspace_Copy Receptor_Score_Workspace_Copy
#define Print_Locseg_Score_Workspace Print_Receptor_Score_Workspace

#define New_Locseg_Ellipse_Score_Workspace New_Receptor_Score_Workspace
#define Default_Locseg_Ellipse_Score_Workspace Default_Receptor_Score_Workspace
#define Delete_Locseg_Ellipse_Score_Workspace Delete_Receptor_Score_Workspace
#define Clean_Locseg_Ellipse_Score_Workspace Clean_Receptor_Score_Workspace
#define Kill_Locseg_Ellipse_Score_Workspace Kill_Receptor_Score_Workspace
#define Locseg_Ellipse_Score_Workspace_Copy Receptor_Score_Workspace_Copy
#define Print_Locseg_Ellipse_Score_Workspace Print_Receptor_Score_Workspace

/*
void Default_Locseg_Fit_Workspace(Locseg_Fit_Workspace *ws);
void Delete_Locseg_Fit_Workspace(Locseg_Fit_Workspace *ws);
void Clean_Locseg_Fit_Workspace(Locseg_Fit_Workspace *ws);
void Kill_Locseg_Fit_Workspace(Locseg_Fit_Workspace *ws);
void Locseg_Fit_Workspace_Copy(Locseg_Fit_Workspace *dst, 
			       const Locseg_Fit_Workspace *src);
void Print_Locseg_Fit_Workspace(const Locseg_Fit_Workspace *ws);
*/

/*
void Delete_Receptor_Fit_Workspace(Receptor_Fit_Workspace *ws);
void Clean_Receptor_Fit_Workspace(Receptor_Fit_Workspace *ws);
void Kill_Receptor_Fit_Workspace(Receptor_Fit_Workspace *ws);
void Print_Receptor_Fit_Workspace(const Receptor_Fit_Workspace *ws);
*/
DECLARE_ZOBJECT_INTERFACE(Receptor_Fit_Workspace)

void Receptor_Fit_Workspace_Copy(Receptor_Fit_Workspace *dst, 
			       const Receptor_Fit_Workspace *src);

Locseg_Fit_Workspace* New_Locseg_Fit_Workspace();
void Default_Locseg_Fit_Workspace(Locseg_Fit_Workspace *ws);
#define Delete_Locseg_Fit_Workspace Delete_Receptor_Fit_Workspace
#define Clean_Locseg_Fit_Workspace Clean_Receptor_Fit_Workspace
#define Kill_Locseg_Fit_Workspace Kill_Receptor_Fit_Workspace
#define Locseg_Fit_Workspace_Copy Receptor_Fit_Workspace_Copy
#define Print_Locseg_Fit_Workspace Print_Receptor_Fit_Workspace

void Default_Rpi_Locseg_Fit_Workspace(Receptor_Fit_Workspace *ws);
void Default_R2_Rect_Fit_Workspace(Receptor_Fit_Workspace *ws);
void Default_R2_Ellipse_Fit_Workspace(Receptor_Fit_Workspace *ws);

Locseg_Ellipse_Fit_Workspace* New_Locseg_Ellipse_Fit_Workspace();
void Default_Locseg_Ellipse_Fit_Workspace(Locseg_Fit_Workspace *ws);
#define Delete_Locseg_Ellipse_Fit_Workspace Delete_Receptor_Fit_Workspace
#define Clean_Locseg_Ellipse_Fit_Workspace Clean_Receptor_Fit_Workspace
#define Kill_Locseg_Ellipse_Fit_Workspace Kill_Receptor_Fit_Workspace
#define Locseg_Ellipse_Fit_Workspace_Copy Receptor_Fit_Workspace_Copy
#define Print_Locseg_Ellipse_Fit_Workspace Print_Receptor_Fit_Workspace

void Locseg_Fit_Workspace_Enable_Cone(Locseg_Fit_Workspace *ws);
void Locseg_Fit_Workspace_Disable_Cone(Locseg_Fit_Workspace *ws);


void Locseg_Fit_Workspace_Set_Var(Locseg_Fit_Workspace *ws,
				  Bitmask_t neuroseg_mask, 
				  Bitmask_t neuropos_mask);
				 
typedef struct _Locseg_Chain_Skel_Workspace {
  Locseg_Fit_Workspace *fw;
  double sr;
  double ss;
  double er;
  double es;
} Locseg_Chain_Skel_Workspace;

DECLARE_ZOBJECT_INTERFACE(Locseg_Chain_Skel_Workspace)


enum { SWC_SVG_LENGTH_WEIGHT, SWC_SVG_SURFAREA_WEIGHT };
enum { SWC_SVG_COLOR_CODE_NONE = 0, SWC_SVG_COLOR_CODE_Z = 1, 
  SWC_SVG_COLOR_CODE_INTENSITY, SWC_SVG_COLOR_CODE_SURFAREA };

typedef struct _Swc_Tree_Svg_Workspace {
  Geo3d_Scalar_Field *puncta;
  BOOL *on_root;
  int *puncta_type;
  BOOL shuffling;
  BOOL showing_count;
  BOOL showing_length;
  int color_code;
  int max_vx;
  int max_vy;
  int weight_option;
} Swc_Tree_Svg_Workspace;

DECLARE_ZOBJECT_INTERFACE(Swc_Tree_Svg_Workspace)
  
__END_DECLS

#endif
