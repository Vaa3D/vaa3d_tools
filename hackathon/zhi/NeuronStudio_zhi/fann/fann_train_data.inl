/*
 * Fast Artificial Neural Network Library (fann) Copyright (C) 2003
 * Steffen Nissen (lukesky@diku.dk)
 * 
 * This library is free software; you can redistribute it and/or modify it 
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "config.h"
#include "fann.h"

/*
 * Reads training data from a file. 
 */
FANN_EXTERNAL struct fann_train_data *FANN_API fann_read_train_from_file(const char *configuration_file)
{
	struct fann_train_data *data;
	FILE *file = fopen(configuration_file, "r");

	if(!file)
	{
		fann_error(NULL, FANN_E_CANT_OPEN_CONFIG_R, configuration_file);
		return NULL;
	}

	data = fann_read_train_from_fd(file, configuration_file);
	fclose(file);
	return data;
}

/*
 * Save training data to a file 
 */
FANN_EXTERNAL int FANN_API fann_save_train(struct fann_train_data *data, const char *filename)
{
	return fann_save_train_internal(data, filename, 0, 0);
}

/*
 * Save training data to a file in fixed point algebra. (Good for testing
 * a network in fixed point) 
 */
FANN_EXTERNAL int FANN_API fann_save_train_to_fixed(struct fann_train_data *data, const char *filename,
													 unsigned int decimal_point)
{
	return fann_save_train_internal(data, filename, 1, decimal_point);
}

/*
 * deallocate the train data structure. 
 */
FANN_EXTERNAL void FANN_API fann_destroy_train(struct fann_train_data *data)
{
	if(data == NULL)
		return;
	if(data->input != NULL)
		fann_safe_free(data->input[0]);
	if(data->output != NULL)
		fann_safe_free(data->output[0]);
	fann_safe_free(data->input);
	fann_safe_free(data->output);
	fann_safe_free(data);
}

/*
 * Test a set of training data and calculate the MSE 
 */
FANN_EXTERNAL float FANN_API fann_test_data(struct fann *ann, struct fann_train_data *data)
{
	unsigned int i;

	fann_reset_MSE(ann);

	for(i = 0; i != data->num_data; i++)
	{
		fann_test(ann, data->input[i], data->output[i]);
	}

	return fann_get_MSE(ann);
}

#ifndef FIXEDFANN

/*
 * Internal train function 
 */
float fann_train_epoch_quickprop(struct fann *ann, struct fann_train_data *data)
{
	unsigned int i;

	if(ann->prev_train_slopes == NULL)
	{
		fann_clear_train_arrays(ann);
	}

	fann_reset_MSE(ann);

	for(i = 0; i < data->num_data; i++)
	{
		fann_run(ann, data->input[i]);
		fann_compute_MSE(ann, data->output[i]);
		fann_backpropagate_MSE(ann);
		fann_update_slopes_batch(ann, ann->first_layer + 1, ann->last_layer - 1);
	}
	fann_update_weights_quickprop(ann, data->num_data, 0, ann->total_connections);

	return fann_get_MSE(ann);
}

/*
 * Internal train function 
 */
float fann_train_epoch_irpropm(struct fann *ann, struct fann_train_data *data)
{
	unsigned int i;

	if(ann->prev_train_slopes == NULL)
	{
		fann_clear_train_arrays(ann);
	}

	fann_reset_MSE(ann);

	for(i = 0; i < data->num_data; i++)
	{
		fann_run(ann, data->input[i]);
		fann_compute_MSE(ann, data->output[i]);
		fann_backpropagate_MSE(ann);
		fann_update_slopes_batch(ann, ann->first_layer + 1, ann->last_layer - 1);
	}

	fann_update_weights_irpropm(ann, 0, ann->total_connections);

	return fann_get_MSE(ann);
}

/*
 * Internal train function 
 */
