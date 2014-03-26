/**@file tz_local_neuroseg.h
 * @brief local neuroseg
 * @author Ting Zhao
 * @date 17-Jan-2008
 */

#ifndef _TZ_LOCAL_NEUROSEG_H_
#define _TZ_LOCAL_NEUROSEG_H_

#include "tz_cdefs.h"
#include "tz_neuroseg.h"
#include "tz_neuropos.h"
#include "tz_local_neuroseg_plane.h"
#include "tz_local_neuroseg_ellipse.h"
#include "tz_stack_draw.h"
#include "tz_geo3d_ellipse.h"
#include "tz_workspace.h"
struct _Pixel_Feature;

__BEGIN_DECLS

/**@addtogroup local_neuroseg_ Local Neuron segment (tz_local_neuroseg.h)
 * @{
 */

/**breif Minimal correlation coffecient for a significant fit.
 *
 */
//#define LOCAL_NEUROSEG_MIN_CORRCOEF 0.35
#define LOCAL_NEUROSEG_MIN_CORRCOEF 0.3
//#define LOCAL_NEUROSEG_MIN_CORRCOEF 0.4

/**@brief The number of parameters. 
 *
 */
const static int LOCAL_NEUROSEG_NPARAM = NEUROSEG_NPARAM + NEUROPOS_NPARAM;

#define DECLARE_LOCAL_NEUROSEG_VAR_NAME					\
  const static char *Local_Neuroseg_Var_Name[] = {			\
    "r1", "cone coefficient", "theta", "psi", "height", "curvature",	\
    "alpha", "ellipse scale", "x", "y", "z"				\
  };

#define DECLARE_LOCAL_NEUROSEG_VAR_MIN(var_min)				\
  double var_min[] = {0.5, -4.0, -Infinity, -Infinity, 2.0, 		\
		      0.0, -Infinity, 0.2,				\
		      -Infinity, -Infinity, -Infinity, 0.5};

#define DECLARE_LOCAL_NEUROSEG_VAR_MAX(var_max)				\
  double var_max[] = {50.0, 4.0, Infinity, Infinity, 30.0, \
		      NEUROSEG_MAX_CURVATURE, Infinity, 20.0,		\
		      Infinity, Infinity, Infinity, 6.0};

/*#define MIN_GRADIENT 1e-5*/ /*obsolete. minimal score gradient */

/**@brief A data structure for local neuron segment.
 *
 * Local_Neuroseg consists of two parts, a neuron segment and its position. The
 * position is the coordinates of the default reference point, which is defined
 * as Neuropos_Reference.
 */
typedef struct _Local_Neuroseg {
  Neuroseg seg;
  neuropos_t pos;
  struct _Pixel_Feature *pf1;
  struct _Pixel_Feature *pf2;
  struct _Pixel_Feature *pf3;
} Local_Neuroseg;

typedef struct _Local_Neuroseg_Slice {
  Neuroseg_Slice seg;
  neuropos_t pos;
} Local_Neuroseg_Slice;

typedef Local_Neuroseg* Local_Neuroseg_P;

/* default reference point */
const static int Neuropos_Reference = NEUROSEG_BOTTOM; 
/*
typedef struct _Locseg_Score_Workspace {
  Stack_Fit_Score fs;
  double (*field_func) (double, double);
  Stack *mask;
} Locseg_Score_Workspace;


typedef struct _Locseg_Fit_Workspace {
  int var_index[20];
  int nvar;
  int *var_link;
  double var_min[20];
  double var_max[20];
  int pos_adjust;
  Locseg_Score_Workspace *sws;
} Locseg_Fit_Workspace;
*/

/**@brief New a Local_Neuroseg.
 *
 * New_Local_Neuroseg() returns a new pointer to a local neuron segment, which 
 * could be destrcuted by Delete_Local_Neuroseg(). The new object is initialized
 * by default properties.
 */
Local_Neuroseg* New_Local_Neuroseg();

/**@brief Delete a Local_Neuroseg.
 *
 * Delete_Local_Neuroseg() deletes \a locseg.
 */
void Delete_Local_Neuroseg(Local_Neuroseg *locseg);

/**@brief Kill a Local_Neuroseg.
 *
 * Kill_Local_Neuroseg() deletes \a locseg.
 */
void Kill_Local_Neuroseg(Local_Neuroseg *locseg);

/**@brief Set the properties of a Local_Neuroseg.
 *
 * Set_Local_Neuroseg() initializes the fields of \a locseg.
 */
void Set_Local_Neuroseg(Local_Neuroseg *locseg, double r1, double c,
			double h, double theta, double psi, double curvature,
			double alpha, double scale,
			double x, double y, double z);

