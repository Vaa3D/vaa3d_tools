/* tz_geo3d_scalar_field.c
 *
 * 17-Apr-2008 Initial write: Ting Zhao
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <utilities.h>
#include "tz_error.h"
#include "tz_darray.h"
#include "tz_stack_sampling.h"
#include "tz_stack_draw.h"
#include "tz_geo3d_point_array.h"
#include "tz_geo3d_utils.h"
#include "tz_geo3d_scalar_field.h"
#include "tz_math.h"
#include "tz_string.h"

INIT_EXCEPTION

void Stack_Fit_Score_Copy(Stack_Fit_Score *des, const Stack_Fit_Score *src)
{
  memcpy(des, src, sizeof(Stack_Fit_Score));
}

void Fprint_Stack_Fit_Score(FILE *fp, const Stack_Fit_Score *fs)
{
  fprintf(fp, "Scores:");
  int i;
  if (fs->n > 0) {
    for (i = 0; i < fs->n; i++) {
      fprintf(fp, " (%d - %g);", fs->options[i], fs->scores[i]);
    }
  } else {
    fprintf(fp, " No score available.");
  }
  fprintf(fp, "\n");
}

void Print_Stack_Fit_Score(const Stack_Fit_Score *fs)
{
  Fprint_Stack_Fit_Score(stdout, fs);
}

void Stack_Fit_Score_Fwrite(const Stack_Fit_Score *fs, FILE *stream)
{
  TZ_ASSERT(fs != NULL, "Null pointer");
  TZ_ASSERT(fs->n <= STACK_FIT_SCORE_NUMBER, "Invalid fit score number");

  fwrite(&(fs->n), sizeof(fs->n), 1, stream);
  if (fs->n > 0) {
    fwrite(fs->scores, sizeof(fs->scores[0]), fs->n, stream);
    fwrite(fs->options, sizeof(fs->options[0]), 
	   fs->n, stream);
  }
}

Stack_Fit_Score* Stack_Fit_Score_Fread(Stack_Fit_Score *fs, FILE *stream)
{
  fread(&(fs->n), sizeof(fs->n), 1, stream);
  TZ_ASSERT(fs->n <= STACK_FIT_SCORE_NUMBER, "Invalid fit score number");
  if (fs->n > 0) {
    fread(fs->scores, sizeof(fs->scores[0]), fs->n, stream);
    fread(fs->options, sizeof(fs->options[0]), fs->n, stream);
  }

  return fs;
}

void reset_geo3d_scalar_field(Geo3d_Scalar_Field* field)
{
  field->size = 0;
  field->points = NULL;
  field->values = NULL;
}

Geo3d_Scalar_Field* New_Geo3d_Scalar_Field()
{
  Geo3d_Scalar_Field *field = (Geo3d_Scalar_Field *) 
    Guarded_Malloc(sizeof(Geo3d_Scalar_Field), "New_Geo3d_Scalar_Field");
  reset_geo3d_scalar_field(field);

  return field;
}

void Delete_Geo3d_Scalar_Field(Geo3d_Scalar_Field *field)
{
  free(field);
}

Geo3d_Scalar_Field*
Construct_Geo3d_Scalar_Field(Geo3d_Scalar_Field *field, int size)
{
  if ((field->points != NULL) || (field->values != NULL)){
    PRINT_EXCEPTION("Construction failed", 
		    "Geo3d_Scalar_Field cannot be constructed " 
		    "because it's not empty");
    return NULL;
  }

  field->size = size;
  field->points = (coordinate_3d_t *) 
    Guarded_Malloc(sizeof(coordinate_3d_t) * size, 
		   "Construct_Geo3d_Scalar_Field");
  field->values = (double *) 
    Guarded_Malloc(sizeof(double) * size, 
		   "Construct_Geo3d_Scalar_Field");

  return field;
}

void Clean_Geo3d_Scalar_Field(Geo3d_Scalar_Field *field)
{
  free(field->points);
  free(field->values);
  reset_geo3d_scalar_field(field);
}

Geo3d_Scalar_Field* Make_Geo3d_Scalar_Field(int size)
{
  Geo3d_Scalar_Field *field = New_Geo3d_Scalar_Field();
  Construct_Geo3d_Scalar_Field(field, size);

  return field;
}

void Kill_Geo3d_Scalar_Field(Geo3d_Scalar_Field *field)
{
  Clean_Geo3d_Scalar_Field(field);
  Delete_Geo3d_Scalar_Field(field);
}


Geo3d_Scalar_Field*
Resize_Geo3d_Scalar_Field(Geo3d_Scalar_Field *field, int size)
{
  if (field->points == NULL){
    PRINT_EXCEPTION("Resize failed", 
		    "Geo3d_Scalar_Field cannot be resized because it's empty ");
    return NULL;
  }

  if (field->size < size) {
    field->points = (coordinate_3d_t *) 
      Guarded_Realloc(field->points, sizeof(coordinate_3d_t) * size, 
          "Construct_Geo3d_Scalar_Field");

    if (field->values != NULL) {
      field->values = (double *) 
        Guarded_Realloc(field->values, sizeof(double) * size, 
            "Construct_Geo3d_Scalar_Field");
    }
  }

  field->size = size;

  return field;
}

Geo3d_Scalar_Field* Copy_Geo3d_Scalar_Field(const Geo3d_Scalar_Field *field)
{
  Geo3d_Scalar_Field *copy = Make_Geo3d_Scalar_Field(field->size);
  memcpy(copy->points, field->points, sizeof(coordinate_3d_t) * field->size);
  memcpy(copy->values, field->values, sizeof(double) * field->size);

  return copy;
}

void Print_Geo3d_Scalar_Field(const Geo3d_Scalar_Field *field)
{
  int i;
  for (i = 0; i < field->size; i++) {
    printf("(%g, %g, %g): %g\n", field->points[i][0], field->points[i][1],
	   field->points[i][2], field->values[i]);
  }
  printf("Number of points: %d\n", field->size);
}

void Print_Geo3d_Scalar_Field_Info(const Geo3d_Scalar_Field *field)
{
  printf("3D scalar field: %d points\n", field->size);
}

Geo3d_Scalar_Field* Read_Geo3d_Scalar_Field(const char *file_path)
{
  FILE *fp = fopen(file_path, "r");
  if (fp == NULL) {
    PRINT_EXCEPTION("IO error", "Cannot open file to read");
    return NULL;
  }

  Geo3d_Scalar_Field *field = New_Geo3d_Scalar_Field();
  fread(&(field->size), sizeof(int), 1, fp);
  Construct_Geo3d_Scalar_Field(field, field->size);
  fread(field->points, sizeof(coordinate_3d_t), field->size, fp);
  fread(field->values, sizeof(double), field->size, fp);
  fclose(fp);

  return field;
}

void Write_Geo3d_Scalar_Field(const char *file_path, 
			      const Geo3d_Scalar_Field *field)
{
  FILE *fp = fopen(file_path, "w");
  if (fp == NULL) {
    PRINT_EXCEPTION("IO error", "Cannot open file to write");
    return;
  }

  fwrite(&(field->size), sizeof(int), 1, fp);
  fwrite(field->points, sizeof(coordinate_3d_t), field->size, fp);
  fwrite(field->values, sizeof(double), field->size, fp);
  fclose(fp);
}

void Geo3d_Scalar_Field_Export_V3d_Marker(const Geo3d_Scalar_Field *field,
					  const char *file_path)
{
  FILE *fp = fopen(file_path, "w");

  if (fp == NULL) {
    TZ_ERROR(ERROR_IO_OPEN);
  }

  int i;
  for (i = 0; i < field->size; i++) {
    fprintf(fp, "%d,%d,%d,%d\n", iround(field->points[i][0]),
	    iround(field->points[i][1]), iround(field->points[i][2]),
	    iround(field->values[i]));
  }

  fclose(fp);
}

/* weight option: 
 * 1: max intensity; 2: intensity; 3: sdev; 4: volsize; 5: mass */
