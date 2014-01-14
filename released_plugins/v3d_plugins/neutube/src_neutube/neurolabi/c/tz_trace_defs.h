/**@file tz_trace_defs.h
 * @brief definitions for tracing
 * @author Ting Zhao
 * @date 20-May-2008
 */

#ifndef _TZ_TRACE_DEFS_H_
#define _TZ_TRACE_DEFS_H_ 

#include "tz_cdefs.h"
#include "tz_bitmask.h"
#include "tz_geo3d_scalar_field.h"
#include "tz_doubly_linked_list_defs.h"
#include "tz_fmatrix.h"

__BEGIN_DECLS

#define CHAIN_VERSION 3.0

typedef int Trace_Test_f(void *argv[]);

/**@struct _Trace_Workspace tz_trace_defs.h
 *
 * Tracing workspace
 */
typedef struct _Trace_Workspace {
  int length;      /**< maximum length to trace */
  BOOL fit_first;  /**< fit the first node or not */
  BOOL refit;      /**< refit or not */
  BOOL break_refit; /**< break refit or not */
  BOOL tune_end; /**< tune ends or not */
  int tscore_option; /**< option of thresholding score */
  double min_score;  /**< minimal score */
  double min_chain_length; /**< the minimal length to be valid*/
  double trace_step; /* step size of tracing */
  double seg_length; /* length of an initial segment */
  int chain_id; /** id of the chain */
  Dlist_Direction_e trace_direction; /**< obsolete field */
  int trace_status[2]; /**< tracing status. s[0] forward, s[1] backward */
  int stop_reason[2];  /**< obsolete field */
  double trace_range[6];  /**< range to trace */
  Stack *sup_stack; /**< supervising stack */
  Stack *trace_mask;  /**< tracing mask */
  BOOL trace_mask_updating; /**< update tracing mask or not */
  Stack *swc_mask;  /**< mask for swc */
  BOOL canvas_updating; /**< update canvas or not */
  Stack *canvas;  /**< canvas for drawing the result */
  double dyvar[5]; /**< reserved field, which has different meanings for
		      different routines. */
  Trace_Test_f *test_func; /**< validation function */
  double resolution[3]; /**< voxel size (unit: um) */
  char save_path[256]; /**< place to save the chain(s) */
  char save_prefix[256]; /**< prefix for the chain files */
  void *fit_workspace; /**< additional parameters for fitting */
  BOOL add_hit; /**< Add hit or not. Default: TRUE */

  FMatrix *Ixx;
  FMatrix *Iyy;
  FMatrix *Izz;
  FMatrix *Ixy;
  FMatrix *Iyz;
  FMatrix *Ixz;
} Trace_Workspace;

/* masks for Trace_Record */
enum {
  TRACE_RECORD_FIT_SCORE_MASK = 1,
  TRACE_RECORD_HIT_REGION_MASK,
  TRACE_RECORD_INDEX_MASK,
  TRACE_RECORD_REFIT_MASK,
  TRACE_RECORD_FIT_HEIGHT_MASK,
  TRACE_RECORD_DIRECTION_MASK,
  TRACE_RECORD_FIX_POINT_MASK
};

/**@struct _Trace_Record tz_trace_defs.h
 *
 * Tracing record
 */
typedef struct _Trace_Record {
  Bitmask_t mask;
  Stack_Fit_Score fs;
  int hit_region;
  int index;
  int refit; /* 0: no refit; 1: refit */
  int fit_height[2];
  Dlist_Direction_e direction;
  double fix_point;
} Trace_Record;

/* tracing status */
enum {
  TRACE_NORMAL = 0,        /* Good, go on. */
  TRACE_HIT_MARK,          /* In a marked region, which has been taken by someone else, stop */
  TRACE_LOW_SCORE,         /* Low score, the signal is not interesting enough, stop */
  TRACE_TOO_LARGE,         /* Too thick, stop */
  TRACE_TOO_SMALL,         /* Too thin, stop */
  TRACE_INVALID_SHAPE,     /* Irregular shape, stop */
  TRACE_LOOP_FORMED,       /* Loop formed, stop */
  TRACE_OUT_OF_BOUND,      /* out of bound, stop */
  TRACE_REPEATED,          /* repeat the same step , stop */
  TRACE_OVER_REFIT,        /* too much refit */
  TRACE_SEED_OUT_OF_BOUND, /* start from a out-of-bound seed */
  TRACE_SIZE_CHANGE,       /* size change too fast */
  TRACE_SIGNAL_CHANGE,     /* signal chainge too fast */
  TRACE_OVERLAP,           /* an overlapping segment */
  TRACE_REFIT,             /* refit required */
  TRACE_NOT_ASSIGNED       /* not asked to trace */
};

typedef struct _Connection_Test_Workspace {
  int hook_spot;
  double dist;
  double cos1;
  double cos2;
  double dist_thre;
  BOOL good_dist;
  double resolution[3];
  char unit; /* 'p' for pixel; 'u' for um */
  Stack *mask;
  double big_euc;
  double big_planar;
  BOOL sp_test;
  BOOL interpolate;
  BOOL crossover_test;
} Connection_Test_Workspace;

__END_DECLS

#endif