/**@brief Reset the properties of a Local_Neuroseg.
 *
 * Reset_Local_Neuroseg() resets \a locseg with default properties.
 */
void Default_Local_Neuroseg(Local_Neuroseg *locseg);

/**@brief Copy a local neuroseg
 *
 * Copy_Local_Neuroseg() returns a copy of \a locseg.
 */
Local_Neuroseg* Copy_Local_Neuroseg(const Local_Neuroseg *locseg);

/**@brief Copy a local neuroseg to another
 *
 * Local_Neuroseg_Copy() copies a local neuron segment from <src> to <des>.
 */
void Local_Neuroseg_Copy(Local_Neuroseg *des, const Local_Neuroseg *src);

/**@brief Print a local neuroseg to a file.
 *
 * Fprint_Local_Neuroseg() prints a neuron segment to the file stream <fp>.
 */
void Fprint_Local_Neuroseg(FILE *fp, const Local_Neuroseg *seg);

/**@brief Print a local neuroseg to standard output.
 *
 * Print_Local_Neuroseg() prints a neuron segment to the standard output.
 */
void Print_Local_Neuroseg(const Local_Neuroseg *seg);

/**@brief Import a local neuroseg from an XML file.
 *
 * Local_Neuroseg_Import_Xml() recoganizes the following fields: "r1", "c",
 * "height", "theta", "psi", "curvature", "alpha", "scale", "position".
 */
Local_Neuroseg *Local_Neuroseg_Import_Xml(const char *file_path, 
					  Local_Neuroseg *out);

/**@brief Write a local neuroseg.
 *
 * Local_Neuroseg_Fwrite() writes <locseg> to the file stream <fp>. Nothing will
 * be done if <locseg> is NULL.
 */
void Local_Neuroseg_Fwrite(const Local_Neuroseg *locseg, FILE *fp);

/**@brief Read a local neuroseg.
 *
 * Local_Neuroseg_Fread() reads a local neuroseg to <locseg> from <fp>. It 
 * supposes the segment is wrtten by Local_Neuroseg_Fwrite().
 */
Local_Neuroseg* Local_Neuroseg_Fread(Local_Neuroseg *locseg, FILE *fp);

/**@brief Write a local neuroseg array.
 *
 * Write_Local_Neuroseg_Array() writes <length> elements of the local neuroseg 
 * array <locseg> to the file <file_path> If the file exists, it will be
 * overwritten; otherwise it will be created.
 */
void Write_Local_Neuroseg_Array(const char *file_path, 
				const Local_Neuroseg *locseg, int length);

/**@brief Read a local neuroseg array.
 *
 * Read_Local_Neuroseg_Array() reads a local neuroseg <array> from the file
 * <file_path>. The number of elements that are read is stored in <length>.
 */
Local_Neuroseg* Read_Local_Neuroseg_Array(const char *file_path, int *length);


/**@brief Coordinate of an axial point
 *
 * Local_Neuroseg_Axis_Position() obtains the coordinate of a point on the axis
 * of locseg and stores it in <apos>. The point has <s> away from
 * the bottom position.
 */
void Local_Neuroseg_Axis_Position(const Local_Neuroseg *locseg,
				  double *apos, double s);

/**@brief Coordinate of a point on the axis of a local neuroseg.
 *
 * Local_Neuroseg_Axis_Coord_N() calculates the coordinate of a point on the 
 * axis of <locseg> and stores the result in <coord>. The point is specified 
 * by the regular parameter <t>.
 */
void Local_Neuroseg_Axis_Coord_N(const Local_Neuroseg *locseg, double t,
				 double *coord);

/**@brief Set local neuroseg variable.
 *
 * Local_Neuroseg_Set_Var() sets the <var_index>th variable of <seg> to <value> 
 */
void Local_Neuroseg_Set_Var(Local_Neuroseg *seg, int var_index, double value);

/**@brief Set a weighted local neuroseg variable.
 *
 * Local_Neuroseg_Set_Var() sets the <var_index>th variable of <seg> to 
 * <value * weight>.
 */
void Local_Neuroseg_Set_Var_W(Local_Neuroseg *seg, int var_index, double value,
			      double weight);

/**@brief Center of a local neuroseg.
 *
 * Local_Neuroseg_Center() stores the center position of <seg> into <pos>.
 */
void Local_Neuroseg_Center(const Local_Neuroseg *seg, double pos[]);

/**@brief Top of a local neuroseg
 *
 * Local_Neuroseg_Top()  stores the top position of <seg> into <pos>.
 */