Geo3d_Scalar_Field *Geo3d_Scalar_Field_Import_Apo_E(const char *file_path,
    int weight_option)
{
  FILE *fp = GUARDED_FOPEN(file_path, "r");

  String_Workspace *sw = New_String_Workspace();

  int n = 0;
  while (Read_Line(fp, sw) != NULL) {
    n++;
  }

  Geo3d_Scalar_Field *field = Make_Geo3d_Scalar_Field(n);
  field->size = 0;

  fseek(fp, 0, SEEK_SET);

  char *line = NULL;
  while ((line = Read_Line(fp, sw)) != NULL) {
    char *token = strsplit(line, ',', 4);
    if (token != NULL) {
      field->points[field->size][2] = atof(token);
    } else {
      continue;
    }

    token = strsplit(token, ',', 1);
    if (token != NULL) {
      field->points[field->size][0] = atof(token);
    } else {
      continue;
    }

    token = strsplit(token, ',', 1);
    if (token != NULL) {
      field->points[field->size][1] = atof(token);
    } else {
      continue;
    }

    token = strsplit(token, ',', weight_option);
    if (token != NULL) {
      field->values[field->size] = atof(token);
    } else {
      continue;
    }

    field->size++;
  }

  Kill_String_Workspace(sw);

  fclose(fp);

  return field;
}

