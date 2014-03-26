/**@file tz_geo3d_scalar_field.h
 * @brief 3d scalar field
 * @author Ting Zhao
 * @date 17-Apr-2008 
 */

#ifndef _TZ_GEO3D_SCALAR_FIELD_H_
#define _TZ_GEO3D_SCALAR_FIELD_H_

#include "tz_cdefs.h"
#include "tz_image_lib_defs.h"
#include "tz_coordinate_3d.h"

__BEGIN_DECLS

/**@addtogroup 3d_scalar_field_ 3D Scalar field samples.
 * @{
 */

/**@brief 3D scalar field samples.
 *
 * Geo3d_Scalar_Field is a data structure for a discrete 3D scalar-value field
 * such as an intensity field. It contains a set of points, each of which has an
 * assiciated scalar value.
 * In the strcutre, <values[i]> is the value of the point
 * (<points[i][0]>, <points[i][1]>, <points[i][2]>).
 */
typedef struct _Geo3d_Scalar_Field {
  int size;                   /* number of points */
  coordinate_3d_t *points;    /* coordinates of the points */
  double *values;             /* values or weights of the points */
} Geo3d_Scalar_Field;

enum {
  STACK_FIT_DOT = 0,     /* 0 dot product*/
  STACK_FIT_CORRCOEF,    /* 1 correlation coefficient */
  STACK_FIT_EDOT,        /* 2 dot product plus intensity */
  STACK_FIT_STAT,        /* 3 testing statistic */
  STACK_FIT_PDOT,        /* 4 dot product of positive field */
  STACK_FIT_MEAN_SIGNAL, /* 5 average signal in positive field */
  STACK_FIT_CORRCOEF_SC, /* 6 scaled correlation coefficient */
  STACK_FIT_DOT_CENTER,  /* 7 dot product + offcenter penalty */
  STACK_FIT_OUTER_SIGNAL, /* 8 average surrounding signal */
  STACK_FIT_VALID_SIGNAL_RATIO, /* 9 how much signal is valid */
  STACK_FIT_LOW_MEAN_SIGNAL, /* 10 average weak signal in positive field */
};

/* maximum number of scores that can be stored */
#define STACK_FIT_SCORE_NUMBER 10 

typedef struct _Stack_Fit_Score {
  int n;                                  /* number of options */
  double scores[STACK_FIT_SCORE_NUMBER];  /* array to store results */
  int options[STACK_FIT_SCORE_NUMBER];    /* option array */  
} Stack_Fit_Score;

void Fprint_Stack_Fit_Score(FILE *fp, const Stack_Fit_Score *fs);
void Print_Stack_Fit_Score(const Stack_Fit_Score *fs);
void Stack_Fit_Score_Copy(Stack_Fit_Score *des, const Stack_Fit_Score *src);

void Stack_Fit_Score_Fwrite(const Stack_Fit_Score *fs, FILE *stream);
Stack_Fit_Score* Stack_Fit_Score_Fread(Stack_Fit_Score *fs, FILE *stream);

Geo3d_Scalar_Field* New_Geo3d_Scalar_Field();
void Delete_Geo3d_Scalar_Field(Geo3d_Scalar_Field *field);

Geo3d_Scalar_Field* 
Construct_Geo3d_Scalar_Field(Geo3d_Scalar_Field *field, int size);
void Clean_Geo3d_Scalar_Field(Geo3d_Scalar_Field *field);

Geo3d_Scalar_Field* Make_Geo3d_Scalar_Field(int size);
void Kill_Geo3d_Scalar_Field(Geo3d_Scalar_Field *field);

Geo3d_Scalar_Field*
Resize_Geo3d_Scalar_Field(Geo3d_Scalar_Field *field, int size);
  
Geo3d_Scalar_Field* Copy_Geo3d_Scalar_Field(const Geo3d_Scalar_Field *field);

void Print_Geo3d_Scalar_Field(const Geo3d_Scalar_Field *field);
void Print_Geo3d_Scalar_Field_Info(const Geo3d_Scalar_Field *field);

Geo3d_Scalar_Field* Read_Geo3d_Scalar_Field(const char *file_path);
void Write_Geo3d_Scalar_Field(const char *file_path, 
			      const Geo3d_Scalar_Field *field);

/**@brief Export a field to a V3D marker file.
 */
void Geo3d_Scalar_Field_Export_V3d_Marker(const Geo3d_Scalar_Field *field,
					  const char *file_path);

/**@brief Import an APO file.
 *
 * Geo3d_Scalar_Field_Import_Apo() imports an apo file <file_path> and turns 
 * the points into a scalar field. The value of each point is its intensity.
 */
Geo3d_Scalar_Field *Geo3d_Scalar_Field_Import_Apo(const char *file_path);

