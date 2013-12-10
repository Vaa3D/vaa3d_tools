/* testgeo3d.c
 *
 * 22-Jan-2008 Initial write: Ting Zhao
 */

#include <stdio.h>
#include <math.h>
#include "tz_error.h"
#include "tz_iarray.h"
#include "tz_stack_neighborhood.h"
#include "tz_stack_lib.h"
#include "tz_stack_attribute.h"
#include "tz_math.h"
#include "tz_utilities.h"
#include "tz_testdata.h"
#include "tz_darray.h"
#include "tz_stack_io.h"

INIT_EXCEPTION_MAIN(e)

int in_bound(int x, int min, int max)
{
  return (x >= min) && (x <= max);
}

#define IN_BOUND_2D(x, y)					\
  (in_bound(y, 0, cheight) && in_bound(x, 0, cwidth))

static inline int *boundary_pixels_2d(int n_nbr, int x, int y, 
				      int cwidth, int cheight,
				      int bound[])
{ 
  bound[0] = IN_BOUND_2D(x - 1, y); 
  bound[1] = IN_BOUND_2D(x + 1, y);
  bound[2] = IN_BOUND_2D(x, y - 1);
  bound[3] = IN_BOUND_2D(x, y + 1);
  bound[4] = IN_BOUND_2D(x - 1, y - 1);
  bound[5] = IN_BOUND_2D(x + 1, y + 1);
  bound[6] = IN_BOUND_2D(x + 1, y - 1);
  bound[7] = IN_BOUND_2D(x - 1, y + 1);

  return (bound);
}


#define IN_BOUND(x, y, z)					\
  (in_bound(z, 0, cdepth) && in_bound(y, 0, cheight) && in_bound(x, 0, cwidth))

static inline int *boundary_pixels_3d(int x, int y, int z,
				      int cwidth, int cheight, int cdepth,
				      int bound[])
{  
  bound[0] = IN_BOUND(x - 1, y, z); 
  bound[1] = IN_BOUND(x + 1, y, z);
  bound[2] = IN_BOUND(x, y - 1, z);
  bound[3] = IN_BOUND(x, y + 1, z);
  bound[6] = IN_BOUND(x - 1, y - 1, z);
  bound[7] = IN_BOUND(x + 1, y + 1, z);
  bound[8] = IN_BOUND(x + 1, y - 1, z);
  bound[9] = IN_BOUND(x - 1, y + 1, z);

  bound[4] = IN_BOUND(x, y, z - 1);
  bound[5] = IN_BOUND(x, y, z + 1);

  bound[10] = IN_BOUND(x - 1, y, z - 1); 
  bound[12] = IN_BOUND(x + 1, y, z - 1);
  bound[14] = IN_BOUND(x, y - 1, z - 1);
  bound[16] = IN_BOUND(x, y + 1, z - 1);
  bound[18] = IN_BOUND(x - 1, y - 1, z - 1);
  bound[20] = IN_BOUND(x + 1, y + 1, z - 1);
  bound[22] = IN_BOUND(x + 1, y - 1, z - 1);
  bound[24] = IN_BOUND(x - 1, y + 1, z - 1);

  bound[13] = IN_BOUND(x - 1, y, z + 1); 
  bound[11] = IN_BOUND(x + 1, y, z + 1);
  bound[17] = IN_BOUND(x, y - 1, z + 1);
  bound[15] = IN_BOUND(x, y + 1, z + 1);
  bound[21] = IN_BOUND(x - 1, y - 1, z + 1);
  bound[19] = IN_BOUND(x + 1, y + 1, z + 1);
  bound[25] = IN_BOUND(x + 1, y - 1, z + 1);
  bound[23] = IN_BOUND(x - 1, y + 1, z + 1);

  return (bound);
}