Geo3d_Scalar_Field *Geo3d_Scalar_Field_Import_Apo(const char *file_path)
{
  return Geo3d_Scalar_Field_Import_Apo_E(file_path, 2);
  /*
  FILE *fp = GUARDED_FOPEN(file_path, "r");

  String_Workspace *sw = New_String_Workspace();

  int n = 0;
  while (Read_Line(fp, sw) != NULL) {
    n++;
  }

  Geo3d_Scalar_Field *field = Make_Geo3d_Scalar_Field(n);
  field->size = 0;

  fseek(fp, 0, SEEK_SET);

  char *line = NULL;
  while ((line = Read_Line(fp, sw)) != NULL) {
    char *token = strsplit(line, ',', 4);
    if (token != NULL) {
      field->points[field->size][2] = atof(token);
    } else {
      continue;
    }

    token = strsplit(token, ',', 1);
    if (token != NULL) {
      field->points[field->size][0] = atof(token);
    } else {
      continue;
    }

    token = strsplit(token, ',', 1);
    if (token != NULL) {
      field->points[field->size][1] = atof(token);
    } else {
      continue;
    }

    token = strsplit(token, ',', 2);
    if (token != NULL) {
      field->values[field->size] = atof(token);
    } else {
      continue;
    }

    field->size++;
  }

  Kill_String_Workspace(sw);

  fclose(fp);

  return field;
  */
}

void Geo3d_Scalar_Field_Boundbox(const Geo3d_Scalar_Field *field,
				 coordinate_3d_t *boundbox)
{
  Coordinate_3d_Copy(boundbox[0], field->points[0]);
  Coordinate_3d_Copy(boundbox[1], field->points[0]);
  
  int i, j;
  for (i = 1; i < field->size; i++) {
    for (j = 0; j < 3; j++) {
      if (field->points[i][j] <= boundbox[0][j]) {
	boundbox[0][j] = field->points[i][j];
      } else if (field->points[i][j] > boundbox[1][j]) {
	boundbox[1][j] = field->points[i][j];
      }
    }
  }
}

void Geo3d_Scalar_Field_Draw_Stack(const Geo3d_Scalar_Field *field, 
				   Stack *stack, const double *coef,
				   const double *range)
{
  ASSERT(field != NULL, "NULL field.");
  ASSERT(stack != NULL, "NULL stack.");

  int i;
  /* Erase the stack. */
  for (i = 0; i < field->size; i++) {
    Stack_Draw_Point(stack, field->points[i][0], field->points[i][1], 
		     field->points[i][2], 0.0, 0);
  }

  double value;
  for (i = 0; i < field->size; i++) {    
    value = field->values[i];

    if (range != NULL) {
      if ((value <= range[0]) || (value > range[1])) {
	continue;
      }
    }

    if (coef != NULL) {
      value = (value + coef[0]) * coef[1];
    }

    Stack_Draw_Point(stack, field->points[i][0], field->points[i][1], 
		     field->points[i][2], value, 1);
  }  
}