float fann_train_epoch_batch(struct fann *ann, struct fann_train_data *data)
{
	unsigned int i;

	fann_reset_MSE(ann);

	for(i = 0; i < data->num_data; i++)
	{
		fann_run(ann, data->input[i]);
		fann_compute_MSE(ann, data->output[i]);
		fann_backpropagate_MSE(ann);
		fann_update_slopes_batch(ann, ann->first_layer + 1, ann->last_layer - 1);
	}

	fann_update_weights_batch(ann, data->num_data, 0, ann->total_connections);

	return fann_get_MSE(ann);
}

/*
 * Internal train function 
 */
float fann_train_epoch_incremental(struct fann *ann, struct fann_train_data *data)
{
	unsigned int i;

	fann_reset_MSE(ann);

	for(i = 0; i != data->num_data; i++)
	{
		fann_train(ann, data->input[i], data->output[i]);
	}

	return fann_get_MSE(ann);
}

/*
 * Train for one epoch with the selected training algorithm 
 */
FANN_EXTERNAL float FANN_API fann_train_epoch(struct fann *ann, struct fann_train_data *data)
{
	switch (ann->training_algorithm)
	{
	case FANN_TRAIN_QUICKPROP:
		return fann_train_epoch_quickprop(ann, data);
	case FANN_TRAIN_RPROP:
		return fann_train_epoch_irpropm(ann, data);
	case FANN_TRAIN_BATCH:
		return fann_train_epoch_batch(ann, data);
	case FANN_TRAIN_INCREMENTAL:
		return fann_train_epoch_incremental(ann, data);
	}
	return 0;
}

FANN_EXTERNAL void FANN_API fann_train_on_data(struct fann *ann, struct fann_train_data *data,
											   unsigned int max_epochs,
											   unsigned int epochs_between_reports,
											   float desired_error)
{
	float error;
	unsigned int i;
	int desired_error_reached;

#ifdef DEBUG
	printf("Training with %s\n", FANN_TRAIN_NAMES[ann->training_algorithm]);
#endif

	if(epochs_between_reports && ann->callback == NULL)
	{
		printf("Max epochs %8d. Desired error: %.10f.\n", max_epochs, desired_error);
	}

	for(i = 1; i <= max_epochs; i++)
	{
		/*
		 * train 
		 */
		error = fann_train_epoch(ann, data);
		desired_error_reached = fann_desired_error_reached(ann, desired_error);

		/*
		 * print current output 
		 */
		if(epochs_between_reports &&
		   (i % epochs_between_reports == 0 || i == max_epochs || i == 1 ||
			desired_error_reached == 0))
		{
			if(ann->callback == NULL)
			{
				printf("Epochs     %8d. Current error: %.10f. Bit fail %d.\n", i, error,
					   ann->num_bit_fail);
			}
			else if(((*ann->callback)(ann, data, max_epochs, epochs_between_reports, 
									  desired_error, i)) == -1)
			{
				/*
				 * you can break the training by returning -1 
				 */
				break;
			}
		}

		if(desired_error_reached == 0)
			break;
	}
}

FANN_EXTERNAL void FANN_API fann_train_on_file(struct fann *ann, const char *filename,
											   unsigned int max_epochs,
											   unsigned int epochs_between_reports,
											   float desired_error)
{
	struct fann_train_data *data = fann_read_train_from_file(filename);

	if(data == NULL)
	{
		return;
	}
	fann_train_on_data(ann, data, max_epochs, epochs_between_reports, desired_error);
	fann_destroy_train(data);
}

#endif

/*
 * shuffles training data, randomizing the order 
 */
FANN_EXTERNAL void FANN_API fann_shuffle_train_data(struct fann_train_data *train_data)
{
	unsigned int dat = 0, elem, swap;
	fann_type temp;

	for(; dat < train_data->num_data; dat++)
	{
		swap = (unsigned int) (rand() % train_data->num_data);
		if(swap != dat)
		{
			for(elem = 0; elem < train_data->num_input; elem++)
			{
				temp = train_data->input[dat][elem];
				train_data->input[dat][elem] = train_data->input[swap][elem];
				train_data->input[swap][elem] = temp;
			}
			for(elem = 0; elem < train_data->num_output; elem++)
			{
				temp = train_data->output[dat][elem];
				train_data->output[dat][elem] = train_data->output[swap][elem];
				train_data->output[swap][elem] = temp;
			}
		}
	}
}