void Local_Neuroseg_Top(const Local_Neuroseg *seg, double pos[]);

/**@brief Bottom of a local neuroseg
 *
 * Local_Neuroseg_Bottom()  stores the bottom position of <seg> into <pos>.
 */
void Local_Neuroseg_Bottom(const Local_Neuroseg *seg, double pos[]);

/**@brief Set the position of a local neuroseg.
 *
 * Set_Neuroseg_Position() set a reference point of <locseg> as <pos>. The
 * reference is specified by <ref>, which can be different from the default
 * reference. 
 */
void  Set_Neuroseg_Position(Local_Neuroseg *locseg, const double pos[],
			    Neuropos_Reference_e ref);

/**@brief Get the position of a local neuroseg.
 *
 * Get_Neuroseg_Position() retrieves the postion of <locseg> and stores it in 
 * <pos>.
 */
void  Get_Neuroseg_Position(const Local_Neuroseg *locseg, double pos[]);

/**@brief Get the normal vector of a local neuroseg.
 *
 * Local_Neuroseg_Normal() retrieves the postion of <locseg> and stores it in 
 * <pos>.
 */
void Local_Neuroseg_Normal(const Local_Neuroseg *locseg, double *ort);

/**@brief Hit test between a point and a local neuroseg
 *
 * Local_Neuroseg_Hit() returns TRUE if a point hits <locseg>, otherwise it
 * returns FALSE.
 */
BOOL Local_Neuroseg_Hit_Test(const Local_Neuroseg *locseg, 
			     double x, double y, double z);

/**@brief Hit test between two local neurosegs
 *
 * Local_Neuroseg_Hit_Test2() returns TRUE if the central axis of <locseg>
 * hits <target>.
 */
BOOL Local_Neuroseg_Hit_Test2(const Local_Neuroseg *target,
			      const Local_Neuroseg *locseg);

/**@brief Test if a point is close to the axis of a local neuroseg.
 *
 * Local_Neuroseg_Axis_Test() return TRUE iff the point (<x>, <y>, <z>) is close
 * enough to the axis of <locseg>. The closest break point is stored in <t> if
 * it is not NULL.
 */
BOOL Local_Neuroseg_Axis_Test(const Local_Neuroseg *locseg,
			      double x, double y, double z, double *t);

/*
 * Return value:
 *   
 */
/*
int Local_Neuroseg_Axis_Test2(const Local_Neuroseg *locseg,
			      const Local_Neuroseg *next_locseg, 
			      double *t);
                              */

/**@brief Test if a local neuroseg contains the axis of another
 *
 * Local_Neuroseg_Contain_Axis() returns true if <container> contains the 
 * central axis of <locseg>.
 */
BOOL Local_Neuroseg_Contain_Axis(const Local_Neuroseg *container,
				 const Local_Neuroseg *locseg);


#define Neuroseg_Fit_Score Stack_Fit_Score

#define Local_Neuroseg_Score Local_Neuroseg_Score_P

/**@brief The score that measures how well a local neuroseg fits a stack.
 *
 * Local_Neursoeg_Score_P() returns the fit score between a local neuron
 * segment and a stack. <fs> specifies options of calculating the score. It
 * also enables the function to calculate different scores at the same time.
 * The returned score is always from the first option. <fs> can also be NULL.
 * When this happens, the function returns the score of option 2.
 */
double Local_Neuroseg_Score_P(const Local_Neuroseg *locseg, const Stack *stack, 
			      double z_scale, Stack_Fit_Score *fs);

/**@brief The score that measures how well a local neuroseg fits a stack.
 *
 * Local_Neuroseg_Score_Pm() is similar to Local_Neuroseg_Score(), except that
 * the mask of the signal is provided. In <mask>, value 0 of a voxel means it is
 * available for score calculation.
 */
double Local_Neuroseg_Score_Pm(const Local_Neuroseg *locseg, 
			       const Stack *stack, double z_scale, 
			       const Stack *mask, Stack_Fit_Score *fs);

/**@brief The score that measures how well a local neuroseg fits a stack.
 *
 * Local_Neuroseg_Score_W() returns the fit score between <locseg> and <stack>.
 * It takes the workspace <ws> as input paremeters and output storage.
 */
double Local_Neuroseg_Score_W(const Local_Neuroseg *locseg, const Stack *stack, 
			      double z_scale, Locseg_Score_Workspace *ws);

/**@brief A general interface for calculating fit score.
 *
 * Local_Neuroseg_Score_R() returns the fit score of a local neuroseg specified
 * by the array <var>. The last element of <var> is z scale. <param> specifies 
 * other parameters. <param[0]> is the signal stack and <param[1]> is the score
 * workspace.
 */