static double field_stack_fit_statistic(double *filter, double *signal, 
					int length)
{
  double stat = 0.0;
  double mu = darray_mean_n(signal, length);
  double var = 0.0;

  int i;
  for (i = 0; i < length; i++) {
    if (!isnan(signal[i])) {
      var += (signal[i] - mu) * (signal[i] - mu);
    }
  }

  if (var == 0.0) {
    stat = 0.0;
  } else {
    int length2 = 0;
    double score = darray_dot_n(filter, signal, length);
    double filter_sum = 0; //darray_sum(filter, length);
    double filter_square_sum = 0;
    for (i = 0; i < length; i++) {
      if (!isnan(signal[i])) {
	filter_sum += filter[i];
	filter_square_sum += filter[i] * filter[i];
	length2++;
      }
    }
    var /= (double) (length2 - 1);
    
    stat = (score - filter_sum * mu) / sqrt(filter_square_sum * var);
  }

  return stat;
}

double* Geo3d_Scalar_Field_Stack_Sampling(const Geo3d_Scalar_Field *field,
					  const Stack *stack, double z_scale,
					  double *signal)
{
  ASSERT_NOT_NULL(field);
  ASSERT_NOT_NULL(stack);

  if (signal == NULL) {
    signal = darray_malloc(field->size); 
  }

  if (z_scale == 1.0) {
    Stack_Points_Sampling(stack, Coordinate_3d_Double_Array(field->points), 
			  field->size, signal);
  } else {
    Stack_Points_Sampling_Z(stack, z_scale, 
			    Coordinate_3d_Double_Array(field->points),
			    field->size, signal);
  }

  return signal;
}

double* Geo3d_Scalar_Field_Stack_Sampling_W(const Geo3d_Scalar_Field *field,
    const Stack *stack, double z_scale, double *signal)

{
  ASSERT_NOT_NULL(field);
  ASSERT_NOT_NULL(stack);
  TZ_ASSERT(field->values != signal, "Conflicting arrays.");
  
  if (signal == NULL) {
    signal = darray_malloc(field->size); 
  }

  if (z_scale == 1.0) {
    Stack_Points_Sampling(stack, Coordinate_3d_Double_Array(field->points), 
			  field->size, signal);
  } else {
    Stack_Points_Sampling_Z(stack, z_scale, 
			    Coordinate_3d_Double_Array(field->points),
			    field->size, signal);
  }

  int i;
  for (i = 0; i < field->size; i++) {
    signal[i] *= field->values[i];
  }

  return signal;
}

double* Geo3d_Scalar_Field_Stack_Sampling_M(const Geo3d_Scalar_Field *field,
					    const Stack *stack, double z_scale,
					    const Stack *mask,
					    double *signal)
{
  if (signal == NULL) {
    signal = darray_malloc(field->size); 
  }

  if (z_scale == 1.0) {
    Stack_Points_Sampling_M(stack, mask,
			    Coordinate_3d_Double_Array(field->points), 
			    field->size, signal);
  } else {
    Stack_Points_Sampling_Zm(stack, z_scale, mask,
			    Coordinate_3d_Double_Array(field->points),
			    field->size, signal);
  }

  return signal;  
}