/**@brief Import an APO file.
 *
 * Geo3d_Scalar_Field_Import_Apo() imports an apo file <file_path> and turns 
 * the points into a scalar field. The value of each point is determined by
 * weight_option: .
 * 1: max intensity; 2: intensity; 3: sdev; 4: volsize; 5: mass
 */
Geo3d_Scalar_Field *Geo3d_Scalar_Field_Import_Apo_E(const char *file_path,
    int weight_option);

/**@brief Bounding box of a scalar field.
 *
 * Geo3d_Scalar_Field_Boundbox() returns the bounding box of <field>.
 * <boundbox[0]> is the left-top corner and <boundbox[1]> is the right-bottom
 * conner.
 */
void Geo3d_Scalar_Field_Boundbox(const Geo3d_Scalar_Field *field,
				 coordinate_3d_t *boundbox);

/**@brief Draw a scalar field.
 *
 * Geo3d_Scalar_Field_Draw_Stack() draws a field in <stack>. The intensity of
 * field is adjusted by (I + coef[0]) * coef[1] if <coef> is not NULL. Only
 * points that have intensities within (range[0], range[1]) are drawn if <range>
 * is not NULL.
 */
void Geo3d_Scalar_Field_Draw_Stack(const Geo3d_Scalar_Field *field, 
				   Stack *stack, const double *coef,
				   const double *range);

/**@addtogroup scalar_field_score_ Scalar field fit score
 * @{
 */

/**@brief fit score between a field and a stack
 *
 * Geo3d_Scalar_Field_Stack_Score() calculates the score of fit between <field>
 * and <stack>.
 */
double Geo3d_Scalar_Field_Stack_Score(const Geo3d_Scalar_Field *field,
				      const Stack *stack, double z_scale,
				      Stack_Fit_Score *fs);


/**@brief Sample a stack.
 *
 * Geo3d_Scalar_Field_Stack_Sampling() samples the intensity field of <stack> 
 * for the points in <field>. The result is stored in <signal> if it is not 
 * NULL. 
 */
double* Geo3d_Scalar_Field_Stack_Sampling(const Geo3d_Scalar_Field *field,
					  const Stack *stack, double z_scale,
					  double *signal);

/**@brief Sample a stack with weights. */
double* Geo3d_Scalar_Field_Stack_Sampling_W(const Geo3d_Scalar_Field *field,
    const Stack *stack, double z_scale, double *signal);

/**@brief Sample a masked stack.
 *
 * Geo3d_Scalar_Field_Stack_Sampling_M() does similar jobs as 
 * Geo3d_Scalar_Field_Satck_Sampling except that the intensity field is masked.
 * When a point is in a masked region (label 1), its sampling value is NaN.
 */
double* Geo3d_Scalar_Field_Stack_Sampling_M(const Geo3d_Scalar_Field *field,
					    const Stack *stack, double z_scale,
					    const Stack *mask,
					    double *signal);

double Geo3d_Scalar_Field_Stack_Score_M(const Geo3d_Scalar_Field *field,
					const Stack *stack, double z_scale,
					const Stack *mask,
					Stack_Fit_Score *fs);

/**@}
 */

/**@brief merge two fields
 *
 * Geo3d_Scalar_Field_Merge() merges two fields and returns it as a new object
 * if <field> is NULL. If <field> is not NULL, the returned object will be 
 * <field> itself. <field> can be the same as <field1>.
 */
Geo3d_Scalar_Field* Geo3d_Scalar_Field_Merge(const Geo3d_Scalar_Field *field1,
					     const Geo3d_Scalar_Field *field2,
					     Geo3d_Scalar_Field *field);

/*
 * Geo3d_Scalar_Field_Translate() translate <field> with (<x>, <y>, <z>).
 */
void Geo3d_Scalar_Field_Translate(Geo3d_Scalar_Field *field,
				  double x, double y, double z);

/**@addtogroup field_attr_ field attributes
 * @{
 */
/**@brief Center of the field.
 */
void Geo3d_Scalar_Field_Center(const Geo3d_Scalar_Field *field, 
			       coordinate_3d_t center);

/**@brief Centroid of a field.
 *
 * Geo3d_Scalar_Field() calculates the centroid of <field> and stores the
 * result in <centroid>.
 */
void Geo3d_Scalar_Field_Centroid(const Geo3d_Scalar_Field *field, 
				 coordinate_3d_t centroid);
void Geo3d_Scalar_Field_Cov(const Geo3d_Scalar_Field *field, double *cov);
void Geo3d_Scalar_Field_Ort(const Geo3d_Scalar_Field *field, double *vec,
			    double *ext);
void Geo3d_Scalar_Field_Pca(const Geo3d_Scalar_Field *field, double *value,
    double *vec);
/**@}
 */

/**@}*/

__END_DECLS

#endif