/*
 * INTERNAL FUNCTION Scales data to a specific range 
 */
void fann_scale_data(fann_type ** data, unsigned int num_data, unsigned int num_elem,
					 fann_type new_min, fann_type new_max)
{
	unsigned int dat, elem;
	fann_type old_min, old_max, temp, old_span, new_span, factor;

	old_min = old_max = data[0][0];

	/*
	 * first calculate min and max 
	 */
	for(dat = 0; dat < num_data; dat++)
	{
		for(elem = 0; elem < num_elem; elem++)
		{
			temp = data[dat][elem];
			if(temp < old_min)
				old_min = temp;
			else if(temp > old_max)
				old_max = temp;
		}
	}

	old_span = old_max - old_min;
	new_span = new_max - new_min;
	factor = new_span / old_span;

	for(dat = 0; dat < num_data; dat++)
	{
		for(elem = 0; elem < num_elem; elem++)
		{
			temp = (data[dat][elem] - old_min) * factor + new_min;
			if(temp < new_min)
			{
				data[dat][elem] = new_min;
				/*
				 * printf("error %f < %f\n", temp, new_min); 
				 */
			}
			else if(temp > new_max)
			{
				data[dat][elem] = new_max;
				/*
				 * printf("error %f > %f\n", temp, new_max); 
				 */
			}
			else
			{
				data[dat][elem] = temp;
			}
		}
	}
}

/*
 * Scales the inputs in the training data to the specified range 
 */
FANN_EXTERNAL void FANN_API fann_scale_input_train_data(struct fann_train_data *train_data,
														fann_type new_min, fann_type new_max)
{
	fann_scale_data(train_data->input, train_data->num_data, train_data->num_input, new_min,
					new_max);
}

/*
 * Scales the inputs in the training data to the specified range 
 */
FANN_EXTERNAL void FANN_API fann_scale_output_train_data(struct fann_train_data *train_data,
														 fann_type new_min, fann_type new_max)
{
	fann_scale_data(train_data->output, train_data->num_data, train_data->num_output, new_min,
					new_max);
}

/*
 * Scales the inputs in the training data to the specified range 
 */
FANN_EXTERNAL void FANN_API fann_scale_train_data(struct fann_train_data *train_data,
												  fann_type new_min, fann_type new_max)
{
	fann_scale_data(train_data->input, train_data->num_data, train_data->num_input, new_min,
					new_max);
	fann_scale_data(train_data->output, train_data->num_data, train_data->num_output, new_min,
					new_max);
}

/*
 * merges training data into a single struct. 
 */
FANN_EXTERNAL struct fann_train_data *FANN_API fann_merge_train_data(struct fann_train_data *data1,
																	 struct fann_train_data *data2)
{
	unsigned int i;
	fann_type *data_input, *data_output;
	struct fann_train_data *dest =
		(struct fann_train_data *) malloc(sizeof(struct fann_train_data));

	if(dest == NULL)
	{
		fann_error((struct fann_error*)data1, FANN_E_CANT_ALLOCATE_MEM);
		return NULL;
	}

	if((data1->num_input != data2->num_input) || (data1->num_output != data2->num_output))
	{
		fann_error((struct fann_error*)data1, FANN_E_TRAIN_DATA_MISMATCH);
		return NULL;
	}

	fann_init_error_data((struct fann_error *) dest);
	dest->error_log = data1->error_log;

	dest->num_data = data1->num_data+data2->num_data;
	dest->num_input = data1->num_input;
	dest->num_output = data1->num_output;
	dest->input = (fann_type **) calloc(dest->num_data, sizeof(fann_type *));
	if(dest->input == NULL)
	{
		fann_error((struct fann_error*)data1, FANN_E_CANT_ALLOCATE_MEM);
		fann_destroy_train(dest);
		return NULL;
	}