double Geo3d_Scalar_Field_Stack_Score(const Geo3d_Scalar_Field *field,
				      const Stack *stack, double z_scale,
				      Stack_Fit_Score *fs)
{
  ASSERT(field != NULL, "NULL field.");
  ASSERT(stack != NULL, "NULL stack.");

  /* alloc <signal> */
  double *signal = darray_malloc(field->size);

  if (z_scale == 1.0) {
    Stack_Points_Sampling(stack, Coordinate_3d_Double_Array(field->points), 
			  field->size, signal);
  } else {
    Stack_Points_Sampling_Z(stack, z_scale, 
			    Coordinate_3d_Double_Array(field->points),
			    field->size, signal);
  }

  double score = 0.0;
  
#ifdef _DEBUG_2
  darray_write("../data/pos.bn", Coordinate_3d_Double_Array(field->points), field->size * 3);
  darray_write("../data/filter.bn", field->values, field->size);
  darray_write("../data/signal.bn", signal, field->size);
#endif

  if (fs != NULL) {
    int i, j;
    for (j = 0; j < fs->n; j++) {
      switch (fs->options[j]) {
      case STACK_FIT_DOT:
	fs->scores[j] = darray_dot_n(field->values, signal, field->size);
	break;
      case STACK_FIT_CORRCOEF:
	fs->scores[j] = darray_corrcoef_n(field->values, signal, field->size);	
	break;
      case STACK_FIT_EDOT:
	fs->scores[j] = darray_dot_n(field->values, signal, field->size) + 
	  darray_sum_n(signal, field->size);
	break;
      case STACK_FIT_STAT:
	fs->scores[j] = field_stack_fit_statistic(field->values, 
						  signal, field->size);
	break;
      case STACK_FIT_PDOT:
	fs->scores[j] = 0;
	for (i = 0; i < field->size; i++) {
	  if (!isnan(signal[i]) && (field->values[i] >= 0.0)) {
	    fs->scores[j] += field->values[i] * signal[i];
	  }
	}
	break;
      case STACK_FIT_MEAN_SIGNAL:
	{
	  int n = 0;
	  fs->scores[j] = 0;
	  for (i = 0; i < field->size; i++) {
	    if (!isnan(signal[i]) && (field->values[i] >= 0.0)) {
	      fs->scores[j] += signal[i];
	      n++;
	    }
	  }
	  fs->scores[j] /= n;
	  break;
	}
      case STACK_FIT_LOW_MEAN_SIGNAL:
	{
	  int n = 0;
	  double mean = 0;
	  for (i = 0; i < field->size; i++) {
	    if (!isnan(signal[i]) && (field->values[i] >= 0.0)) {
	      mean += signal[i];
	      n++;
	    }
	  }
	  mean /= n;
	  fs->scores[j] = 0;
	  n = 0;
	  for (i = 0; i < field->size; i++) {
	    if (!isnan(signal[i]) && (field->values[i] >= 0.0) 
		&& (signal[i] < mean)) {
	      fs->scores[j] += signal[i];
	      n++;
	    }
	  }
	  if (n > field->size / 10) {
	    fs->scores[j] /= n;
	  } else {
	    fs->scores[j] = mean;
	  }
	  break;
	}
      case STACK_FIT_CORRCOEF_SC:
	fs->scores[j] = darray_corrcoef_n(field->values, signal, field->size) * 
	  darray_max(signal, field->size, NULL);
	break;
      case STACK_FIT_DOT_CENTER:
	fs->scores[j] = darray_dot_n(field->values, signal, field->size);
	coordinate_3d_t c1, c2;
	Geo3d_Scalar_Field_Center(field, c1);
	Geo3d_Scalar_Field field2;
	field2.size = field->size;
	field2.points = field->points;
	field2.values = signal;
	Geo3d_Scalar_Field_Center(&field2, c2);
	fs->scores[j] /= (5.0 + Coordinate_3d_Distance(c1, c2));
	//printf("offcenter value: %g\n", Coordinate_3d_Distance(c1, c2));
	break;
      case STACK_FIT_OUTER_SIGNAL:
	{
	  int n = 0;
	  fs->scores[j] = 0;
	  for (i = 0; i < field->size; i++) {
	    if (!isnan(signal[i]) && (field->values[i] < 0.0)) {
	      fs->scores[j] += signal[i];
	      n++;
	    }
	  }
	  fs->scores[j] /= n;
	  break;
	}
      case STACK_FIT_VALID_SIGNAL_RATIO:
	fs->scores[j] = 0;
	for (i = 0; i < field->size; i++) {
	  if (!isnan(signal[i])) {
	    fs->scores[j]++;
	  }
	}
	fs->scores[j] /= field->size;
	break;
      default:
	TZ_ERROR(ERROR_DATA_VALUE);
	break;
      }
    }
    score = fs->scores[0];
  } else {
    //score = darray_corrcoef(field->values, signal, field->size) * 
    //  darray_max(signal, field->size, NULL);
    score = darray_dot_n(field->values, signal, field->size);// * norm_factor;
    /*
    coordinate_3d_t c1, c2;
    Geo3d_Scalar_Field_Centroid(field, c1);
    Geo3d_Scalar_Field field2;
    field2.size = field->size;
    field2.points = field->points;
    field2.values = signal;
    Geo3d_Scalar_Field_Centroid(&field2, c2);
    score /= (50.0 + Coordinate_3d_Distance(c1, c2));
    */
    //printf("offcenter value: %g\n", Coordinate_3d_Distance(c1, c2));
    //score = darray_simscore(signal, field->values, field->size);
    //score = darray_corrcoef(field->values, signal, field->size);
  }
  
  /* free <signal> */
  free(signal);

  return score;  
}