double Local_Neuroseg_Score_R(const double *var, const void *param);

/**@brief Parameter array of a local neuroseg.
 *
 * Local_Neuroseg_Param_Array() retrieves the parameters of <seg> and store them
 * in <param>. <z_scale> is also stored in <param>.
 */
int Local_Neuroseg_Param_Array(const Local_Neuroseg *seg, double z_scale, 
			       double *param);

/**@brief Weighted parameter array of a local neuroseg.
 *
 * Local_Neuroseg_Param_Array_W() retrieves the parameters of <seg> and divides
 * them by <weight>. The parameters are stored in <param>. <z_scale> is also
 * stored in <param>.
 */
int Local_Neuroseg_Param_Array_W(const Local_Neuroseg *seg, double z_scale, 
				 const double *weight, double *param);

/**@brief Validate local neuroseg variables.
 *
 * Local_Neuroseg_Validate() reset each element of <var> if it is out of the bound
 * defined by <var_min> and <var_max>. <param> is just a reserved space for
 * additional arguments and can be NULL.
 */
void Local_Neuroseg_Validate(double *var, const double *var_min, 
			     const double *var_max, const void *param);

/*@brief Generic interface for score calculation.
 *
 * Local_Neuroseg_Score_G() is a generic interface for score calculation. A
 * default score option is used.
 */
double Local_Neuroseg_Score_G(const double *var, const Stack *stack);

/*@brief Generic interface for score calculation.
 *
 * Local_Neuroseg_Score_Gv() is another generic interface that can be used to
 * construct a continuous function.
 */
double Local_Neuroseg_Score_Gv(const double *var, const void *param);

/**@brief Generic interface for score calculation of weighted variables.
 */
double Local_Neuroseg_Score_Gw(const double *var, const Stack *stack,
			       const double *weight);

/**@brief Generic interface for score calculation of weighted variables.
 *
 * Local_Neuroseg_Score_Gvw() is similiar to Local_Neuroseg_Score_Gw(). But it
 * is more generic.
 */
double Local_Neuroseg_Score_Gvw(const double *var, const void *param);

/**@brief Generic interface for masked score calculation.
 *
 * Local_Neuroseg_Score_Gvm() returns the fit score of a local neuroseg
 * specified by <var>. <param[0]> is the signal stack and <param[1]> is the
 * mask.
 */
double Local_Neuroseg_Score_Gvm(const double *var, const void *param);

/**@brief Variable mask to variable indices.
 *
 * Local_Neuroseg_Var_Mask_To_Index() turns variable masks into variable
 * indices. Since a local neuron segment is composed of two parts, neuron
 * segment and its position, there are two masks to input. <neuroseg_mask> is
 * the mask for neuron segment and <neuropos_mask> is the mask for the
 * position. The result is stored in <index>. It returns the number of
 * active variables.
 */
int Local_Neuroseg_Var_Mask_To_Index(Bitmask_t neuroseg_mask, 
				     Bitmask_t neuropos_mask, int *index);


/**@brief Variable mask to variable indices.
 *
 * Local_Neuroseg_Var_Mask_To_Index_R() will add radius variables regardless if
 * it is included <neuroseg_mask>.
 */
int Local_Neuroseg_Var_Mask_To_Index_R(Bitmask_t neuroseg_mask, 
				       Bitmask_t neuropos_mask, int *index);

/*
 * Local_Neuroseg_Var() extracts variables from <locseg> and stores them in
 * <var>. Each element of <var> is the address of a variable of <locseg>.
 * It returns the number of variables.
 */
int Local_Neuroseg_Var(const Local_Neuroseg *locseg, double *var[]);


/*
 * Fit_Local_Neuroseg_P() and Fit_Local_Neuroseg_Pr() use perceptor routines
 * to fit local neuron segment. Fit_Local_Neuroseg_Pr() allows users to
 * specify the ranges of parameters, while Fit_Local_Neuroseg_P() uses default
 * ranges.
 */


/**@brief Fit a local neuroseg.
 *
 * Fit_Local_Neuroseg_P() fits <locseg> to <stack>. <var_index>, <nvar> and
 * <var_link> specifies variable configuration.
 */
double Fit_Local_Neuroseg_P(Local_Neuroseg *locseg, const Stack *stack, 
			    const int *var_index, int nvar, int *var_link,
			    double z_scale, Neuroseg_Fit_Score *fs);