	dest->output = (fann_type **) calloc(dest->num_data, sizeof(fann_type *));
	if(dest->output == NULL)
	{
		fann_error((struct fann_error*)data1, FANN_E_CANT_ALLOCATE_MEM);
		fann_destroy_train(dest);
		return NULL;
	}

	data_input = (fann_type *) calloc(dest->num_input * dest->num_data, sizeof(fann_type));
	if(data_input == NULL)
	{
		fann_error((struct fann_error*)data1, FANN_E_CANT_ALLOCATE_MEM);
		fann_destroy_train(dest);
		return NULL;
	}
	memcpy(data_input, data1->input[0], dest->num_input * data1->num_data * sizeof(fann_type));
	memcpy(data_input + (dest->num_input*data1->num_data), 
		data2->input[0], dest->num_input * data2->num_data * sizeof(fann_type));

	data_output = (fann_type *) calloc(dest->num_output * dest->num_data, sizeof(fann_type));
	if(data_output == NULL)
	{
		fann_error((struct fann_error*)data1, FANN_E_CANT_ALLOCATE_MEM);
		fann_destroy_train(dest);
		return NULL;
	}
	memcpy(data_output, data1->output[0], dest->num_output * data1->num_data * sizeof(fann_type));
	memcpy(data_output + (dest->num_output*data1->num_data), 
		data2->output[0], dest->num_output * data2->num_data * sizeof(fann_type));

	for(i = 0; i != dest->num_data; i++)
	{
		dest->input[i] = data_input;
		data_input += dest->num_input;
		dest->output[i] = data_output;
		data_output += dest->num_output;
	}
	return dest;
}

/*
 * return a copy of a fann_train_data struct 
 */
FANN_EXTERNAL struct fann_train_data *FANN_API fann_duplicate_train_data(struct fann_train_data
																		 *data)
{
	unsigned int i;
	fann_type *data_input, *data_output;
	struct fann_train_data *dest =
		(struct fann_train_data *) malloc(sizeof(struct fann_train_data));

	if(dest == NULL)
	{
		fann_error((struct fann_error*)data, FANN_E_CANT_ALLOCATE_MEM);
		return NULL;
	}

	fann_init_error_data((struct fann_error *) dest);
	dest->error_log = data->error_log;

	dest->num_data = data->num_data;
	dest->num_input = data->num_input;
	dest->num_output = data->num_output;
	dest->input = (fann_type **) calloc(dest->num_data, sizeof(fann_type *));
	if(dest->input == NULL)
	{
		fann_error((struct fann_error*)data, FANN_E_CANT_ALLOCATE_MEM);
		fann_destroy_train(dest);
		return NULL;
	}

	dest->output = (fann_type **) calloc(dest->num_data, sizeof(fann_type *));
	if(dest->output == NULL)
	{
		fann_error((struct fann_error*)data, FANN_E_CANT_ALLOCATE_MEM);
		fann_destroy_train(dest);
		return NULL;
	}

	data_input = (fann_type *) calloc(dest->num_input * dest->num_data, sizeof(fann_type));
	if(data_input == NULL)
	{
		fann_error((struct fann_error*)data, FANN_E_CANT_ALLOCATE_MEM);
		fann_destroy_train(dest);
		return NULL;
	}
	memcpy(data_input, data->input[0], dest->num_input * dest->num_data * sizeof(fann_type));

	data_output = (fann_type *) calloc(dest->num_output * dest->num_data, sizeof(fann_type));
	if(data_output == NULL)
	{
		fann_error((struct fann_error*)data, FANN_E_CANT_ALLOCATE_MEM);
		fann_destroy_train(dest);
		return NULL;
	}
	memcpy(data_output, data->output[0], dest->num_output * dest->num_data * sizeof(fann_type));

	for(i = 0; i != dest->num_data; i++)
	{
		dest->input[i] = data_input;
		data_input += dest->num_input;
		dest->output[i] = data_output;
		data_output += dest->num_output;
	}
	return dest;
}