double Geo3d_Scalar_Field_Stack_Score_M(const Geo3d_Scalar_Field *field,
					const Stack *stack, double z_scale,
					const Stack *mask,
					Stack_Fit_Score *fs)
{
  ASSERT(field != NULL, "NULL field.");
  ASSERT(stack != NULL, "NULL stack.");

  /* alloc <signal> */
  double *signal = darray_malloc(field->size);

  if (z_scale == 1.0) {
    Stack_Points_Sampling_M(stack, mask, 
			    Coordinate_3d_Double_Array(field->points), 
			    field->size, signal);
  } else {
    Stack_Points_Sampling_Zm(stack, z_scale, mask,
			    Coordinate_3d_Double_Array(field->points),
			    field->size, signal);
  }

  double score = 0.0;
  
#ifdef _DEBUG_2
  darray_write("../data/pos.bn", Coordinate_3d_Double_Array(field->points), field->size * 3);
  darray_write("../data/filter.bn", field->values, field->size);
  darray_write("../data/signal.bn", signal, field->size);
#endif

  if (fs != NULL) {
    int i, j;
    for (j = 0; j < fs->n; j++) {
      switch (fs->options[j]) {
      case STACK_FIT_DOT:
	fs->scores[j] = darray_dot_n(field->values, signal, field->size);
	break;
      case STACK_FIT_CORRCOEF:
	fs->scores[j] = darray_corrcoef_n(field->values, signal, field->size);	
	break;
      case STACK_FIT_EDOT:
	fs->scores[j] = darray_dot_n(field->values, signal, field->size) + 
	  darray_sum_n(signal, field->size);
	break;
      case STACK_FIT_STAT:
	fs->scores[j] = field_stack_fit_statistic(field->values, 
						  signal, field->size);
	break;
      case STACK_FIT_PDOT:
	fs->scores[j] = 0;
	for (i = 0; i < field->size; i++) {
	  if (!isnan(signal[i]) && (field->values[i] > 0.0)) {
	    fs->scores[j] += field->values[i] * signal[i];
	  }
	}
	break;
      case STACK_FIT_MEAN_SIGNAL:
	{
	  int n = 0;
	  fs->scores[j] = 0;
	  for (i = 0; i < field->size; i++) {
	    if (!isnan(signal[i]) && (field->values[i] > 0.0)) {
	      fs->scores[j] += signal[i];
	      n++;
	    }
	  }
	  fs->scores[j] /= n;
	  break;
	}
      case STACK_FIT_CORRCOEF_SC:
	fs->scores[j] = darray_corrcoef_n(field->values, signal, field->size) * 
	  darray_max(signal, field->size, NULL);
	break;
      case STACK_FIT_DOT_CENTER:
	fs->scores[j] = darray_dot_n(field->values, signal, field->size);
	coordinate_3d_t c1, c2;
	Geo3d_Scalar_Field_Center(field, c1);
	Geo3d_Scalar_Field field2;
	field2.size = field->size;
	field2.points = field->points;
	field2.values = signal;
	Geo3d_Scalar_Field_Center(&field2, c2);
	fs->scores[j] /= (5.0 + Coordinate_3d_Distance(c1, c2));
	//printf("offcenter value: %g\n", Coordinate_3d_Distance(c1, c2));
	break;
      case STACK_FIT_OUTER_SIGNAL:
	{
	  int n = 0;
	  fs->scores[j] = 0;
	  for (i = 0; i < field->size; i++) {
	    if (!isnan(signal[i]) && (field->values[i] < 0.0)) {
	      fs->scores[j] += signal[i];
	      n++;
	    }
	  }
	  fs->scores[j] /= n;
	  break;
	}
      case STACK_FIT_VALID_SIGNAL_RATIO:
	fs->scores[j] = 0;
	for (i = 0; i < field->size; i++) {
	  if (!isnan(signal[i])) {
	    fs->scores[j]++;
	  }
	}
	fs->scores[j] /= field->size;
	break;
      default:
	THROW(ERROR_DATA_VALUE);
	break;
      }
    }
    score = fs->scores[0];
  } else {
    //score = darray_corrcoef(field->values, signal, field->size) * 
    //  darray_max(signal, field->size, NULL);
    score = darray_dot_nw(field->values, signal, field->size);// * norm_factor;
    //score = darray_corrcoef_n(field->values, signal, field->size);
    /*
    coordinate_3d_t c1, c2;
    Geo3d_Scalar_Field_Centroid(field, c1);
    Geo3d_Scalar_Field field2;
    field2.size = field->size;
    field2.points = field->points;
    field2.values = signal;
    Geo3d_Scalar_Field_Centroid(&field2, c2);
    score /= (50.0 + Coordinate_3d_Distance(c1, c2));
    */
    //printf("offcenter value: %g\n", Coordinate_3d_Distance(c1, c2));
    //score = darray_simscore(signal, field->values, field->size);
    //score = darray_corrcoef(field->values, signal, field->size);
  }
  
  /* free <signal> */
  free(signal);

  return score;    
}