double Fit_Local_Neuroseg_Pm(Local_Neuroseg *locseg, const Stack *stack, 
			     const int *var_index, int nvar, int *var_link,
			     double z_scale, const Stack *mask,
			     Neuroseg_Fit_Score *fs);

double Fit_Local_Neuroseg_Pr(Local_Neuroseg *locseg, const Stack *stack, 
			     const int *var_index, int nvar, int *var_link,
			     double *var_min, double *var_max,
			     double z_scale, Neuroseg_Fit_Score *fs);

double Fit_Local_Neuroseg_Pr_M(Local_Neuroseg *locseg, const Stack *stack, 
			       const int *var_index, int nvar, int *var_link,
			       double *var_min, double *var_max,
			       double z_scale, const Stack *mask,
			       Neuroseg_Fit_Score *fs);

/**@brief Fit a local neuroseg.
 *
 * Fit_Local_Neuroseg_W() fits <locseg> to <stack> with z scale <z_scale>.
 * The paramters of fitting are specified in <ws>. It returns the score of 
 * fitting. 
 */
double Fit_Local_Neuroseg_W(Local_Neuroseg *locseg, const Stack *stack,
			    double z_scale, Locseg_Fit_Workspace *ws);

/**@brief Search the orientation of a local neuroseg
 *
 * Local_Neuroseg_Orientation_Search() searchs a neuron segment that has the
 * best fit in the orientation space. It returns the best searched score found.
 * <locseg> will be changed to be the best-fit neuron segment.
 *
 */
double Local_Neuroseg_Orientation_Search(Local_Neuroseg *locseg, 
					 const Stack *stack, double z_scale,
					 Neuroseg_Fit_Score *fs);

/**@brief Search the position of a local neuroseg
 *
 * Local_Neuroseg_Position_Search() searchs the position a neuron segment 
 * that has the best fit along the direction <v> for 0 to <range>. The 
 * searching step is specified by <step>. It returns the best searched score 
 * found.
 */
double Local_Neuroseg_Position_Search(Local_Neuroseg *locseg, 
				      const Stack *stack, double z_scale,
				      double v[3], double range, double step,
				      Neuroseg_Fit_Score *fs);

double Local_Neuroseg_Radius_Search(Local_Neuroseg *locseg, 
				    const Stack *stack, double z_scale,
				    double start, double end, double step,
				    Neuroseg_Fit_Score *fs);
double Local_Neuroseg_Scale_Search(Local_Neuroseg *locseg, 
				   const Stack *stack, double z_scale,
				   double start, double end, double step,
				   Neuroseg_Fit_Score *fs);
double Local_Neuroseg_R_Scale_Search(Local_Neuroseg *locseg, 
				     const Stack *stack, double z_scale,
				     double r_start, double r_end, 
				     double r_step,
				     double s_start, double s_end, 
				     double s_step,
				     Neuroseg_Fit_Score *fs);

/* search orientation by fixing the bottom */
double Local_Neuroseg_Orientation_Search_B(Local_Neuroseg *locseg, 
					   const Stack *stack, double z_scale,
					   Neuroseg_Fit_Score *fs);
/* search orientation by fixing the center */
double Local_Neuroseg_Orientation_Search_C(Local_Neuroseg *locseg, 
					   const Stack *stack, double z_scale,
					   Neuroseg_Fit_Score *fs);

void Local_Neuroseg_Orientation_Adjust(Local_Neuroseg *locseg, 
				       const Stack *stack, double z_scale);

/* the bottom is fixed in height search */
/*
 *
 *
 * Local_Neuroseg_Height_Search() searches the height of a neuron segment that
 * has the best fit with <stack>. The searching step is specified by <step>. 
 * It returns the best searched score found.
 */
int Local_Neuroseg_Height_Search_P(Local_Neuroseg *locseg, 
				   const Stack *stack, double z_scale);
int Local_Neuroseg_Height_Search_E(Local_Neuroseg *locseg, int base,
				   const Stack *stack, double z_scale);
int Local_Neuroseg_Height_Search_W(Local_Neuroseg *locseg, 
				   const Stack *stack, double z_scale,
				   Locseg_Score_Workspace *sws);

void Local_Neuroseg_Position_Adjust(Local_Neuroseg *locseg, 
				    const Stack *stack, double z_scale);

BOOL Local_Neuroseg_Good_Score(Local_Neuroseg *locseg, double score, 
			       double min_score);

/* 
 * Next_Local_Neuroseg() creates a neuron segment that is next to <locseg1>.
 * The result is stored in <locseg2> and the returned pointer is also <locseg2>
 * if <locseg2> is not NULL. Otherwise the result is returned as a new object.
 * The position of the new segment is on the line of the central axis of 
 * <locseg1> and the distance between the two segments is the product between
 * <t> and the height of <locseg1>.
 */