FANN_EXTERNAL struct fann_train_data *FANN_API fann_subset_train_data(struct fann_train_data
																		 *data, unsigned int pos,
																		 unsigned int length)
{
	unsigned int i;
	fann_type *data_input, *data_output;
	struct fann_train_data *dest =
		(struct fann_train_data *) malloc(sizeof(struct fann_train_data));

	if(dest == NULL)
	{
		fann_error((struct fann_error*)data, FANN_E_CANT_ALLOCATE_MEM);
		return NULL;
	}
	
	if(pos > data->num_data || pos+length > data->num_data)
	{
		fann_error((struct fann_error*)data, FANN_E_TRAIN_DATA_SUBSET, pos, length, data->num_data);
		return NULL;
	}

	fann_init_error_data((struct fann_error *) dest);
	dest->error_log = data->error_log;

	dest->num_data = length;
	dest->num_input = data->num_input;
	dest->num_output = data->num_output;
	dest->input = (fann_type **) calloc(dest->num_data, sizeof(fann_type *));
	if(dest->input == NULL)
	{
		fann_error((struct fann_error*)data, FANN_E_CANT_ALLOCATE_MEM);
		fann_destroy_train(dest);
		return NULL;
	}

	dest->output = (fann_type **) calloc(dest->num_data, sizeof(fann_type *));
	if(dest->output == NULL)
	{
		fann_error((struct fann_error*)data, FANN_E_CANT_ALLOCATE_MEM);
		fann_destroy_train(dest);
		return NULL;
	}

	data_input = (fann_type *) calloc(dest->num_input * dest->num_data, sizeof(fann_type));
	if(data_input == NULL)
	{
		fann_error((struct fann_error*)data, FANN_E_CANT_ALLOCATE_MEM);
		fann_destroy_train(dest);
		return NULL;
	}
	memcpy(data_input, data->input[pos], dest->num_input * dest->num_data * sizeof(fann_type));

	data_output = (fann_type *) calloc(dest->num_output * dest->num_data, sizeof(fann_type));
	if(data_output == NULL)
	{
		fann_error((struct fann_error*)data, FANN_E_CANT_ALLOCATE_MEM);
		fann_destroy_train(dest);
		return NULL;
	}
	memcpy(data_output, data->output[pos], dest->num_output * dest->num_data * sizeof(fann_type));

	for(i = 0; i != dest->num_data; i++)
	{
		dest->input[i] = data_input;
		data_input += dest->num_input;
		dest->output[i] = data_output;
		data_output += dest->num_output;
	}
	return dest;
}

FANN_EXTERNAL unsigned int FANN_API fann_length_train_data(struct fann_train_data *data)
{
	return data->num_data;
}

FANN_EXTERNAL unsigned int FANN_API fann_num_input_train_data(struct fann_train_data *data)
{
	return data->num_input;
}

FANN_EXTERNAL unsigned int FANN_API fann_num_output_train_data(struct fann_train_data *data)
{
	return data->num_output;
}

/* INTERNAL FUNCTION
   Save the train data structure.
 */
int fann_save_train_internal(struct fann_train_data *data, const char *filename,
							  unsigned int save_as_fixed, unsigned int decimal_point)
{
	int retval = 0;
	FILE *file = fopen(filename, "w");

	if(!file)
	{
		fann_error((struct fann_error *) data, FANN_E_CANT_OPEN_TD_W, filename);
		return -1;
	}
	retval = fann_save_train_internal_fd(data, file, filename, save_as_fixed, decimal_point);
	fclose(file);
	
	return retval;
}

/* INTERNAL FUNCTION
   Save the train data structure.
 */
