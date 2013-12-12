/**@file tz_trace_utils.h
 * @brief tracing utilities
 * @author Ting Zhao
 * @date 02-JUL-2008
 */

#ifndef _TZ_TRACE_UTILS_H_
#define _TZ_TRACE_UTILS_H_

#include "tz_cdefs.h"
#include "tz_trace_defs.h"
#include "tz_local_neuroseg.h"

__BEGIN_DECLS

/**@addtogroup trace_utils_ Tracing utilities (tz_trace_utils.h)
 * @{
 */

Trace_Workspace* New_Trace_Workspace();
void Default_Trace_Workspace(Trace_Workspace *tw);
void Clean_Trace_Workspace(Trace_Workspace *tw);
void Kill_Trace_Workspace(Trace_Workspace *tw);

void Trace_Workspace_Set_Trace_Status(Trace_Workspace *tw, int forward_status, 
				      int backward_status);

int Trace_Workspace_Mask_Value(const Trace_Workspace *tw, 
			       const double pos[3]);
int Trace_Workspace_Mask_Value_Z(const Trace_Workspace *tw, 
				 double pos[3], double z_scale);

BOOL Trace_Workspace_Point_In_Bound(const Trace_Workspace *tw, 
				    const double pos[3]);
BOOL Trace_Workspace_Point_In_Bound_Z(const Trace_Workspace *tw, 
				      double pos[3], double z_scale);
void Trace_Workspace_Set_Fit_Mask(Trace_Workspace *tw, Stack *mask);
BOOL Trace_Workspace_Is_Masked(const Trace_Workspace *tw);
void Print_Trace_Workspace(const Trace_Workspace *tw);

void Print_Trace_Status(int status);

Trace_Record* New_Trace_Record();
void Reset_Trace_Record(Trace_Record *tr);

void Delete_Trace_Record(Trace_Record *tr);

Trace_Record* Copy_Trace_Record(Trace_Record *tr);

void Fprint_Trace_Record(FILE *fp, const Trace_Record *tr);
void Print_Trace_Record(const Trace_Record *tr);

void Trace_Record_Fwrite(const Trace_Record *tr, FILE *stream);
Trace_Record* Trace_Record_Fread(Trace_Record *tr, FILE *stream);

void Trace_Record_Fwrite_V(const Trace_Record *tr, FILE *stream, 
			   double version);
Trace_Record* Trace_Record_Fread_V(Trace_Record *tr, FILE *stream, 
				   double version);

void Trace_Record_Copy(Trace_Record *des, const Trace_Record *src);

void Trace_Record_Set_Score(Trace_Record *tr, const Stack_Fit_Score *fs);
void Trace_Record_Set_Hit_Region(Trace_Record *tr, int hit_region);
void Trace_Record_Set_Index(Trace_Record *tr, int index);
void Trace_Record_Set_Refit(Trace_Record *tr, BOOL is_refit);
void Trace_Record_Set_Direction(Trace_Record *tr, Dlist_Direction_e direction);
void Trace_Record_Set_Fit_Height(Trace_Record *tr, int index, int value);
void Trace_Record_Set_Fix_Point(Trace_Record *tr, double value);
BOOL Trace_Record_Has_Fix_Point(Trace_Record *tr);
void Trace_Record_Disable_Fix_Point(Trace_Record *tr);
int Trace_Record_Index(const Trace_Record *tr);
int Trace_Record_Refit(const Trace_Record *tr);
int Trace_Record_Fit_Height(const Trace_Record *tr, int index);
Dlist_Direction_e Trace_Record_Direction(const Trace_Record *tr);
double Trace_Record_Fix_Point(const Trace_Record *tr);

typedef struct _Trace_Evaluate_Seed_Workspace {
  int score_option;
  double min_score;
  int nseed;
  int fit_option;
  BOOL zshift;
  Stack *base_mask;
  Stack *trace_mask;
  Locseg_Fit_Workspace *fws;
  double *score;
  Local_Neuroseg *locseg;
} Trace_Evaluate_Seed_Workspace;

Trace_Evaluate_Seed_Workspace* New_Trace_Evaluate_Seed_Workspace();
void Default_Trace_Evaluate_Seed_Workspace(Trace_Evaluate_Seed_Workspace *ws);
void Clean_Trace_Evaluate_Seed_Workspace(Trace_Evaluate_Seed_Workspace *ws);
void Kill_Trace_Evaluate_Seed_Workspace(Trace_Evaluate_Seed_Workspace *ws);

/**@brief Evalulate a set of seeds.
  *
  * Trace_Evaluate_Seed() evaluates the points in <seed> and calculates how well
  * each point can be a tracing seed. The result is stored in <ws->score> and
  * <ws->locseg>.
 */
void Trace_Evaluate_Seed(const Geo3d_Scalar_Field *seed, 
			 const Stack *stack, double z_scale, 
			 Trace_Evaluate_Seed_Workspace *ws);

Connection_Test_Workspace* New_Connection_Test_Workspace();
void Default_Connection_Test_Workspace(Connection_Test_Workspace *ctw);
void Delete_Connection_Test_Workspace(Connection_Test_Workspace *ctw);
void Kill_Connection_Test_Workspace(Connection_Test_Workspace *ctw);

void Connection_Test_Workspace_Read_Resolution(Connection_Test_Workspace *ctw,
					       const char *filepath);

#ifdef _DEBUG_
void stack_adjust_zpos(const Stack *stack, int x, int y, int *z);
#endif

/**@}*/

__END_DECLS

#endif