Local_Neuroseg* Next_Local_Neuroseg(const Local_Neuroseg *locseg1, 
				    Local_Neuroseg *locseg2, double t);

/*
 * Flip_Local_Neuroseg() flips a neuron segment that is orients oppositely 
 * to the orignial direction, while the is no change of the field of the 
 * segment.
 */
void Flip_Local_Neuroseg(Local_Neuroseg *locseg);

/**@brief Test if a local neuroseg contains a certain stack value.
 *
 * Local_Neuroseg_Has_Stack_Value() returns TRUE if inside <locseg>, a voxel 
 * from <stack> has the intensity <value>; Otherwise it returns FALSE.
 */
BOOL Local_Neuroseg_Has_Stack_Value(const Local_Neuroseg *locseg, Stack *stack,
				    double z_scale, double value);

/*
 * Local_Neuroseg_Label() draws <seg> in <stack> with the specified <color>:
 *   0: red; 1: green; 2: blue; -1: dark
 *
 * Local_Neuroseg_Label_G() labels a grey stack by a constant value.
 */
void Local_Neuroseg_Label(const Local_Neuroseg *seg, Stack *stack, int color,
			  double z_scale);
void Local_Neuroseg_Label_G(const Local_Neuroseg *seg, Stack *stack, int flag,
			    int value, double z_scale);
void Local_Neuroseg_Label_C(const Local_Neuroseg *seg, Stack *stack, 
			    double z_scale, const color_t color);

/*
 * Local_Neuroseg_Label_W() draws <seg> in <stack> in a way specified by the
 * workspace <ws>.
 */
void Local_Neuroseg_Label_W(const Local_Neuroseg *seg, Stack *stack, 
			    double z_scale, Locseg_Label_Workspace *ws);

/*
 * Local_Neuroseg_Field_S() returns the scalar field as a special data
 * structure. Local_Neuroseg_Field_Sp() does a similar job but only samples
 * withing a non-negative range.
 */
Geo3d_Scalar_Field* Local_Neuroseg_Field_S(const Local_Neuroseg *locseg, 
					   Neuroseg_Field_f field_func,
					   Geo3d_Scalar_Field *field);
Geo3d_Scalar_Field* Local_Neuroseg_Field_Sp(const Local_Neuroseg *locseg, 
					    Neuroseg_Field_f field_func,
					    Geo3d_Scalar_Field *field);
Geo3d_Scalar_Field* Local_Neuroseg_Field_Z(const Local_Neuroseg *locseg,
					   double z, double step,
					   Neuroseg_Field_f field_func,
					   Geo3d_Scalar_Field *field);

/*
 * Local_Neuroseg_From_Geo3d_Circle() turns a 3D circle to a neuron segment.
 */
void Local_Neuroseg_From_Geo3d_Circle(Local_Neuroseg *seg, 
				      const Geo3d_Circle *circle);

/*
 * Local_Neuroseg_Height_Profile() returns an array that stores the fit to the
 * signal <stack> along the segment orientation. The array has the same address
 * as <profile> when <profile> is not NULL.
 */
double* Local_Neuroseg_Height_Profile(const Local_Neuroseg *locseg, 
				      const Stack *stack, double z_scale,
				      int n, int option, 
				      Neuroseg_Field_f field_func,
				      double *profile);

void Local_Neuroseg_Fprint_Vrml(const Local_Neuroseg *seg, FILE *stream);

/*
 * Local_Neuroseg_To_Plane() turns the plane of a neuron segment at <z> 
 * relative to its bottom to a plane.
 */
Local_Neuroseg_Plane* Local_Neuroseg_To_Plane(const Local_Neuroseg *seg, 
					      double z,
					      Local_Neuroseg_Plane *locnp);
/*
Geo3d_Circle* 
Local_Neuroseg_To_Circle_Z(const Local_Neuroseg *locseg, 
			   double z, Geo3d_Circle *circle);
Geo3d_Circle* 
Local_Neuroseg_To_Circle_T(const Local_Neuroseg *locseg, 
			   double t, Geo3d_Circle *circle);
*/

Geo3d_Circle* Local_Neuroseg_To_Circle(const Local_Neuroseg *locseg,
				       Neuropos_Reference_e ref, int option,
				       Geo3d_Circle *circle);
Geo3d_Circle* 
Local_Neuroseg_To_Circle_Z(const Local_Neuroseg *locseg, 
			   double z, int option, Geo3d_Circle *circle);