int fann_save_train_internal_fd(struct fann_train_data *data, FILE * file, const char *filename,
								 unsigned int save_as_fixed, unsigned int decimal_point)
{
	unsigned int num_data = data->num_data;
	unsigned int num_input = data->num_input;
	unsigned int num_output = data->num_output;
	unsigned int i, j;
	int retval = 0;

#ifndef FIXEDFANN
	unsigned int multiplier = 1 << decimal_point;
#endif

	fprintf(file, "%u %u %u\n", data->num_data, data->num_input, data->num_output);

	for(i = 0; i < num_data; i++)
	{
		for(j = 0; j < num_input; j++)
		{
#ifndef FIXEDFANN
			if(save_as_fixed)
			{
				fprintf(file, "%d ", (int) (data->input[i][j] * multiplier));
			}
			else
			{
				if(((int) floor(data->input[i][j] + 0.5) * 1000000) ==
				   ((int) floor(data->input[i][j] * 1000000.0 + 0.5)))
				{
					fprintf(file, "%d ", (int) data->input[i][j]);
				}
				else
				{
					fprintf(file, "%f ", data->input[i][j]);
				}
			}
#else
			fprintf(file, FANNPRINTF " ", data->input[i][j]);
#endif
		}
		fprintf(file, "\n");

		for(j = 0; j < num_output; j++)
		{
#ifndef FIXEDFANN
			if(save_as_fixed)
			{
				fprintf(file, "%d ", (int) (data->output[i][j] * multiplier));
			}
			else
			{
				if(((int) floor(data->output[i][j] + 0.5) * 1000000) ==
				   ((int) floor(data->output[i][j] * 1000000.0 + 0.5)))
				{
					fprintf(file, "%d ", (int) data->output[i][j]);
				}
				else
				{
					fprintf(file, "%f ", data->output[i][j]);
				}
			}
#else
			fprintf(file, FANNPRINTF " ", data->output[i][j]);
#endif
		}
		fprintf(file, "\n");
	}
	
	return retval;
}


/*
 * INTERNAL FUNCTION Reads training data from a file descriptor. 
 */
struct fann_train_data *fann_read_train_from_fd(FILE * file, const char *filename)
{
	unsigned int num_input, num_output, num_data, i, j;
	unsigned int line = 1;
	fann_type *data_input, *data_output;
	struct fann_train_data *data =
		(struct fann_train_data *) malloc(sizeof(struct fann_train_data));

	if(data == NULL)
	{
		fann_error(NULL, FANN_E_CANT_ALLOCATE_MEM);
		return NULL;
	}

	if(fscanf(file, "%u %u %u\n", &num_data, &num_input, &num_output) != 3)
	{
		fann_error(NULL, FANN_E_CANT_READ_TD, filename, line);
		fann_destroy_train(data);
		return NULL;
	}
	line++;

	fann_init_error_data((struct fann_error *) data);

	data->num_data = num_data;
	data->num_input = num_input;
	data->num_output = num_output;
	data->input = (fann_type **) calloc(num_data, sizeof(fann_type *));
	if(data->input == NULL)
	{
		fann_error(NULL, FANN_E_CANT_ALLOCATE_MEM);
		fann_destroy_train(data);
		return NULL;
	}

	data->output = (fann_type **) calloc(num_data, sizeof(fann_type *));
	if(data->output == NULL)
	{
		fann_error(NULL, FANN_E_CANT_ALLOCATE_MEM);
		fann_destroy_train(data);
		return NULL;
	}

	data_input = (fann_type *) calloc(num_input * num_data, sizeof(fann_type));
	if(data_input == NULL)
	{
		fann_error(NULL, FANN_E_CANT_ALLOCATE_MEM);
		fann_destroy_train(data);
		return NULL;
	}

	data_output = (fann_type *) calloc(num_output * num_data, sizeof(fann_type));
	if(data_output == NULL)
	{
		fann_error(NULL, FANN_E_CANT_ALLOCATE_MEM);
		fann_destroy_train(data);
		return NULL;
	}