Geo3d_Scalar_Field* Geo3d_Scalar_Field_Merge(const Geo3d_Scalar_Field *field1,
					     const Geo3d_Scalar_Field *field2,
					     Geo3d_Scalar_Field *field)
{
  int size1 = field1->size;

  if (field == NULL) {
    field = Make_Geo3d_Scalar_Field(field1->size + field2->size);
  } else {
    Resize_Geo3d_Scalar_Field(field, field1->size + field2->size);
  }

  if (field1 != field) {
    memcpy(field->points, field1->points, 
        sizeof(coordinate_3d_t) * field1->size);
    memcpy(field->values, field1->values, sizeof(double) * field1->size);
  }

  memcpy(field->points + size1, field2->points, 
      sizeof(coordinate_3d_t) * field2->size);
  memcpy(field->values + size1, field2->values, 
	 sizeof(double) * field2->size);  
  
  return field;
}

void Geo3d_Scalar_Field_Translate(Geo3d_Scalar_Field *field,
				  double x, double y, double z)
{
  Geo3d_Point_Array_Translate(field->points, field->size, x, y, z);
}

void Geo3d_Scalar_Field_Center(const Geo3d_Scalar_Field *field, 
			       coordinate_3d_t center)
{
  ASSERT(field != NULL, "null field");

  int i;
  center[0] = 0.0;
  center[1] = 0.0;
  center[2] = 0.0;

  for (i = 0; i < field->size; i++) {
    center[0] += field->points[i][0];
    center[1] += field->points[i][1];
    center[2] += field->points[i][2];
  }

  center[0] /= field->size;
  center[1] /= field->size;
  center[2] /= field->size;
}