Geo3d_Circle* 
Local_Neuroseg_To_Circle_T(const Local_Neuroseg *locseg, 
			   double t, int option, Geo3d_Circle *circle);

Local_Neuroseg_Ellipse* 
Local_Neuroseg_To_Ellipse_Z(const Local_Neuroseg *seg, 
			    double z, Local_Neuroseg_Ellipse *locne);
Local_Neuroseg_Ellipse* 
Local_Neuroseg_To_Ellipse_T(const Local_Neuroseg *locseg, 
			    double t, Local_Neuroseg_Ellipse *locne);

Local_Neuroseg_Ellipse* 
Local_Neuroseg_Central_Ellipse(const Local_Neuroseg *seg, 
			       Local_Neuroseg_Ellipse *locne);
coordinate_3d_t* 
Local_Neuroseg_Halo_Points(const Local_Neuroseg *locseg,
			   int nsample, coordinate_3d_t *pts);

Local_Neuroseg* Local_Neuroseg_From_Plane(Local_Neuroseg *seg,
					  const Local_Neuroseg_Plane *locnp);

/* inverse z-scaling*/
void Local_Neuroseg_Scale_Z(Local_Neuroseg *locseg, double z_scale);

/* xy-scaling */
void Local_Neuroseg_Scale_XY(Local_Neuroseg *locseg, double xy_scale);

/* xy-scaling and z-scaling*/
void Local_Neuroseg_Scale(Local_Neuroseg *locseg, double xy_scale, 
			  double z_scale);

/**@brief Change the top of a local neuroseg.
 *
 * Local_Neuroseg_Chain_Top() sets the top of <locseg> to <new_top> while
 * keeping the bottom unchanged. If the distance between <new_top> and the
 * bottom of <locseg> is less than 0.1, the change is considered as unreliable.
 * In this case, the function sets the height of <locseg> to 1 without change
 * other attributes.
 */
void Local_Neuroseg_Change_Top(Local_Neuroseg *locseg, const double *new_top);
void Local_Neuroseg_Change_Bottom(Local_Neuroseg *locseg, 
				  const double *new_bottom);
void Local_Neuroseg_Change_Height(Local_Neuroseg *locseg, double new_height);

void Local_Neuroseg_Set_Bottom_Top(Local_Neuroseg *locseg, const double *bottom,
				   const double *top);

void Local_Neuroseg_Break_Gap(Local_Neuroseg *locseg,
			      const Stack *stack, double z_scale);

void Local_Neuroseg_Chop(Local_Neuroseg *locseg, double ratio);

double Local_Neuroseg_Average_Signal(const Local_Neuroseg *locseg, 
				     const Stack *stack, double z_scale);

double Local_Neuroseg_Average_Weak_Signal(const Local_Neuroseg *locseg, 
					  const Stack *stack, double z_scale);

Geo3d_Ball* Local_Neuroseg_Ball_Bound(const Local_Neuroseg *locseg,
				      Geo3d_Ball *ball);

/**@brief Turns a neuroseg into a circle.
 *
 * Local_Neuroseg_To_Geo3d_Circle() stores a circle from \a locseg in \a sc.
 * The circle can be the bottom, center or top of \a locseg according to the
 * value of \a ref.
 */
/*
void Local_Neuroseg_To_Geo3d_Circle(const Local_Neuroseg *locseg,
				    Geo3d_Circle *sc,
				    Neuropos_Reference_e ref);

void Local_Neuroseg_To_Geo3d_Circle_Z(const Local_Neuroseg *locseg,
				      Geo3d_Circle *sc,
				      double z);

void Local_Neuroseg_To_Geo3d_Circle_S(const Local_Neuroseg *locseg,
				      Geo3d_Circle *sc,
				      Neuropos_Reference_e ref,
				      double xy_scale, double z_scale);
*/
void Local_Neuroseg_To_Geo3d_Ellipse_Z(const Local_Neuroseg *locseg,
				       Geo3d_Ellipse *sc,
				       double z);

void Local_Neuroseg_To_Ellipse(const Local_Neuroseg *locseg,
			       Local_Neuroseg_Ellipse *locne,
			       Neuropos_Reference_e ref);

#define NEUROCOMP_LOCAL_NEUROSEG(nc) ((Local_Neuroseg*) ((nc)->data))

void Local_Neuroseg_Draw_Stack(Local_Neuroseg *locseg, Stack *stack,
			       const Stack_Draw_Workspace *ws);

int Local_Neuroseg_Stack_Feature(Local_Neuroseg *locseg, Stack *stack,
				  double z_scale, double *feats);