	for(i = 0; i != num_data; i++)
	{
		data->input[i] = data_input;
		data_input += num_input;

		for(j = 0; j != num_input; j++)
		{
			if(fscanf(file, FANNSCANF " ", &data->input[i][j]) != 1)
			{
				fann_error(NULL, FANN_E_CANT_READ_TD, filename, line);
				fann_destroy_train(data);
				return NULL;
			}
		}
		line++;

		data->output[i] = data_output;
		data_output += num_output;

		for(j = 0; j != num_output; j++)
		{
			if(fscanf(file, FANNSCANF " ", &data->output[i][j]) != 1)
			{
				fann_error(NULL, FANN_E_CANT_READ_TD, filename, line);
				fann_destroy_train(data);
				return NULL;
			}
		}
		line++;
	}
	return data;
}

/*
 * INTERNAL FUNCTION returns 0 if the desired error is reached and -1 if it is not reached
 */
int fann_desired_error_reached(struct fann *ann, float desired_error)
{
	switch (ann->train_stop_function)
	{
	case FANN_STOPFUNC_MSE:
		if(fann_get_MSE(ann) <= desired_error)
			return 0;
		break;
	case FANN_STOPFUNC_BIT:
		if(ann->num_bit_fail <= (unsigned int)desired_error)
			return 0;
		break;
	}
	return -1;
}


struct fann_train_data *fann_create_train(unsigned int num_data, unsigned int num_input, unsigned int num_output)
{
	struct fann_train_data *data;
	unsigned int i;
	fann_type *data_input, *data_output;


	data = (struct fann_train_data *) calloc(1, sizeof(struct fann_train_data));
	if(data == NULL)
	{
		fann_error(NULL, FANN_E_CANT_ALLOCATE_MEM);
		return NULL;
	}

	// set the error information
	fann_init_error_data((struct fann_error *) data);

	// set the size variables
	data->num_data = num_data;
	data->num_input = num_input;
	data->num_output = num_output;

	// allocate memory for pointers into input array
	data->input = (fann_type **) calloc(num_data, sizeof(fann_type *));
	if(data->input == NULL)
	{
		fann_error(NULL, FANN_E_CANT_ALLOCATE_MEM);
		fann_destroy_train(data);
		return NULL;
	}

	//allocate memory for pointers into output array
	data->output = (fann_type **) calloc(num_data, sizeof(fann_type *));
	if(data->output == NULL)
	{
		fann_error(NULL, FANN_E_CANT_ALLOCATE_MEM);
		fann_destroy_train(data);
		return NULL;
	}

	//allocate actual input data
	data_input = (fann_type *) calloc(num_input * num_data, sizeof(fann_type));
	if(data_input == NULL)
	{
		fann_error(NULL, FANN_E_CANT_ALLOCATE_MEM);
		fann_destroy_train(data);
		return NULL;
	}

	//allocate actual output data
	data_output = (fann_type *) calloc(num_output * num_data, sizeof(fann_type));
	if(data_output == NULL)
	{
		fann_error(NULL, FANN_E_CANT_ALLOCATE_MEM);
		fann_destroy_train(data);
		return NULL;
	}

	//set all the pointers
	for(i = 0; i < num_data; i++)
	{
		data->input[i] = data_input;
		data_input += num_input;

		data->output[i] = data_output;
		data_output += num_output;
	}
	return data;
}


int fann_set_train_data(struct fann_train_data* data, unsigned int num, fann_type* input, fann_type* output)
{
	if(num>=data->num_data)
	{
		fann_error(NULL, FANN_E_INDEX_OUT_OF_BOUND);
		return -1;
	}

	memcpy( data->input[num], input, data->num_input * sizeof( fann_type ) );
	memcpy( data->output[num], output, data->num_output * sizeof( fann_type ) );

	return 0;
}


int fann_get_train_data(struct fann_train_data* data, unsigned int num, fann_type* input, fann_type* output)
{
	if(num>=data->num_data)
	{
		fann_error(NULL, FANN_E_INDEX_OUT_OF_BOUND);
		return -1;
	}

	memcpy( input, data->input[num], data->num_input * sizeof( fann_type ) );
	memcpy( output, data->output[num], data->num_output * sizeof( fann_type ) );

	return 0;
}