int main(int argc, char *argv[])
{
#if 1
  static char *Spec[] = {"[-t]", NULL};

  Process_Arguments(argc, argv, Spec, 1);
 
  if (Is_Arg_Matched("-t")) {
    /* Example test */
    int neighbor[26];

    /* Offsets of the neighbors */
    
    /* 4-connected neighbor offset for 5x7 image */
    Stack_Neighbor_Offset(4, 5, 7, neighbor);
    
    /* The distance of each neighbor. */
    const double *dist = Stack_Neighbor_Dist(4);

    if ((neighbor[0] != -1) || (neighbor[1] != 1) || (neighbor[2] != -5) ||
	(neighbor[3] != 5)) {
      PRINT_EXCEPTION("Bug?", "Unexpected neighbor offsets.");
      return 1;
    }

    int i;
    for (i = 0; i < 4; i++) {
      if (dist[i] != 1.0) {
	PRINT_EXCEPTION("Bug?", "unexpected distance.");
	return 1;
      }
    }

    /* 8-connected */
    Stack_Neighbor_Offset(8, 5, 7, neighbor);

    dist  = Stack_Neighbor_Dist(8);

    if ((neighbor[0] != -1) || (neighbor[1] != 1) || (neighbor[2] != -5) ||
	(neighbor[3] != 5) || (neighbor[4] != -6) || (neighbor[5] != 6) ||
	(neighbor[6] != -4) || (neighbor[7] != 4)) {
      PRINT_EXCEPTION("Bug?", "Unexpected neighbor offsets.");
      return 1;
    }

    for (i = 0; i< 4; i++) {
      if (dist[i] != 1.0) {
	PRINT_EXCEPTION("Bug?", "Unexpected distance.");
	return 1;
      }
    }

    for (i = 4; i < 8; i++) { 
      if(Compare_Float(dist[i], sqrt(2.0), 1e-5) != 0) {
	PRINT_EXCEPTION("Bug?", "Unexpected neighbor offsets.");
	return 1;
      }
    }

    /* 6-connected */
    Stack_Neighbor_Offset(6, 5, 7, neighbor);

    dist = Stack_Neighbor_Dist(6);

    if ((neighbor[0] != -1) || (neighbor[1] != 1) || (neighbor[2] != -5) ||
	(neighbor[3] != 5) || (neighbor[4] != -35) || (neighbor[5] != 35)) {
      PRINT_EXCEPTION("Bug?", "Unexpected neighbor offsets.");
      return 1;
    }

    for (i = 0; i< 6; i++) {
      if (dist[i] != 1.0) {
	PRINT_EXCEPTION("Bug?", "Unexpected distance.");
	return 1;
      }
    }
    
    /* 18-connected */
    Stack_Neighbor_Offset(18, 5, 7, neighbor);

    dist = Stack_Neighbor_Dist(18);

    if ((neighbor[0] != -1) || (neighbor[1] != 1) || (neighbor[2] != -5) ||
	(neighbor[3] != 5) || (neighbor[4] != -35) || (neighbor[5] != 35) ||
	(neighbor[6] != -6) || (neighbor[7] != 6) || (neighbor[8] != -4) ||
	(neighbor[9] != 4) || (neighbor[10] != -36) || (neighbor[11] != 36) ||
	(neighbor[12] != -34) || (neighbor[13] != 34) || 
	(neighbor[14] != -40) || (neighbor[15] != 40) || 
        (neighbor[16] != -30) || (neighbor[17] != 30)) {
      PRINT_EXCEPTION("Bug?", "Unexpected neighbor offsets.");
      return 1;
    }

    for (i = 0; i< 6; i++) {
      if (dist[i] != 1.0) {
	PRINT_EXCEPTION("Bug?", "Unexpected distance.");
	return 1;
      }
    }

    for (i = 6; i < 18; i++) { 
      if(Compare_Float(dist[i], sqrt(2.0), 1e-5) != 0) {
	PRINT_EXCEPTION("Bug?", "Unexpected neighbor offsets.");
	return 1;
      }
    }

    /* 26-connected */
    Stack_Neighbor_Offset(26, 5, 7, neighbor);

    dist = Stack_Neighbor_Dist(26);

    if ((neighbor[0] != -1) || (neighbor[1] != 1) || (neighbor[2] != -5) ||
	(neighbor[3] != 5) || (neighbor[4] != -35) || (neighbor[5] != 35) ||
	(neighbor[6] != -6) || (neighbor[7] != 6) || (neighbor[8] != -4) ||
	(neighbor[9] != 4) || (neighbor[10] != -36) || (neighbor[11] != 36) ||
	(neighbor[12] != -34) || (neighbor[13] != 34) || 
	(neighbor[14] != -40) || (neighbor[15] != 40) || 
        (neighbor[16] != -30) || (neighbor[17] != 30) ||
	(neighbor[18] != -41) || (neighbor[19] != 41) ||
	(neighbor[20] != -29) || (neighbor[21] != 29) ||
	(neighbor[22] != -39) || (neighbor[23] != 39) ||
	(neighbor[24] != -31) || (neighbor[25] != 31)) {
      PRINT_EXCEPTION("Bug?", "Unexpected neighbor offsets.");
      return 1;
    }

    for (i = 0; i< 6; i++) {
      if (dist[i] != 1.0) {
	PRINT_EXCEPTION("Bug?", "Unexpected distance.");
	return 1;
      }
    }

    for (i = 6; i < 18; i++) { 
      if(Compare_Float(dist[i], sqrt(2.0), 1e-5) != 0) {
	PRINT_EXCEPTION("Bug?", "Unexpected neighbor offsets.");
	return 1;
      }
    }

    for (i = 18; i < 25; i++) { 
      if(Compare_Float(dist[i], sqrt(3.0), 1e-5) != 0) {
	printf("%g\n", dist[i]);
	PRINT_EXCEPTION("Bug?", "Unexpected neighbor offsets.");
	return 1;
      }
    }

    /* Bound test */
    int is_in_bound[26];

    int n = Stack_Neighbor_Bound_Test(8, 2, 2, 2, 0, 0, 0, is_in_bound); 

    if (n != 3) {
      PRINT_EXCEPTION("Bug?", "Unexpected in-bound value.");
      return 1;
    }

    int tmp_bound[26];
    boundary_pixels_2d(8, 0, 0, 2, 2, tmp_bound);
    
    for (i = 0; i < 8; i++) {
      if (is_in_bound[i] != tmp_bound[i]) {
	PRINT_EXCEPTION("Bug?", "Unmatched bound test");
	return 1;
      }
    }

    n = Stack_Neighbor_Bound_Test(26, 2, 2, 2, 0, 0, 0, is_in_bound); 

    if (n != 7) {
      PRINT_EXCEPTION("Bug?", "Unexpected in-bound value.");
      return 1;
    }

    boundary_pixels_3d(0, 0, 0, 2, 2, 2, tmp_bound);
    
    for (i = 0; i < 26; i++) {
      if (is_in_bound[i] != tmp_bound[i]) {
	PRINT_EXCEPTION("Bug?", "Unmatched bound test");
	return 1;
      }
    }

    n = Stack_Neighbor_Bound_Test(26, 2, 2, 2, 1, 1, 1, is_in_bound); 

    if (n != 26) {
      PRINT_EXCEPTION("Bug?", "Unexpected in-bound value.");
      return 1;
    }
    
    n = Stack_Neighbor_Bound_Test(26, 2, 2, 2, -2, -2, -2, is_in_bound); 

    if (n != 0) {
      printf("%d\n", n);
      PRINT_EXCEPTION("Bug?", "Unexpected in-bound value.");
      return 1;
    }
    
    /* Sample neighbors */
    Stack *stack = Index_Stack(GREY, 3, 3, 3);

    double s[26];
    Stack_Neighbor_Sampling(stack, 26, 1, 1, 1, -1, s);

    double ground_truth[26] = {
      12.0, 14.0, 10.0, 16.0, 4.0, 22.0, 9.0, 17.0, 11.0,
      15.0, 3.0, 23.0, 5.0, 21.0, 1.0, 25.0, 7.0,
      19.0, 0.0, 26.0, 8.0, 18.0, 2.0, 24.0, 6.0, 20.0 
    };

    for (i = 0; i < 26; i++) {
      if (s[i] != ground_truth[i]) {
	PRINT_EXCEPTION("Bug?", "Wrong sampling value.");
	return 1;
      }
    }

    Stack_Neighbor_Sampling_I(stack, 26, 13, -1, s);

    for (i = 0; i < 26; i++) {
      if (s[i] != ground_truth[i]) {
	PRINT_EXCEPTION("Bug?", "Wrong sampling value.");
	return 1;
      }
    }

    double v = Stack_Neighbor_Min(stack, 26, 0, 0, 0);
    if (v != 0) {
      PRINT_EXCEPTION("Bug?", "Wrong sampling value.");
      return 1;
    }

    v = Stack_Neighbor_Mean(stack, 26, 0, 0, 0);
    if (Compare_Float(v, 6.5, 1e-5) != 0) {
      printf("%g\n", v);
      PRINT_EXCEPTION("Bug?", "Wrong sampling value.");
      return 1;
    }
    printf(":) Testing passed.\n");
  }
#endif

#if 0
  int is_in_bound[26];
  int is_in_bound2[26];
  int n_nbr = 4;
  int cwidth = 10;
  int cheight = 10;
  int cdepth = 10;
  int x = 0;
  int y = 0;
  int z = 0;
  int n_in_bound;
  int i;

  Stack *stack = Read_Stack("../data/fly_neuron.tif");
  double s[26];
  Stack_Neighbor_Sampling(stack, n_nbr, x, y, z, -1, s);
  //Print_Neighborhood(s, n_nbr);

  
  for (z = -2; z <= cdepth + 2; z++) {
    for (y = -2; y <= cheight + 2; y++) {
      for (x = -2; x <= cwidth + 2; x++) {
	n_in_bound = Stack_Neighbor_Bound_Test_S(n_nbr, cwidth, cheight, cdepth,
					       x, y, z, is_in_bound);
	if (n_in_bound == 0) {
	  for (i = 0; i < n_nbr; i++) {
	    is_in_bound[i] = 0;
	  }
	}
	if (n_in_bound == n_nbr) {
	  for (i = 0; i < n_nbr; i++) {
	    is_in_bound[i] = 1;
	  }
	}
	
	int n_in_bound2 = 0;
	for (i = 0; i< n_nbr; i++) {
	  n_in_bound2 += is_in_bound[i];
	}

	if (n_in_bound != n_in_bound2) {
	  printf("%d, %d, %d \n", x, y, z);
	  printf("%d should be %d\n", n_in_bound, n_in_bound2);
	  return 1;
	}
	boundary_pixels_2d(n_nbr, x, y, cwidth, cheight, is_in_bound2);
	//	boundary_pixels_3d(x, y, z, cwidth, cheight, cdepth, is_in_bound2);
	for (i = 0; i < n_nbr; i++) {
	  if (is_in_bound[i] != is_in_bound2[i]) {
	    printf("%d, %d, %d \n", x, y, z);
	    Print_Neighborhood(is_in_bound, n_nbr);
	    Print_Neighborhood(is_in_bound2, n_nbr);
	  }
	}
      }
    }
  }
  
#endif

#if 0
  Stack *stack = Make_Stack(1, 3, 3, 3);
  uint8 array[27] = {18, 14, 22, 10, 4, 12, 24, 16, 20, 6, 2, 8, 0, 100, 1,
		     9, 3, 7, 21, 17, 25, 13, 5, 11, 23, 15, 19}; 
  stack->array = array;
  
  double s[26];
  Stack_Neighbor_Sampling(stack, 26, 1, 1, 1, 255, s);
  Print_Neighborhood_F(s, 26);


  const int *x_offset = Stack_Neighbor_X_Offset(26);
  const int *y_offset = Stack_Neighbor_Y_Offset(26);
  const int *z_offset = Stack_Neighbor_Z_Offset(26);

  int i;
  int x, y, z;
  x = y = z = 1;
  for (i = 0; i < 26; i++) {
    s[i] = Get_Stack_Pixel(stack, x + x_offset[i], y + y_offset[i], 
			   z + z_offset[i], 0);
  }

  Print_Neighborhood_F(s, 26);
#endif

#if 0
  Stack *stack = Read_Stack("../data/mouse_single_org.tif");
  
  double s[26];
  Stack_Neighbor_Sampling_I(stack, 26, stack->height * stack->width + stack->height + 1, 255, s);
  Print_Neighborhood_F(s, 26);


  const int *x_offset = Stack_Neighbor_X_Offset(26);
  const int *y_offset = Stack_Neighbor_Y_Offset(26);
  const int *z_offset = Stack_Neighbor_Z_Offset(26);

  int i;
  int x, y, z;
  x = y = z = 1;
  for (i = 0; i < 26; i++) {
    s[i] = Get_Stack_Pixel(stack, x + x_offset[i], y + y_offset[i], 
			   z + z_offset[i], 0);
  }

  Print_Neighborhood_F(s, 26);
#endif

#if 0
  Stack *stack = Make_Stack(GREY, 5, 5, 3);
  int i;
  int nvoxel = Stack_Voxel_Number(stack);
  for (i = 0; i < nvoxel; i++) {
    stack->array[i] = i;
  }
  Print_Stack_Value(stack);

  Stack *stack2 = Make_Stack(GREY, 5, 5, 3);
  int x, y, z;
  i = 0;
  for (z = 0; z < stack->depth; z++) {
    for (y = 0; y < stack->height; y++) {
      for (x = 0; x < stack->width; x++) {
	stack2->array[i] = iround(Stack_Neighbor_Mean(stack, 26, x, y, z));
	i++;
      }
    }
  }
  Print_Stack_Value(stack2);
#endif

#if 0
  int is_in_bound[26];
  int n_nbr = 26;
  int cwidth = 0;
  int cheight = 0;
  int cdepth = 0;
  int x = 0;
  int y = 0;
  int z = 0;
  int n_in_bound;
  int i;

  const int *x_nbr = Stack_Neighbor_X_Offset(n_nbr); 
  const int *y_nbr = Stack_Neighbor_Y_Offset(n_nbr); 
  const int *z_nbr = Stack_Neighbor_Z_Offset(n_nbr); 

  for (z = 0; z <= cdepth; z++) {
    for (y = 0; y <= cheight; y++) {
      for (x = 0; x <= cwidth; x++) {
	n_in_bound = Stack_Neighbor_Bound_Test_S(n_nbr, cwidth, cheight, cdepth,
					       x, y, z, is_in_bound);
	if (n_in_bound == 0) {
	  for (i = 0; i < n_nbr; i++) {
	    is_in_bound[i] = 0;
	  }
	}
	if (n_in_bound == n_nbr) {
	  for (i = 0; i < n_nbr; i++) {
	    is_in_bound[i] = 1;
	  }
	}
	
	for (i = 0; i < n_nbr; i++) {
	  if (is_in_bound[i] != IS_IN_CLOSE_RANGE3(x + x_nbr[i], 
						   y + y_nbr[i], 
						   z + z_nbr[i], 0, cwidth,
						   0, cheight, 0, cdepth)) {
	    printf("%d, %d, %d; %d \n", x, y, z, i);
	    Print_Neighborhood(is_in_bound, n_nbr);
	    return 1;
	  }
	}
      }
    }
  }
#endif

#if 0
  Stack *stack = Make_Stack(GREY, 3, 3, 3);
  Zero_Stack(stack);
  Stack_Label_Neighbor_26(stack, 1048575);
  Print_Stack_Value(stack);
#endif

#if 1
  int neighbor_offset[26];
  Stack_Neighbor_Offset(26, 3, 3, neighbor_offset);

  Stack *stack = Make_Stack(GREY, 3, 3, 3);
  Zero_Stack(stack);
  int i;
  for (i = 0; i < 26; ++i) {
    stack->array[13 + neighbor_offset[i]] = i + 1;
  }

  Print_Stack(stack);
#endif

  return 0;
}