double Local_Neuroseg_Optimize(Local_Neuroseg *locseg, const Stack *stack, 
			       double z_scale, int option);

double Local_Neuroseg_Optimize_W(Local_Neuroseg *locseg, const Stack *stack, 
				 double z_scale, int option,
				 Locseg_Fit_Workspace *ws);

double Local_Neuroseg_Array_Maxr(Local_Neuroseg *locseg, int length);
/**@}*/

void Local_Neuroseg_Stack_Position(const double position[3], int c[3], 
				   double offpos[3], double z_scale);

int Local_Neuroseg_Hit_Mask(const Local_Neuroseg *locseg, const Stack *mask,
			    double z_scale);

double Local_Neuroseg_Center_Sample(const Local_Neuroseg *locseg, 
				    const Stack *stack, double z_scale);

double Local_Neuroseg_Top_Sample(const Local_Neuroseg *locseg, 
				 const Stack *stack, double z_scale);

double Local_Neuroseg_Planar_Dist(const Local_Neuroseg *locseg1,
				  const Local_Neuroseg *locseg2);

double Local_Neuroseg_Planar_Dist_L(const Local_Neuroseg *locseg1,
				    const Local_Neuroseg *locseg2);

void Local_Neuroseg_Stretch(Local_Neuroseg *locseg, double scale, 
			    double offset, int direction);

void Local_Neuroseg_Swc_Fprint_T(FILE *fp, const Local_Neuroseg *locseg,
				 int start_id, int parent_id, int type);
void Local_Neuroseg_Swc_Fprint(FILE *fp, const Local_Neuroseg *locseg,
			       int start_id, int parent_id);

double Local_Neuroseg_Point_Dist(const Local_Neuroseg *locseg,
				 double x, double y, double z, double res);

double Local_Neuroseg_Lineseg_Dist(const Local_Neuroseg *locseg,
				   const coordinate_3d_t start, 
				   const coordinate_3d_t end, double res);

double Local_Neuroseg_Dist2(const Local_Neuroseg *locseg1, 
			    const Local_Neuroseg *locseg2, double *pos);

double Local_Neuroseg_Lineseg_Dist_S(const Local_Neuroseg *locseg,
				     const coordinate_3d_t start, 
				     const coordinate_3d_t end, double *pos);

/**@brief Distance between a point and surface of a local neuroseg
 *
 * Local_Neuroseg_Point_Dist_S() returns the distance between the surface of
 * <locseg> and (<x>, <y>, <z>). It returns 0 if (<x>, <y>, <z>) is inside 
 * <locseg>. When <pt> is not NULL, it stores the closest point on the surface 
 * to <pt> or (<x>, <y>, <z>) itself if the point is inside. 
 */
double Local_Neuroseg_Point_Dist_S(const Local_Neuroseg *locseg, 
				   double x, double y, double z,
				   double *pt);

Stack* Local_Neuroseg_Stack(const Local_Neuroseg *locseg, const Stack *stack);

void Local_Neuroseg_Rotate(Local_Neuroseg *locseg, double theta, double psi);

/**@brief Translate a local neuroseg.
 *
 * Local_Neuroseg_Translate() translates <locseg> by <offset>.
 */
void Local_Neuroseg_Translate(Local_Neuroseg *locseg, const double *offset);

/**@brief Intersection of two local neuroseg
 *
 * Local_Neuroseg_Intersect() returns the distance between the axis of <locseg1>
 * and the axis of <locseg2>. The two points that give the distance are stored
 * as breaks in <t1> and <ts>.
 */
double Local_Neuroseg_Intersect(const Local_Neuroseg *locseg1, 
				const Local_Neuroseg *locseg2,
				double *t1, double *t2);

Neuropos_Reference_e Local_Neuroseg_Fixon(const Local_Neuroseg *locseg1, 
					  const Local_Neuroseg *locseg2, 
					  double *t);

/**@brief Compare the tangent vectors of two local neurosegs.
 *
 * See the M document for the meaning of the returned value.
 */
int Local_Neuroseg_Tangent_Compare(const Local_Neuroseg *locseg1,
				   const Local_Neuroseg *locseg2);

/**@brief Connection features of two local neuroseg.
 */
double* Locseg_Conn_Feature(const Local_Neuroseg *locseg1, 
			    const Local_Neuroseg *locseg2,
			    const Stack *stack, const double *res, 
			    double *feat, int *n);

double Local_Neuroseg_Zscore(const Local_Neuroseg *locseg);

Local_Neuroseg* Local_Neuroseg_From_Field(Geo3d_Scalar_Field *field,
    Local_Neuroseg *locseg);
__END_DECLS

#endif