void Geo3d_Scalar_Field_Centroid(const Geo3d_Scalar_Field *field, 
				 coordinate_3d_t centroid)
{
  ASSERT(field != NULL, "null field");

  double weight = 0.0;
  int i;
  centroid[0] = 0.0;
  centroid[1] = 0.0;
  centroid[2] = 0.0;

  for (i = 0; i < field->size; i++) {
    if (!isnan(field->values[i])) {
      weight += field->values[i];
      centroid[0] += field->points[i][0] * field->values[i];
      centroid[1] += field->points[i][1] * field->values[i];
      centroid[2] += field->points[i][2] * field->values[i];
    }
  }

  if (weight == 0.0) {
    Geo3d_Scalar_Field_Center(field, centroid);
  } else {
    centroid[0] /= weight;
    centroid[1] /= weight;
    centroid[2] /= weight;
  }
}

void Geo3d_Scalar_Field_Cov(const Geo3d_Scalar_Field *field, double *cov)
{
  coordinate_3d_t centroid;
  Geo3d_Scalar_Field_Centroid(field, centroid);
  int i, j;
  double weight = 0.0;
  double mean_shift[3];
  
  for (j = 0; j < 9; j++) {
    cov[j] = 0.0;
  }
  
  for (i = 0; i < field->size; i++) {
    if (!isnan(field->values[i])) {
      weight += field->values[i];
      for (j = 0; j < 3; j++) {
	mean_shift[j] = field->points[i][j] - centroid[j];
      }
      cov[0] += mean_shift[0] * mean_shift[0] * field->values[i]; /*00*/
      cov[1] += mean_shift[0] * mean_shift[1] * field->values[i]; /*01*/
      cov[2] += mean_shift[0] * mean_shift[2] * field->values[i]; /*02*/
      cov[4] += mean_shift[1] * mean_shift[1] * field->values[i]; /*11*/
      cov[5] += mean_shift[1] * mean_shift[2] * field->values[i]; /*12*/
      cov[8] += mean_shift[2] * mean_shift[2] * field->values[i]; /*22*/
    }
  }

  cov[3] = cov[1];
  cov[6] = cov[2];
  cov[7] = cov[5];

  if (weight != 0.0) {
    for (j = 0; j < 9; j++) {
      cov[j] /= weight;
    }
  }
}

void Geo3d_Scalar_Field_Ort(const Geo3d_Scalar_Field *field, double *vec,
			    double *ext)
{
  double cov[9];
  Geo3d_Scalar_Field_Cov(field, cov);
  Geo3d_Cov_Orientation(cov, vec, ext);
}

void Geo3d_Scalar_Field_Pca(const Geo3d_Scalar_Field *field, double *value,
    double *vec)
{
  double cov[9];
  Geo3d_Scalar_Field_Cov(field, cov);
#ifdef _DEBUG_2
  darray_print2(cov, 3, 3);
#endif

  if (Matrix_Eigen_Value_Cs(cov[0], cov[4], cov[8], cov[1], cov[2], cov[5], 
	value) == 0) {
    Matrix_Eigen_Vector_Cs(cov[0], cov[4], cov[8], cov[1], cov[2], cov[5], 
	value[0], vec);
    Matrix_Eigen_Vector_Cs(cov[0], cov[4], cov[8], cov[1], cov[2], cov[5], 
	value[1], vec + 3);
    if (Compare_Float(value[1], value[2], 1e-5) == 0) {
      Geo3d_Cross_Product(vec[0], vec[1], vec[2], vec[3], vec[4], vec[5],
	  vec + 6, vec + 7, vec + 8);
    } else {
      Matrix_Eigen_Vector_Cs(cov[0], cov[4], cov[8], cov[1], cov[2], cov[5], 
	  value[2], vec + 6);
    }
  } else {
    int i;
    for (i = 0; i < 9; i++) {
      vec[i] = 0;
    }
    vec[0] = 1;
    vec[3] = 1;
    vec[6] = 1;
  }
}
