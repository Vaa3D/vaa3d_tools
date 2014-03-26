/* tz_neighborhood.c
 *
 * 15-Jan-2008 Initial write: Ting Zhao
 */

#include <math.h>
#include "tz_error.h"
#include "tz_constant.h"
#include "tz_image_lib_defs.h"
#include "tz_stack_utils.h"
#include "tz_stack_neighborhood.h"

INIT_EXCEPTION


void Print_Neighborhood(int neighbor[], int n_nbr)
{
  switch (n_nbr) {
  case 4:
    printf(" |%d| \n", neighbor[2]);
    printf("%d| |%d\n", neighbor[0], neighbor[1]);
    printf(" |%d| \n", neighbor[3]);
    break;
  case 8:
    printf("%d|%d|%d\n", neighbor[4], neighbor[2], neighbor[6]);
    printf("%d| |%d\n", neighbor[0], neighbor[1]);
    printf("%d|%d|%d \n", neighbor[7], neighbor[3], neighbor[5]);
    break;
  case 6:
    printf(" |%d| \n", neighbor[4]);
    printf(" |-| \n");
    printf(" |%d| \n", neighbor[2]);
    printf("%d| |%d\n", neighbor[0], neighbor[1]);
    printf(" |%d| \n", neighbor[3]);
    printf(" |-| \n");
    printf(" |%d| \n", neighbor[5]);
    break;
  case 18:
    printf(" |%d| \n", neighbor[14]);
    printf("%d|%d|%d\n", neighbor[10], neighbor[4], neighbor[12]);
    printf(" |%d| \n", neighbor[16]);
    printf(" |-| \n");
    printf("%d|%d|%d\n", neighbor[6], neighbor[2], neighbor[8]);
    printf("%d| |%d\n", neighbor[0], neighbor[1]);
    printf("%d|%d|%d\n", neighbor[9], neighbor[3], neighbor[7]);
    printf(" |-| \n");
    printf(" |%d| \n", neighbor[17]);
    printf("%d|%d|%d\n", neighbor[13], neighbor[5], neighbor[11]);
    printf(" |%d| \n", neighbor[15]);
    break;
  case 26:
    printf("%d|%d|%d\n", neighbor[18], neighbor[14], neighbor[22]);
    printf("%d|%d|%d\n", neighbor[10], neighbor[4], neighbor[12]);
    printf("%d|%d|%d\n", neighbor[24], neighbor[16], neighbor[20]);
    printf(" |-| \n");
    printf("%d|%d|%d\n", neighbor[6], neighbor[2], neighbor[8]);
    printf("%d| |%d\n", neighbor[0], neighbor[1]);
    printf("%d|%d|%d\n", neighbor[9], neighbor[3], neighbor[7]);
    printf(" |-| \n");
    printf("%d|%d|%d\n", neighbor[21], neighbor[17], neighbor[25]);
    printf("%d|%d|%d\n", neighbor[13], neighbor[5], neighbor[11]);
    printf("%d|%d|%d\n", neighbor[23], neighbor[15], neighbor[19]);
    break;
  default:
    break;
  }

  printf("\n");
}

void Print_Neighborhood_F(double neighbor[], int n_nbr)
{
  switch (n_nbr) {
  case 4:
    printf(" |%g| \n", neighbor[2]);
    printf("%g| |%g\n", neighbor[0], neighbor[1]);
    printf(" |%g| \n", neighbor[3]);
    break;
  case 8:
    printf("%g|%g|%g\n", neighbor[4], neighbor[2], neighbor[6]);
    printf("%g| |%g\n", neighbor[0], neighbor[1]);
    printf("%g|%g|%g \n", neighbor[7], neighbor[3], neighbor[5]);
    break;
  case 6:
    printf(" |%g| \n", neighbor[4]);
    printf(" |-| \n");
    printf(" |%g| \n", neighbor[2]);
    printf("%g| |%g\n", neighbor[0], neighbor[1]);
    printf(" |%g| \n", neighbor[3]);
    printf(" |-| \n");
    printf(" |%g| \n", neighbor[5]);
    break;
  case 18:
    printf(" |%g| \n", neighbor[14]);
    printf("%g|%g|%g\n", neighbor[10], neighbor[4], neighbor[12]);
    printf(" |%g| \n", neighbor[16]);
    printf(" |-| \n");
    printf("%g|%g|%g\n", neighbor[6], neighbor[2], neighbor[8]);
    printf("%g| |%g\n", neighbor[0], neighbor[1]);
    printf("%g|%g|%g\n", neighbor[9], neighbor[3], neighbor[7]);
    printf(" |-| \n");
    printf(" |%g| \n", neighbor[17]);
    printf("%g|%g|%g\n", neighbor[13], neighbor[5], neighbor[11]);
    printf(" |%g| \n", neighbor[15]);
    break;
  case 26:
    printf("%g|%g|%g\n", neighbor[18], neighbor[14], neighbor[22]);
    printf("%g|%g|%g\n", neighbor[10], neighbor[4], neighbor[12]);
    printf("%g|%g|%g\n", neighbor[24], neighbor[16], neighbor[20]);
    printf(" |-| \n");
    printf("%g|%g|%g\n", neighbor[6], neighbor[2], neighbor[8]);
    printf("%g| |%g\n", neighbor[0], neighbor[1]);
    printf("%g|%g|%g\n", neighbor[9], neighbor[3], neighbor[7]);
    printf(" |-| \n");
    printf("%g|%g|%g\n", neighbor[21], neighbor[17], neighbor[25]);
    printf("%g|%g|%g\n", neighbor[13], neighbor[5], neighbor[11]);
    printf("%g|%g|%g\n", neighbor[23], neighbor[15], neighbor[19]);
    break;
  default:
    break;
  }

  printf("\n");
}

void Stack_Neighbor_Offset(int n_nbr, int width, int height, int neighbor[])
{
  int area;

  switch (n_nbr) {
  case 4:
    neighbor[0] = -1;
    neighbor[1] = -neighbor[0];
    neighbor[2] = -width;
    neighbor[3] = -neighbor[2];
    break;
  case 8:
    Stack_Neighbor_Offset(4, width, height, neighbor);
    neighbor[4] = -width - 1;
    neighbor[5] = -neighbor[4];
    neighbor[6] = -width + 1;
    neighbor[7] = -neighbor[6];
    break;
  case 6:
    Stack_Neighbor_Offset(4, width, height, neighbor);
    area = width * height;
    neighbor[4] = -area;
    neighbor[5] = -neighbor[4];
    break;
  case 10:
    Stack_Neighbor_Offset(6, width, height, neighbor);
    neighbor[6] = -width - 1;
    neighbor[7] = -neighbor[6];
    neighbor[8] = -width + 1;
    neighbor[9] = -neighbor[8];
    break;
  case 18:
    Stack_Neighbor_Offset(6, width, height, neighbor);
    neighbor[6] = -width - 1;
    neighbor[7] = -neighbor[6];
    neighbor[8] = -width + 1;
    neighbor[9] = -neighbor[8];
    neighbor[10] = neighbor[4] - 1;
    neighbor[11] = -neighbor[10];
    neighbor[12] = neighbor[4] + 1;
    neighbor[13] = -neighbor[12];
    neighbor[14] = neighbor[4] - width;
    neighbor[15] = -neighbor[14];
    neighbor[16] = neighbor[4] + width;
    neighbor[17] = -neighbor[16];
    break;
  case 26:
    Stack_Neighbor_Offset(18, width, height, neighbor);
    neighbor[18] = neighbor[4] + neighbor[6];
    neighbor[19] = -neighbor[18];
    neighbor[20] = neighbor[4] + neighbor[7];
    neighbor[21] = -neighbor[20];
    neighbor[22] = neighbor[4] + neighbor[8];
    neighbor[23] = -neighbor[22];
    neighbor[24] = neighbor[4] + neighbor[9];
    neighbor[25] = -neighbor[24];
    break;
  default:
    THROW(ERROR_DATA_VALUE);
    break;
  }
}

void Stack_Neighbor_Positive_Offset(int n_nbr, int width, int height, 
				    int neighbor[])
{
  int area;

  switch (n_nbr) {
  case 4:
    neighbor[0] = 1;
    neighbor[1] = width;
    break;
  case 8:
    Stack_Neighbor_Offset(4, width, height, neighbor);
    neighbor[2] = width + 1;
    neighbor[3] = width - 1;
    break;
  case 6:
    Stack_Neighbor_Offset(4, width, height, neighbor);
    area = width * height;
    neighbor[2] = area;
    break;
  case 18:
    Stack_Neighbor_Offset(6, width, height, neighbor);
    neighbor[3] = width + 1;
    neighbor[4] = width - 1;
    neighbor[5] = neighbor[2] + 1;
    neighbor[6] = neighbor[2] - 1;
    neighbor[7] = neighbor[2] - width;
    neighbor[8] = neighbor[2] + width;
    break;
  case 26:
    Stack_Neighbor_Offset(18, width, height, neighbor);
    neighbor[9] = neighbor[2] + neighbor[3];
    neighbor[10] = neighbor[2] + neighbor[4];
    neighbor[11] = neighbor[2] - neighbor[3];
    neighbor[12] = neighbor[2] - neighbor[4];
    break;
  default:
    THROW(ERROR_DATA_VALUE);
    break;
  }
}

static const double Dist_2d[8] = {1.0, 1.0, 1.0, 1.0, 
				  TZ_SQRT2, TZ_SQRT2, TZ_SQRT2, TZ_SQRT2};
static const double Dist_3d[26] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 
				   TZ_SQRT2, TZ_SQRT2, TZ_SQRT2, TZ_SQRT2,
				   TZ_SQRT2, TZ_SQRT2, TZ_SQRT2, TZ_SQRT2,
				   TZ_SQRT2, TZ_SQRT2, TZ_SQRT2, TZ_SQRT2,
				   TZ_SQRT3, TZ_SQRT3, TZ_SQRT3, TZ_SQRT3, 
				   TZ_SQRT3, TZ_SQRT3, TZ_SQRT3, TZ_SQRT3};

const double* Stack_Neighbor_Dist(int conn)
{
  if ((conn == 4) || (conn == 8)) {
    return Dist_2d;
  } else {
    return Dist_3d;
  }
}

void Stack_Neighbor_Dist_R(int conn, const double *res, double *dist)
{
  if (res[0] <= 0.0 || res[1] <= 0.0 || res[2] <= 0.0) {
    int i;
    for (i = 0; i < conn; i++) {
      dist[i] = 1.0;
    }

    return;
  }

  switch (conn) {
  case 4:
    dist[0] = dist[1] = res[0];
    dist[2] = dist[3] = res[1];
    break;
  case 8:
    dist[0] = dist[1] = res[0];
    dist[2] = dist[3] = res[1];
    dist[4] = dist[5] = dist[6] = dist[7] 
      = sqrt(res[0] * res[0] + res[1] * res[1]);
    break;
  case 6:
    dist[0] = dist[1] = res[0];
    dist[2] = dist[3] = res[1];
    dist[4] = dist[5] = res[2];
    break;
  case 18:
    dist[0] = dist[1] = res[0];
    dist[2] = dist[3] = res[1];
    dist[4] = dist[5] = res[2];
    dist[6] = dist[7] = dist[8] = dist[9] 
      = sqrt(res[0] * res[0] + res[1] * res[1]);
    dist[10] = dist[11] = dist[12] = dist[13] 
      = sqrt(res[0] * res[0] + res[2] * res[2]);
    dist[14] = dist[15] = dist[16] = dist[17] 
      = sqrt(res[1] * res[1] + res[2] * res[2]);
    break;
  case 26:
    dist[0] = dist[1] = res[0];
    dist[2] = dist[3] = res[1];
    dist[4] = dist[5] = res[2];
    dist[6] = dist[7] = dist[8] = dist[9] 
      = sqrt(res[0] * res[0] + res[1] * res[1]);
    dist[10] = dist[11] = dist[12] = dist[13] 
      = sqrt(res[0] * res[0] + res[2] * res[2]);
    dist[14] = dist[15] = dist[16] = dist[17] 
      = sqrt(res[1] * res[1] + res[2] * res[2]);
    dist[18] = dist[19] = dist[20] = dist[21] = dist[22] = dist[23] = dist[24]
      = dist[25] = sqrt(res[0] * res[0] + res[1] * res[1] + res[2] * res[2]);
    break;
  default:
    TZ_ERROR(ERROR_DATA_VALUE);
    break;
  }
}

static const int X_Offset_2d[8] = {-1, 1, 0, 0, -1, 1, 1, -1};
static const int Y_Offset_2d[8] = {0, 0, -1, 1, -1, 1, -1, 1};
static const int Z_Offset_2d[8] = {0, 0, 0, 0, 0, 0, 0, 0};

static const int X_Offset_3d[26] = {-1, 1, 0, 0, 0, 0, -1, 1, 1, -1,
				    -1, 1, 1, -1, 0, 0, 0, 0, -1, 1,
				    1, -1, 1, -1, -1, 1};
static const int Y_Offset_3d[26] = {0, 0, -1, 1, 0, 0, -1, 1, -1, 1,
				    0, 0, 0, 0, -1, 1, 1, -1, -1, 1,
				    1, -1, -1, 1, 1, -1};
static const int Z_Offset_3d[26] = {0, 0, 0, 0, -1, 1, 0, 0, 0, 0,
				    -1, 1, -1, 1, -1, 1, -1, 1, -1, 1,
				    -1, 1, -1, 1, -1, 1};

const int* Stack_Neighbor_X_Offset(int conn)
{
  if (conn == 8) {
    return X_Offset_2d;
  } else {
    return X_Offset_3d;
  }
}

const int* Stack_Neighbor_Y_Offset(int conn)
{
  if (conn == 8) {
    return Y_Offset_2d;
  } else {
    return Y_Offset_3d;
  }
}

const int* Stack_Neighbor_Z_Offset(int conn)
{
  if (conn == 8) {
    return Z_Offset_2d;
  } else {
    return Z_Offset_3d;
  }
}

#define SET_NEIGHBOR4_BOUND(b0, b1, b2, b3, is_in_bound, n, n_in_bound) \
  is_in_bound[0] = b0;							\
  is_in_bound[1] = b1;							\
  is_in_bound[2] = b2;							\
  is_in_bound[3] = b3;							\
  n_in_bound = n;

#define SET_NEIGHBOR8_BOUND(b0, b1, b2, b3, b4, b5, b6, b7, is_in_bound, n, n_in_bound) \
  is_in_bound[0] = b0;							\
  is_in_bound[1] = b1;							\
  is_in_bound[2] = b2;							\
  is_in_bound[3] = b3;							\
  is_in_bound[4] = b4;							\
  is_in_bound[5] = b5;							\
  is_in_bound[6] = b6;							\
  is_in_bound[7] = b7;							\
  n_in_bound = n;

#define SET_NEIGHBOR6_BOUND(b0, b1, b2, b3, b4, b5, is_in_bound, n, n_in_bound) \
  is_in_bound[0] = b0;							\
  is_in_bound[1] = b1;							\
  is_in_bound[2] = b2;							\
  is_in_bound[3] = b3;							\
  is_in_bound[4] = b4;							\
  is_in_bound[5] = b5;							\
  n_in_bound = n;

#define SET_NEIGHBOR10_BOUND(b0, b1, b2, b3, b4, b5, b6, b7, b8, b9, is_in_bound, n, n_in_bound) \
  is_in_bound[0] = b0;							\
  is_in_bound[1] = b1;							\
  is_in_bound[2] = b2;							\
  is_in_bound[3] = b3;							\
  is_in_bound[4] = b4;							\
  is_in_bound[5] = b5;							\
  is_in_bound[6] = b6;							\
  is_in_bound[7] = b7;							\
  is_in_bound[8] = b8;							\
  is_in_bound[9] = b9;							\
  n_in_bound = n;

#define SET_NEIGHBOR18_BOUND(b0, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15, b16, b17, is_in_bound, n, n_in_bound) \
  is_in_bound[0] = b0;							\
  is_in_bound[1] = b1;							\
  is_in_bound[2] = b2;							\
  is_in_bound[3] = b3;							\
  is_in_bound[4] = b4;							\
  is_in_bound[5] = b5;							\
  is_in_bound[6] = b6;							\
  is_in_bound[7] = b7;							\
  is_in_bound[8] = b8;							\
  is_in_bound[9] = b9;							\
  is_in_bound[10] = b10;						\
  is_in_bound[11] = b11;						\
  is_in_bound[12] = b12;						\
  is_in_bound[13] = b13;						\
  is_in_bound[14] = b14;						\
  is_in_bound[15] = b15;						\
  is_in_bound[16] = b16;						\
  is_in_bound[17] = b17;						\
  n_in_bound = n;

#define SET_NEIGHBOR26_BOUND(b0, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15, b16, b17, b18, b19, b20, b21, b22, b23, b24, b25, is_in_bound, n, n_in_bound) \
  is_in_bound[0] = b0;							\
  is_in_bound[1] = b1;							\
  is_in_bound[2] = b2;							\
  is_in_bound[3] = b3;							\
  is_in_bound[4] = b4;							\
  is_in_bound[5] = b5;							\
  is_in_bound[6] = b6;							\
  is_in_bound[7] = b7;							\
  is_in_bound[8] = b8;							\
  is_in_bound[9] = b9;							\
  is_in_bound[10] = b10;						\
  is_in_bound[11] = b11;						\
  is_in_bound[12] = b12;						\
  is_in_bound[13] = b13;						\
  is_in_bound[14] = b14;						\
  is_in_bound[15] = b15;						\
  is_in_bound[16] = b16;						\
  is_in_bound[17] = b17;						\
  is_in_bound[18] = b18;						\
  is_in_bound[19] = b19;						\
  is_in_bound[20] = b20;						\
  is_in_bound[21] = b21;						\
  is_in_bound[22] = b22;						\
  is_in_bound[23] = b23;						\
  is_in_bound[24] = b24;						\
  is_in_bound[25] = b25;						\
  n_in_bound = n;

/* Stack_Neighbor_Bound_Test(): Boundary test for the neighbors.
 *
 * Args: n_nbr - number of neighbors;
 *       cwidth - boudnary index of the first dimension;
 *       cheight - boundary index of the second dimension;
 *       cdepth - boundary index of the third dimension;
 *       x, y, z - cooridnates of the central voxel;
 *       is_in_bound - array to store results if not all neighbors in bound or
 *                     out bound.
 *
 * Return: number of neighbors in bound.
 */
int Stack_Neighbor_Bound_Test(int n_nbr, int cwidth, int cheight, int cdepth, 
			       int x, int y, int z, int is_in_bound[])
{
  int n_in_bound = 0;

  switch (n_nbr) {
  case 4:
    if ((x > 0) && (x < cwidth) && (y > 0) && (y < cheight) && 
	(z >= 0) && (z <= cdepth)) { 
      /* all neighbors are in x bound */
      return n_nbr;
    }

    if ((x < -1) || (x > cwidth + 1) || (y < -1) || (y > cheight + 1) ||
	(z < 0) || (z > cdepth)) {
      /* all neighbors are out bound */
      return 0;
    } else { /* some neighbors are in bound and some are not */
      if (y == -1) {
	if ((x >= 0) && (x <= cwidth)) {
	  SET_NEIGHBOR4_BOUND(0, 0, 0, 1, is_in_bound, 1, n_in_bound);
	} else {
	  return 0;
	}
      } else if (y > cheight) {
	if ((x >= 0) && (x <= cwidth)) {
	  SET_NEIGHBOR4_BOUND(0, 0, 1, 0, is_in_bound, 1, n_in_bound);
	} else {
	  return 0;
	}
      } else if (y == 0) {
	if (x == -1) {
	  SET_NEIGHBOR4_BOUND(0, 1, 0, 0, is_in_bound, 1, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR4_BOUND(0, 1, 0, 1, is_in_bound, 2, n_in_bound);
	} else if (x > cwidth) {
	  SET_NEIGHBOR4_BOUND(1, 0, 0, 0, is_in_bound, 1, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR4_BOUND(1, 0, 0, 1, is_in_bound, 2, n_in_bound);
	} else {
	  SET_NEIGHBOR4_BOUND(1, 1, 0, 1, is_in_bound, 3, n_in_bound);
	}
      } else if (y == cheight ) {
	if (x == -1) {
	  SET_NEIGHBOR4_BOUND(0, 1, 0, 0, is_in_bound, 1, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR4_BOUND(0, 1, 1, 0, is_in_bound, 2, n_in_bound);
	} else if (x > cwidth) {
	  SET_NEIGHBOR4_BOUND(1, 0, 0, 0, is_in_bound, 1, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR4_BOUND(1, 0, 1, 0, is_in_bound, 2, n_in_bound);
	} else {
	  SET_NEIGHBOR4_BOUND(1, 1, 1, 0, is_in_bound, 3, n_in_bound);
	}
      } else {
	if (x == -1) {
	  SET_NEIGHBOR4_BOUND(0, 1, 0, 0, is_in_bound, 1, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR4_BOUND(0, 1, 1, 1, is_in_bound, 3, n_in_bound);
	} else if (x > cwidth) {
	  SET_NEIGHBOR4_BOUND(1, 0, 0, 0, is_in_bound, 1, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR4_BOUND(1, 0, 1, 1, is_in_bound, 3, n_in_bound);
	}
      }
    }
    break;
  case 8:
    if ((x > 0) && (x < cwidth) && (y > 0) && (y < cheight) && 
	(z >= 0) && (z <= cdepth)) {
      /* all neighbors are in x bound */
      return n_nbr;
    }
    
    if ((x < -1) || (x > cwidth + 1) || (y < -1) || (y > cheight + 1) ||
	(z < 0) || (z > cdepth)) {
      /* all neighbors are out bound */
      return 0;
    } else { /* some neighbors are in bound and some are not */
      if (y == -1) {
	if (x == -1) {
	  SET_NEIGHBOR8_BOUND(0, 0, 0, 0, 0, 1, 0, 0, is_in_bound, 
			      1, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR8_BOUND(0, 0, 0, 1, 0, 1, 0, 0, is_in_bound, 
			      2, n_in_bound);
	} else if (x > cwidth) {
	  SET_NEIGHBOR8_BOUND(0, 0, 0, 0, 0, 0, 0, 1, is_in_bound, 
			      1, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR8_BOUND(0, 0, 0, 1, 0, 0, 0, 1, is_in_bound, 
			      2, n_in_bound);
	} else {
	  SET_NEIGHBOR8_BOUND(0, 0, 0, 1, 0, 1, 0, 1, is_in_bound, 
			      3, n_in_bound);
	}
      } else if (y > cheight) {
	if (x == -1) {
	  SET_NEIGHBOR8_BOUND(0, 0, 0, 0, 0, 0, 1, 0, is_in_bound, 
			      1, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR8_BOUND(0, 0, 1, 0, 0, 0, 1, 0, is_in_bound, 
			      2, n_in_bound);
	} else if (x > cwidth) {
	  SET_NEIGHBOR8_BOUND(0, 0, 0, 0, 1, 0, 0, 0, is_in_bound, 
			      1, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR8_BOUND(0, 0, 1, 0, 1, 0, 0, 0, is_in_bound, 
			      2, n_in_bound);
	} else {
	  SET_NEIGHBOR8_BOUND(0, 0, 1, 0, 1, 0, 1, 0, is_in_bound, 
			      3, n_in_bound);
	}
      } else if (y == 0) {
	if (x == -1) {
	  SET_NEIGHBOR8_BOUND(0, 1, 0, 0, 0, 1, 0, 0, is_in_bound, 
			      2, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR8_BOUND(0, 1, 0, 1, 0, 1, 0, 0, is_in_bound, 
			      3, n_in_bound);
	} else if (x > cwidth) {
	  SET_NEIGHBOR8_BOUND(1, 0, 0, 0, 0, 0, 0, 1, is_in_bound, 
			      2, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR8_BOUND(1, 0, 0, 1, 0, 0, 0, 1, is_in_bound, 
			      3, n_in_bound);
	} else {
	  SET_NEIGHBOR8_BOUND(1, 1, 0, 1, 0, 1, 0, 1, is_in_bound, 
			      5, n_in_bound);
	}
      } else if (y == cheight ) {
	if (x == -1) {
	  SET_NEIGHBOR8_BOUND(0, 1, 0, 0, 0, 0, 1, 0, is_in_bound, 
			      2, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR8_BOUND(0, 1, 1, 0, 0, 0, 1, 0, is_in_bound, 
			      3, n_in_bound);
	} else if (x > cwidth) {
	  SET_NEIGHBOR8_BOUND(1, 0, 0, 0, 1, 0, 0, 0, is_in_bound, 
			      2, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR8_BOUND(1, 0, 1, 0, 1, 0, 0, 0, is_in_bound, 
			      3, n_in_bound);
	} else {
	  SET_NEIGHBOR8_BOUND(1, 1, 1, 0, 1, 0, 1, 0, is_in_bound, 
			      5, n_in_bound);
	}
      } else {
	if (x == -1) {
	  SET_NEIGHBOR8_BOUND(0, 1, 0, 0, 0, 1, 1, 0, is_in_bound, 
			      3, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR8_BOUND(0, 1, 1, 1, 0, 1, 1, 0, is_in_bound, 
			      5, n_in_bound);
	} else if (x > cwidth) {
	  SET_NEIGHBOR8_BOUND(1, 0, 0, 0, 1, 0, 0, 1, is_in_bound, 
			      3, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR8_BOUND(1, 0, 1, 1, 1, 0, 0, 1, is_in_bound, 
			      5, n_in_bound);
	}
      }
    }
    break;
  case 6:
    if ((x > 0) && (x < cwidth) && (y > 0) && (y < cheight) && 
	(z > 0) && (z < cdepth)) { 
      /* all neighbors are in x bound */
      return n_nbr;
    }
    
    if ((x < -1) || (x > cwidth + 1) || (y < -1) || (y > cheight + 1) ||
	(z < -1) || (z > cdepth + 1)) {
      /* all neighbors are out bound */
      return 0;
    }
    
    if (z == -1) {
      if ((x == -1) || (y == -1) || (x == cwidth + 1) || (y == cheight + 1)) {
	return 0;
      } else {
	SET_NEIGHBOR6_BOUND(0, 0, 0, 0, 0, 1, is_in_bound, 1, n_in_bound);
      }
    } else if (z == cdepth + 1) {
      if ((x == -1) || (y == -1) || (x == cwidth + 1) || (y == cheight + 1)) {
	return 0;
      } else {
	SET_NEIGHBOR6_BOUND(0, 0, 0, 0, 1, 0, is_in_bound, 1, n_in_bound);
      }
    } else if (z == 0) {
      if (y == -1) {
	if ((x >= 0) && (x <= cwidth)) {
	  SET_NEIGHBOR6_BOUND(0, 0, 0, 1, 0, 0, is_in_bound, 1, n_in_bound);
	} else {
	  return 0;
	}
      } else if (y > cheight) {
	if ((x >= 0) && (x <= cwidth)) {
	  SET_NEIGHBOR6_BOUND(0, 0, 1, 0, 0, 0, is_in_bound, 1, n_in_bound);
	} else {
	  return 0;
	}
      } else if (y == 0) {
	if (x == -1) {
	  SET_NEIGHBOR6_BOUND(0, 1, 0, 0, 0, 0, is_in_bound, 1, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR6_BOUND(0, 1, 0, 1, 0, 1, is_in_bound, 3, n_in_bound);
	} else if (x > cwidth) {
	  SET_NEIGHBOR6_BOUND(1, 0, 0, 0, 0, 0, is_in_bound, 1, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR6_BOUND(1, 0, 0, 1, 0, 1, is_in_bound, 3, n_in_bound);
	} else {
	  SET_NEIGHBOR6_BOUND(1, 1, 0, 1, 0, 1, is_in_bound, 4, n_in_bound);
	}
      } else if (y == cheight ) {
	if (x == -1) {
	  SET_NEIGHBOR6_BOUND(0, 1, 0, 0, 0, 0, is_in_bound, 1, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR6_BOUND(0, 1, 1, 0, 0, 1, is_in_bound, 3, n_in_bound);
	} else if (x > cwidth) {
	  SET_NEIGHBOR6_BOUND(1, 0, 0, 0, 0, 0, is_in_bound, 1, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR6_BOUND(1, 0, 1, 0, 0, 1, is_in_bound, 3, n_in_bound);
	} else {
	  SET_NEIGHBOR6_BOUND(1, 1, 1, 0, 0, 1, is_in_bound, 4, n_in_bound);
	}
      } else {
	if (x == -1) {
	  SET_NEIGHBOR6_BOUND(0, 1, 0, 0, 0, 0, is_in_bound, 1, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR6_BOUND(0, 1, 1, 1, 0, 1, is_in_bound, 4, n_in_bound);
	} else if (x > cwidth) {
	  SET_NEIGHBOR6_BOUND(1, 0, 0, 0, 0, 0, is_in_bound, 1, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR6_BOUND(1, 0, 1, 1, 0, 1, is_in_bound, 4, n_in_bound);
	} else {
	  SET_NEIGHBOR6_BOUND(1, 1, 1, 1, 0, 1, is_in_bound, 5, n_in_bound);
	}
      }
    } else if (z == cdepth){
      if (y == -1) {
	if ((x >= 0) && (x <= cwidth)) {
	  SET_NEIGHBOR6_BOUND(0, 0, 0, 1, 0, 0, is_in_bound, 1, n_in_bound);
	} else {
	  return 0;
	}
      } else if (y > cheight) {
	if ((x >= 0) && (x <= cwidth)) {
	  SET_NEIGHBOR6_BOUND(0, 0, 1, 0, 0, 0, is_in_bound, 1, n_in_bound);
	} else {
	  return 0;
	}
      } else if (y == 0) {
	if (x == -1) {
	  SET_NEIGHBOR6_BOUND(0, 1, 0, 0, 0, 0, is_in_bound, 1, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR6_BOUND(0, 1, 0, 1, 1, 0, is_in_bound, 3, n_in_bound);
	} else if (x > cwidth) {
	  SET_NEIGHBOR6_BOUND(1, 0, 0, 0, 0, 0, is_in_bound, 1, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR6_BOUND(1, 0, 0, 1, 1, 0, is_in_bound, 3, n_in_bound);
	} else {
	  SET_NEIGHBOR6_BOUND(1, 1, 0, 1, 1, 0, is_in_bound, 4, n_in_bound);
	}
      } else if (y == cheight ) {
	if (x == -1) {
	  SET_NEIGHBOR6_BOUND(0, 1, 0, 0, 0, 0, is_in_bound, 1, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR6_BOUND(0, 1, 1, 0, 1, 0, is_in_bound, 3, n_in_bound);
	} else if (x > cwidth) {
	  SET_NEIGHBOR6_BOUND(1, 0, 0, 0, 0, 0, is_in_bound, 1, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR6_BOUND(1, 0, 1, 0, 1, 0, is_in_bound, 3, n_in_bound);
	} else {
	  SET_NEIGHBOR6_BOUND(1, 1, 1, 0, 1, 0, is_in_bound, 4, n_in_bound);
	}
      } else {
	if (x == -1) {
	  SET_NEIGHBOR6_BOUND(0, 1, 0, 0, 0, 0, is_in_bound, 1, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR6_BOUND(0, 1, 1, 1, 1, 0, is_in_bound, 4, n_in_bound);
	} else if (x > cwidth) {
	  SET_NEIGHBOR6_BOUND(1, 0, 0, 0, 0, 0, is_in_bound, 1, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR6_BOUND(1, 0, 1, 1, 1, 0, is_in_bound, 4, n_in_bound);
	} else {
	  SET_NEIGHBOR6_BOUND(1, 1, 1, 1, 1, 0, is_in_bound, 5, n_in_bound);
	}
      }
    } else {      
      if (y == -1) {
	if ((x >= 0) && (x <= cwidth)) {
	  SET_NEIGHBOR6_BOUND(0, 0, 0, 1, 0, 0, is_in_bound, 1, n_in_bound);
	} else {
	  return 0;
	}
      } else if (y > cheight) {
	if ((x >= 0) && (x <= cwidth)) {
	  SET_NEIGHBOR6_BOUND(0, 0, 1, 0, 0, 0, is_in_bound, 1, n_in_bound);
	} else {
	  return 0;
	}
      } else if (y == 0) {
	if (x == -1) {
	  SET_NEIGHBOR6_BOUND(0, 1, 0, 0, 0, 0, is_in_bound, 1, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR6_BOUND(0, 1, 0, 1, 1, 1, is_in_bound, 4, n_in_bound);
	} else if (x > cwidth) {
	  SET_NEIGHBOR6_BOUND(1, 0, 0, 0, 0, 0, is_in_bound, 1, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR6_BOUND(1, 0, 0, 1, 1, 1, is_in_bound, 4, n_in_bound);
	} else {
	  SET_NEIGHBOR6_BOUND(1, 1, 0, 1, 1, 1, is_in_bound, 5, n_in_bound);
	}
      } else if (y == cheight ) {
	if (x == -1) {
	  SET_NEIGHBOR6_BOUND(0, 1, 0, 0, 0, 0, is_in_bound, 1, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR6_BOUND(0, 1, 1, 0, 1, 1, is_in_bound, 4, n_in_bound);
	} else if (x > cwidth) {
	  SET_NEIGHBOR6_BOUND(1, 0, 0, 0, 0, 0, is_in_bound, 1, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR6_BOUND(1, 0, 1, 0, 1, 1, is_in_bound, 4, n_in_bound);
	} else {
	  SET_NEIGHBOR6_BOUND(1, 1, 1, 0, 1, 1, is_in_bound, 5, n_in_bound);
	}
      } else {
	if (x == -1) {
	  SET_NEIGHBOR6_BOUND(0, 1, 0, 0, 0, 0, is_in_bound, 1, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR6_BOUND(0, 1, 1, 1, 1, 1, is_in_bound, 5, n_in_bound);
	} else if (x > cwidth) {
	  SET_NEIGHBOR6_BOUND(1, 0, 0, 0, 0, 0, is_in_bound, 1, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR6_BOUND(1, 0, 1, 1, 1, 1, is_in_bound, 5, n_in_bound);
	}
      }
    }
    break;
  case 18:
    if ((x > 0) && (x < cwidth) && (y > 0) && (y < cheight) && 
	(z > 0) && (z < cdepth)) { 
      /* all neighbors are in x bound */
      return n_nbr;
    }

    if ((x < -1) || (x > cwidth + 1) || (y < -1) || (y > cheight + 1) ||
	(z < -1) || (z > cdepth + 1)) {
      /* all neighbors are out bound */
      return 0;
    }

    if (z == -1) {
      if (y == -1) {	
	if ((x == -1) || (x == cwidth + 1)) {
	  return 0;
	} else {
	  SET_NEIGHBOR18_BOUND(0, 0, 0, 0, 0, 0,
			       0, 0, 0, 0, 0, 0, 
			       0, 0, 0, 1, 0, 0, is_in_bound, 1, n_in_bound);
	}
      } else if (y == cheight + 1) {
	if ((x == -1) || (x == cwidth + 1)) {
	  return 0;
	} else {
	  SET_NEIGHBOR18_BOUND(0, 0, 0, 0, 0, 0,
			       0, 0, 0, 0, 0, 0, 
			       0, 0, 0, 0, 0, 1, is_in_bound, 1, n_in_bound);
	}
      } else if (y == 0) {
	if (x == -1) {
	  SET_NEIGHBOR18_BOUND(0, 0, 0, 0, 0, 0,
			       0, 0, 0, 0, 0, 1, 
			       0, 0, 0, 0, 0, 0, is_in_bound, 1, n_in_bound);
	} else if (x == cwidth + 1) {
	  SET_NEIGHBOR18_BOUND(0, 0, 0, 0, 0, 0,
			       0, 0, 0, 0, 0, 0, 
			       0, 1, 0, 0, 0, 0, is_in_bound, 1, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR18_BOUND(0, 0, 0, 0, 0, 1,
			       0, 0, 0, 0, 0, 1, 
			       0, 0, 0, 1, 0, 0, is_in_bound, 3, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR18_BOUND(0, 0, 0, 0, 0, 1,
			       0, 0, 0, 0, 0, 0, 
			       0, 1, 0, 1, 0, 0, is_in_bound, 3, n_in_bound);
	} else {
	  SET_NEIGHBOR18_BOUND(0, 0, 0, 0, 0, 1,
			       0, 0, 0, 0, 0, 1, 
			       0, 1, 0, 1, 0, 0, is_in_bound, 4, n_in_bound);
	}
      } else if (y == cheight) {
	if (x == -1) {
	  SET_NEIGHBOR18_BOUND(0, 0, 0, 0, 0, 0,
			       0, 0, 0, 0, 0, 1, 
			       0, 0, 0, 0, 0, 0, is_in_bound, 1, n_in_bound);
	} else if (x == cwidth + 1) {
	  SET_NEIGHBOR18_BOUND(0, 0, 0, 0, 0, 0,
			       0, 0, 0, 0, 0, 0, 
			       0, 1, 0, 0, 0, 0, is_in_bound, 1, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR18_BOUND(0, 0, 0, 0, 0, 1,
			       0, 0, 0, 0, 0, 1, 
			       0, 0, 0, 0, 0, 1, is_in_bound, 3, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR18_BOUND(0, 0, 0, 0, 0, 1,
			       0, 0, 0, 0, 0, 0, 
			       0, 1, 0, 0, 0, 1, is_in_bound, 3, n_in_bound);
	} else {
	  SET_NEIGHBOR18_BOUND(0, 0, 0, 0, 0, 1,
			       0, 0, 0, 0, 0, 1, 
			       0, 1, 0, 0, 0, 1, is_in_bound, 4, n_in_bound);
	}
      } else {
	if (x == -1) {
	  SET_NEIGHBOR18_BOUND(0, 0, 0, 0, 0, 0,
			       0, 0, 0, 0, 0, 1, 
			       0, 0, 0, 0, 0, 0, is_in_bound, 1, n_in_bound);
	} else if (x == cwidth + 1) {
	  SET_NEIGHBOR18_BOUND(0, 0, 0, 0, 0, 0,
			       0, 0, 0, 0, 0, 0, 
			       0, 1, 0, 0, 0, 0, is_in_bound, 1, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR18_BOUND(0, 0, 0, 0, 0, 1,
			       0, 0, 0, 0, 0, 1, 
			       0, 0, 0, 1, 0, 1, is_in_bound, 4, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR18_BOUND(0, 0, 0, 0, 0, 1,
			       0, 0, 0, 0, 0, 0, 
			       0, 1, 0, 1, 0, 1, is_in_bound, 4, n_in_bound);
	} else {
	  SET_NEIGHBOR18_BOUND(0, 0, 0, 0, 0, 1,
			       0, 0, 0, 0, 0, 1, 
			       0, 1, 0, 1, 0, 1, is_in_bound, 5, n_in_bound);
	}
      }
    } else if (z > cdepth) {
      if (y == -1) {	
	if ((x == -1) || (x > cwidth)) {
	  return 0;
	} else {
	  SET_NEIGHBOR18_BOUND(0, 0, 0, 0, 0, 0,
			       0, 0, 0, 0, 0, 0, 
			       0, 0, 0, 0, 1, 0, is_in_bound, 1, n_in_bound);
	}
      } else if (y > cheight) {
	if ((x == -1) || (x > cwidth)) {
	  return 0;
	} else {
	  SET_NEIGHBOR18_BOUND(0, 0, 0, 0, 0, 0,
			       0, 0, 0, 0, 0, 0, 
			       0, 0, 1, 0, 0, 0, is_in_bound, 1, n_in_bound);
	}
      } else if (y == 0) {
	if (x == -1) {
	  SET_NEIGHBOR18_BOUND(0, 0, 0, 0, 0, 0,
			       0, 0, 0, 0, 0, 0, 
			       1, 0, 0, 0, 0, 0, is_in_bound, 1, n_in_bound);
	} else if (x > cwidth) {
	  SET_NEIGHBOR18_BOUND(0, 0, 0, 0, 0, 0,
			       0, 0, 0, 0, 1, 0, 
			       0, 0, 0, 0, 0, 0, is_in_bound, 1, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR18_BOUND(0, 0, 0, 0, 1, 0,
			       0, 0, 0, 0, 0, 0, 
			       1, 0, 0, 0, 1, 0, is_in_bound, 3, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR18_BOUND(0, 0, 0, 0, 1, 0,
			       0, 0, 0, 0, 1, 0, 
			       0, 0, 0, 0, 1, 0, is_in_bound, 3, n_in_bound);
	} else {
	  SET_NEIGHBOR18_BOUND(0, 0, 0, 0, 1, 0,
			       0, 0, 0, 0, 1, 0, 
			       1, 0, 0, 0, 1, 0, is_in_bound, 4, n_in_bound);
	}
      } else if (y == cheight) {
	if (x == -1) {
	  SET_NEIGHBOR18_BOUND(0, 0, 0, 0, 0, 0,
			       0, 0, 0, 0, 0, 0, 
			       1, 0, 0, 0, 0, 0, is_in_bound, 1, n_in_bound);
	} else if (x > cwidth) {
	  SET_NEIGHBOR18_BOUND(0, 0, 0, 0, 0, 0,
			       0, 0, 0, 0, 1, 0, 
			       0, 0, 0, 0, 0, 0, is_in_bound, 1, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR18_BOUND(0, 0, 0, 0, 1, 0,
			       0, 0, 0, 0, 0, 0, 
			       1, 0, 1, 0, 0, 0, is_in_bound, 3, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR18_BOUND(0, 0, 0, 0, 1, 0,
			       0, 0, 0, 0, 1, 0, 
			       0, 0, 1, 0, 0, 0, is_in_bound, 3, n_in_bound);
	} else {
	  SET_NEIGHBOR18_BOUND(0, 0, 0, 0, 1, 0,
			       0, 0, 0, 0, 1, 0, 
			       1, 0, 1, 0, 0, 0, is_in_bound, 4, n_in_bound);
	}
      } else {
	if (x == -1) {
	  SET_NEIGHBOR18_BOUND(0, 0, 0, 0, 0, 0,
			       0, 0, 0, 0, 0, 0, 
			       1, 0, 0, 0, 0, 0, is_in_bound, 1, n_in_bound);
	} else if (x > cwidth) {
	  SET_NEIGHBOR18_BOUND(0, 0, 0, 0, 0, 0,
			       0, 0, 0, 0, 1, 0, 
			       0, 0, 0, 0, 0, 0, is_in_bound, 1, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR18_BOUND(0, 0, 0, 0, 1, 0,
			       0, 0, 0, 0, 0, 0, 
			       1, 0, 1, 0, 1, 0, is_in_bound, 4, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR18_BOUND(0, 0, 0, 0, 1, 0,
			       0, 0, 0, 0, 1, 0, 
			       0, 0, 1, 0, 1, 0, is_in_bound, 4, n_in_bound);
	} else {
	  SET_NEIGHBOR18_BOUND(0, 0, 0, 0, 1, 0,
			       0, 0, 0, 0, 1, 0, 
			       1, 0, 1, 0, 1, 0, is_in_bound, 5, n_in_bound);
	}
      }
    } else if (z == 0) {      
      if (y == -1) {
	if (x == -1){
	  SET_NEIGHBOR18_BOUND(0, 0, 0, 0, 0, 0,
			       0, 1, 0, 0, 0, 0, 
			       0, 0, 0, 0, 0, 0, is_in_bound, 1, n_in_bound);
	} else if (x > cwidth){
	  SET_NEIGHBOR18_BOUND(0, 0, 0, 0, 0, 0,
			       0, 0, 0, 1, 0, 0, 
			       0, 0, 0, 0, 0, 0, is_in_bound, 1, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR18_BOUND(0, 0, 0, 1, 0, 0,
			       0, 1, 0, 0, 0, 0, 
			       0, 0, 0, 1, 0, 0, is_in_bound, 3, n_in_bound);
	  
	} else if (x == cwidth) {
	  SET_NEIGHBOR18_BOUND(0, 0, 0, 1, 0, 0,
			       0, 0, 0, 1, 0, 0, 
			       0, 0, 0, 1, 0, 0, is_in_bound, 3, n_in_bound);
	} else {
	  SET_NEIGHBOR18_BOUND(0, 0, 0, 1, 0, 0,
			       0, 1, 0, 1, 0, 0, 
			       0, 0, 0, 1, 0, 0, is_in_bound, 4, n_in_bound);
	}
      } else if (y > cheight) {
	if (x == -1){
	  SET_NEIGHBOR18_BOUND(0, 0, 0, 0, 0, 0,
			       0, 0, 1, 0, 0, 0, 
			       0, 0, 0, 0, 0, 0, is_in_bound, 1, n_in_bound);
	} else if (x > cwidth){
	  SET_NEIGHBOR18_BOUND(0, 0, 0, 0, 0, 0,
			       1, 0, 0, 0, 0, 0, 
			       0, 0, 0, 0, 0, 0, is_in_bound, 1, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR18_BOUND(0, 0, 1, 0, 0, 0,
			       0, 0, 1, 0, 0, 0, 
			       0, 0, 0, 0, 0, 1, is_in_bound, 3, n_in_bound);
	  
	} else if (x == cwidth) {
	  SET_NEIGHBOR18_BOUND(0, 0, 1, 0, 0, 0,
			       1, 0, 0, 0, 0, 0, 
			       0, 0, 0, 0, 0, 1, is_in_bound, 3, n_in_bound);
	} else {
	  SET_NEIGHBOR18_BOUND(0, 0, 1, 0, 0, 0,
			       1, 0, 1, 0, 0, 0, 
			       0, 0, 0, 0, 0, 1, is_in_bound, 4, n_in_bound);
	}
      } else if (y == 0) {
	if (x == -1) {
	  SET_NEIGHBOR18_BOUND(0, 1, 0, 0, 0, 0,
			       0, 1, 0, 0, 0, 1, 
			       0, 0, 0, 0, 0, 0, is_in_bound, 3, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR18_BOUND(0, 1, 0, 1, 0, 1,
			       0, 1, 0, 0, 0, 1, 
			       0, 0, 0, 1, 0, 0, is_in_bound, 6, n_in_bound);
	} else if (x > cwidth) {
	  SET_NEIGHBOR18_BOUND(1, 0, 0, 0, 0, 0,
			       0, 0, 0, 1, 0, 0, 
			       0, 1, 0, 0, 0, 0, is_in_bound, 3, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR18_BOUND(1, 0, 0, 1, 0, 1,
			       0, 0, 0, 1, 0, 0, 
			       0, 1, 0, 1, 0, 0, is_in_bound, 6, n_in_bound);
	} else {
	  SET_NEIGHBOR18_BOUND(1, 1, 0, 1, 0, 1,
			       0, 1, 0, 1, 0, 1, 
			       0, 1, 0, 1, 0, 0, is_in_bound, 9, n_in_bound);
	}
      } else if (y == cheight) {
	if (x == -1) {
	  SET_NEIGHBOR18_BOUND(0, 1, 0, 0, 0, 0,
			       0, 0, 1, 0, 0, 1, 
			       0, 0, 0, 0, 0, 0, is_in_bound, 3, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR18_BOUND(0, 1, 1, 0, 0, 1,
			       0, 0, 1, 0, 0, 1, 
			       0, 0, 0, 0, 0, 1, is_in_bound, 6, n_in_bound);
	} else if (x > cwidth) {
	  SET_NEIGHBOR18_BOUND(1, 0, 0, 0, 0, 0,
			       1, 0, 0, 0, 0, 0, 
			       0, 1, 0, 0, 0, 0, is_in_bound, 3, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR18_BOUND(1, 0, 1, 0, 0, 1,
			       1, 0, 0, 0, 0, 0, 
			       0, 1, 0, 0, 0, 1, is_in_bound, 6, n_in_bound);
	} else {
	  SET_NEIGHBOR18_BOUND(1, 1, 1, 0, 0, 1,
			       1, 0, 1, 0, 0, 1, 
			       0, 1, 0, 0, 0, 1, is_in_bound, 9, n_in_bound);
	}
      } else {
	if (x == -1) {
	  SET_NEIGHBOR18_BOUND(0, 1, 0, 0, 0, 0,
			       0, 1, 1, 0, 0, 1, 
			       0, 0, 0, 0, 0, 0, is_in_bound, 4, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR18_BOUND(0, 1, 1, 1, 0, 1,
			       0, 1, 1, 0, 0, 1, 
			       0, 0, 0, 1, 0, 1, is_in_bound, 9, n_in_bound);
	} else if (x > cwidth) {
	  SET_NEIGHBOR18_BOUND(1, 0, 0, 0, 0, 0,
			       1, 0, 0, 1, 0, 0, 
			       0, 1, 0, 0, 0, 0, is_in_bound, 4, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR18_BOUND(1, 0, 1, 1, 0, 1,
			       1, 0, 0, 1, 0, 0, 
			       0, 1, 0, 1, 0, 1, is_in_bound, 9, n_in_bound);
	} else {
	  SET_NEIGHBOR18_BOUND(1, 1, 1, 1, 0, 1,
			       1, 1, 1, 1, 0, 1, 
			       0, 1, 0, 1, 0, 1, is_in_bound, 13, n_in_bound);
	}
      }
    } else if (z == cdepth) {
      if (y == -1) {
	if (x == -1){
	  SET_NEIGHBOR18_BOUND(0, 0, 0, 0, 0, 0,
			       0, 1, 0, 0, 0, 0, 
			       0, 0, 0, 0, 0, 0, is_in_bound, 1, n_in_bound);
	} else if (x > cwidth){
	  SET_NEIGHBOR18_BOUND(0, 0, 0, 0, 0, 0,
			       0, 0, 0, 1, 0, 0, 
			       0, 0, 0, 0, 0, 0, is_in_bound, 1, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR18_BOUND(0, 0, 0, 1, 0, 0,
			       0, 1, 0, 0, 0, 0, 
			       0, 0, 0, 0, 1, 0, is_in_bound, 3, n_in_bound);
	  
	} else if (x == cwidth) {
	  SET_NEIGHBOR18_BOUND(0, 0, 0, 1, 0, 0,
			       0, 0, 0, 1, 0, 0, 
			       0, 0, 0, 0, 1, 0, is_in_bound, 3, n_in_bound);
	} else {
	  SET_NEIGHBOR18_BOUND(0, 0, 0, 1, 0, 0,
			       0, 1, 0, 1, 0, 0, 
			       0, 0, 0, 0, 1, 0, is_in_bound, 4, n_in_bound);
	}
      } else if (y > cheight) {
	if (x == -1){
	  SET_NEIGHBOR18_BOUND(0, 0, 0, 0, 0, 0,
			       0, 0, 1, 0, 0, 0, 
			       0, 0, 0, 0, 0, 0, is_in_bound, 1, n_in_bound);
	} else if (x > cwidth){
	  SET_NEIGHBOR18_BOUND(0, 0, 0, 0, 0, 0,
			       1, 0, 0, 0, 0, 0, 
			       0, 0, 0, 0, 0, 0, is_in_bound, 1, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR18_BOUND(0, 0, 1, 0, 0, 0,
			       0, 0, 1, 0, 0, 0, 
			       0, 0, 1, 0, 0, 0, is_in_bound, 3, n_in_bound);
	  
	} else if (x == cwidth) {
	  SET_NEIGHBOR18_BOUND(0, 0, 1, 0, 0, 0,
			       1, 0, 0, 0, 0, 0, 
			       0, 0, 1, 0, 0, 0, is_in_bound, 3, n_in_bound);
	} else {
	  SET_NEIGHBOR18_BOUND(0, 0, 1, 0, 0, 0,
			       1, 0, 1, 0, 0, 0, 
			       0, 0, 1, 0, 0, 0, is_in_bound, 4, n_in_bound);
	}
      } else if (y == 0) {
	if (x == -1) {
	  SET_NEIGHBOR18_BOUND(0, 1, 0, 0, 0, 0,
			       0, 1, 0, 0, 0, 0, 
			       1, 0, 0, 0, 0, 0, is_in_bound, 3, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR18_BOUND(0, 1, 0, 1, 1, 0,
			       0, 1, 0, 0, 0, 0, 
			       1, 0, 0, 0, 1, 0, is_in_bound, 6, n_in_bound);
	} else if (x > cwidth) {
	  SET_NEIGHBOR18_BOUND(1, 0, 0, 0, 0, 0,
			       0, 0, 0, 1, 1, 0, 
			       0, 0, 0, 0, 0, 0, is_in_bound, 3, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR18_BOUND(1, 0, 0, 1, 1, 0,
			       0, 0, 0, 1, 1, 0, 
			       0, 0, 0, 0, 1, 0, is_in_bound, 6, n_in_bound);
	} else {
	  SET_NEIGHBOR18_BOUND(1, 1, 0, 1, 1, 0,
			       0, 1, 0, 1, 1, 0, 
			       1, 0, 0, 0, 1, 0, is_in_bound, 9, n_in_bound);
	}
      } else if (y == cheight ) {
	if (x == -1) {
	  SET_NEIGHBOR18_BOUND(0, 1, 0, 0, 0, 0,
			       0, 0, 1, 0, 0, 0, 
			       1, 0, 0, 0, 0, 0, is_in_bound, 3, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR18_BOUND(0, 1, 1, 0, 1, 0,
			       0, 0, 1, 0, 0, 0, 
			       1, 0, 1, 0, 0, 0, is_in_bound, 6, n_in_bound);
	} else if (x > cwidth) {
	  SET_NEIGHBOR18_BOUND(1, 0, 0, 0, 0, 0,
			       1, 0, 0, 0, 1, 0, 
			       0, 0, 0, 0, 0, 0, is_in_bound, 3, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR18_BOUND(1, 0, 1, 0, 1, 0,
			       1, 0, 0, 0, 1, 0, 
			       0, 0, 1, 0, 0, 0, is_in_bound, 6, n_in_bound);
	} else {
	  SET_NEIGHBOR18_BOUND(1, 1, 1, 0, 1, 0,
			       1, 0, 1, 0, 1, 0, 
			       1, 0, 1, 0, 0, 0, is_in_bound, 9, n_in_bound);
	}
      } else {
	if (x == -1) {
	  SET_NEIGHBOR18_BOUND(0, 1, 0, 0, 0, 0,
			       0, 1, 1, 0, 0, 0, 
			       1, 0, 0, 0, 0, 0, is_in_bound, 4, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR18_BOUND(0, 1, 1, 1, 1, 0,
			       0, 1, 1, 0, 0, 0, 
			       1, 0, 1, 0, 1, 0, is_in_bound, 9, n_in_bound);
	} else if (x > cwidth) {
	  SET_NEIGHBOR18_BOUND(1, 0, 0, 0, 0, 0,
			       1, 0, 0, 1, 1, 0, 
			       0, 0, 0, 0, 0, 0, is_in_bound, 4, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR18_BOUND(1, 0, 1, 1, 1, 0,
			       1, 0, 0, 1, 1, 0, 
			       0, 0, 1, 0, 1, 0, is_in_bound, 9, n_in_bound);
	} else {
	  SET_NEIGHBOR18_BOUND(1, 1, 1, 1, 1, 0,
			       1, 1, 1, 1, 1, 0, 
			       1, 0, 1, 0, 1, 0, is_in_bound, 13, n_in_bound);
	}
      }
    } else {    
      if (y == -1) {
	if (x == -1) {
	  SET_NEIGHBOR18_BOUND(0, 0, 0, 0, 0, 0,
			       0, 1, 0, 0, 0, 0, 
			       0, 0, 0, 0, 0, 0, is_in_bound, 1, n_in_bound);
	} else if (x ==0) {
	  SET_NEIGHBOR18_BOUND(0, 0, 0, 1, 0, 0,
			       0, 1, 0, 0, 0, 0, 
			       0, 0, 0, 1, 1, 0, is_in_bound, 4, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR18_BOUND(0, 0, 0, 1, 0, 0,
			       0, 0, 0, 1, 0, 0, 
			       0, 0, 0, 1, 1, 0, is_in_bound, 4, n_in_bound);
	} else if (x > cwidth) {
	  SET_NEIGHBOR18_BOUND(0, 0, 0, 0, 0, 0,
			       0, 0, 0, 1, 0, 0, 
			       0, 0, 0, 0, 0, 0, is_in_bound, 1, n_in_bound);
	} else {
	  SET_NEIGHBOR18_BOUND(0, 0, 0, 1, 0, 0,
			       0, 1, 0, 1, 0, 0, 
			       0, 0, 0, 1, 1, 0, is_in_bound, 5, n_in_bound);
	}
      } else if (y > cheight) {
	if (x == -1) {
	  SET_NEIGHBOR18_BOUND(0, 0, 0, 0, 0, 0,
			       0, 0, 1, 0, 0, 0, 
			       0, 0, 0, 0, 0, 0, is_in_bound, 1, n_in_bound);
	} else if (x ==0) {
	  SET_NEIGHBOR18_BOUND(0, 0, 1, 0, 0, 0,
			       0, 0, 1, 0, 0, 0, 
			       0, 0, 1, 0, 0, 1, is_in_bound, 4, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR18_BOUND(0, 0, 1, 0, 0, 0,
			       1, 0, 0, 0, 0, 0, 
			       0, 0, 1, 0, 0, 1, is_in_bound, 4, n_in_bound);
	} else if (x > cwidth) {
	  SET_NEIGHBOR18_BOUND(0, 0, 0, 0, 0, 0,
			       1, 0, 0, 0, 0, 0, 
			       0, 0, 0, 0, 0, 0, is_in_bound, 1, n_in_bound);
	} else {
	  SET_NEIGHBOR18_BOUND(0, 0, 1, 0, 0, 0,
			       1, 0, 1, 0, 0, 0, 
			       0, 0, 1, 0, 0, 1, is_in_bound, 5, n_in_bound);
	}
      } else if (y == 0) {
	if (x == -1) {
	  SET_NEIGHBOR18_BOUND(0, 1, 0, 0, 0, 0,
			       0, 1, 0, 0, 0, 1, 
			       1, 0, 0, 0, 0, 0, is_in_bound, 4, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR18_BOUND(0, 1, 0, 1, 1, 1,
			       0, 1, 0, 0, 0, 1, 
			       1, 0, 0, 1, 1, 0, is_in_bound, 9, n_in_bound);
	} else if (x > cwidth) {
	  SET_NEIGHBOR18_BOUND(1, 0, 0, 0, 0, 0,
			       0, 0, 0, 1, 1, 0, 
			       0, 1, 0, 0, 0, 0, is_in_bound, 4, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR18_BOUND(1, 0, 0, 1, 1, 1,
			       0, 0, 0, 1, 1, 0, 
			       0, 1, 0, 1, 1, 0, is_in_bound, 9, n_in_bound);
	} else {
	  SET_NEIGHBOR18_BOUND(1, 1, 0, 1, 1, 1,
			       0, 1, 0, 1, 1, 1, 
			       1, 1, 0, 1, 1, 0, is_in_bound, 13, n_in_bound);
	}
      } else if (y == cheight ) {
	if (x == -1) {
	  SET_NEIGHBOR18_BOUND(0, 1, 0, 0, 0, 0,
			       0, 0, 1, 0, 0, 1, 
			       1, 0, 0, 0, 0, 0, is_in_bound, 4, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR18_BOUND(0, 1, 1, 0, 1, 1,
			       0, 0, 1, 0, 0, 1, 
			       1, 0, 1, 0, 0, 1, is_in_bound, 9, n_in_bound);
	} else if (x > cwidth) {
	  SET_NEIGHBOR18_BOUND(1, 0, 0, 0, 0, 0,
			       1, 0, 0, 0, 1, 0, 
			       0, 1, 0, 0, 0, 0, is_in_bound, 4, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR18_BOUND(1, 0, 1, 0, 1, 1,
			       1, 0, 0, 0, 1, 0, 
			       0, 1, 1, 0, 0, 1, is_in_bound, 9, n_in_bound);
	} else {
	  SET_NEIGHBOR18_BOUND(1, 1, 1, 0, 1, 1,
			       1, 0, 1, 0, 1, 1, 
			       1, 1, 1, 0, 0, 1, is_in_bound, 13, n_in_bound);
	}
      } else {
	if (x == -1) {
	  SET_NEIGHBOR18_BOUND(0, 1, 0, 0, 0, 0,
			       0, 1, 1, 0, 0, 1, 
			       1, 0, 0, 0, 0, 0, is_in_bound, 5, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR18_BOUND(0, 1, 1, 1, 1, 1,
			       0, 1, 1, 0, 0, 1, 
			       1, 0, 1, 1, 1, 1, is_in_bound, 13, n_in_bound);
	} else if (x > cwidth) {
	  SET_NEIGHBOR18_BOUND(1, 0, 0, 0, 0, 0,
			       1, 0, 0, 1, 1, 0, 
			       0, 1, 0, 0, 0, 0, is_in_bound, 5, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR18_BOUND(1, 0, 1, 1, 1, 1,
			       1, 0, 0, 1, 1, 0, 
			       0, 1, 1, 1, 1, 1, is_in_bound, 13, n_in_bound);
	}
      }
    }
    break;

  case 26:
    if ((x > 0) && (x < cwidth) && (y > 0) && (y < cheight) && 
	(z > 0) && (z < cdepth)) { 
      /* all neighbors are in bound */
      return n_nbr;
    }

    if ((x < -1) || (x > cwidth + 1) || (y < -1) || (y > cheight + 1) ||
	(z < -1) || (z > cdepth + 1)) {
      /* all neighbors are out of bound */
      return 0;
    }

    if (z == -1) {
      if (y == -1) {
	if (x == -1) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 0, 0, 
			       0, 0, 0, 0, 0, 0,
			       0, 0, 0, 0, 0, 0, 
			       0, 1, 0, 0, 0, 0,
			       0, 0, is_in_bound, 1, n_in_bound);
	} else if (x > cwidth) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 0, 0, 
			       0, 0, 0, 0, 0, 0,
			       0, 0, 0, 0, 0, 0, 
			       0, 0, 0, 0, 0, 1,
			       0, 0, is_in_bound, 1, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 0, 0, 
			       0, 0, 0, 0, 0, 0,
			       0, 0, 0, 1, 0, 0, 
			       0, 1, 0, 0, 0, 0,
			       0, 0, is_in_bound, 2, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 0, 0, 
			       0, 0, 0, 0, 0, 0,
			       0, 0, 0, 1, 0, 0, 
			       0, 0, 0, 0, 0, 1,
			       0, 0, is_in_bound, 2, n_in_bound);
	} else {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 0, 0, 
			       0, 0, 0, 0, 0, 0,
			       0, 0, 0, 1, 0, 0, 
			       0, 1, 0, 0, 0, 1,
			       0, 0, is_in_bound, 3, n_in_bound);
	}
      } else if (y > cheight) {
	if (x == -1) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 0, 0, 
			       0, 0, 0, 0, 0, 0,
			       0, 0, 0, 0, 0, 0, 
			       0, 0, 0, 0, 0, 0,
			       0, 1, is_in_bound, 1, n_in_bound);
	} else if (x == cwidth + 1) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 0, 0, 
			       0, 0, 0, 0, 0, 0,
			       0, 0, 0, 0, 0, 0, 
			       0, 0, 0, 1, 0, 0,
			       0, 0, is_in_bound, 1, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 0, 0, 
			       0, 0, 0, 0, 0, 0,
			       0, 0, 0, 0, 0, 1, 
			       0, 0, 0, 0, 0, 0,
			       0, 1, is_in_bound, 2, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 0, 0, 
			       0, 0, 0, 0, 0, 0,
			       0, 0, 0, 0, 0, 1, 
			       0, 0, 0, 1, 0, 0,
			       0, 0, is_in_bound, 2, n_in_bound);
	} else {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 0, 0, 
			       0, 0, 0, 0, 0, 0,
			       0, 0, 0, 0, 0, 1, 
			       0, 0, 0, 1, 0, 0,
			       0, 1, is_in_bound, 3, n_in_bound);
	}
      } else if (y == 0) {
	if (x == -1) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 0, 0, 
			       0, 0, 0, 0, 0, 1,
			       0, 0, 0, 0, 0, 0, 
			       0, 1, 0, 0, 0, 0,
			       0, 0, is_in_bound, 2, n_in_bound);
	} else if (x == cwidth + 1) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 0, 0, 
			       0, 0, 0, 0, 0, 0,
			       0, 1, 0, 0, 0, 0, 
			       0, 0, 0, 0, 0, 1,
			       0, 0, is_in_bound, 2, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 0, 1, 
			       0, 0, 0, 0, 0, 1,
			       0, 0, 0, 1, 0, 0, 
			       0, 1, 0, 0, 0, 0,
			       0, 0, is_in_bound, 4, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 0, 1, 
			       0, 0, 0, 0, 0, 0,
			       0, 1, 0, 1, 0, 0, 
			       0, 0, 0, 0, 0, 1,
			       0, 0, is_in_bound, 4, n_in_bound);
	} else {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 0, 1, 
			       0, 0, 0, 0, 0, 1,
			       0, 1, 0, 1, 0, 0, 
			       0, 1, 0, 0, 0, 1,
			       0, 0, is_in_bound, 6, n_in_bound);
	}
      } else if (y == cheight) {
	if (x == -1) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 0, 0, 
			       0, 0, 0, 0, 0, 1,
			       0, 0, 0, 0, 0, 0, 
			       0, 0, 0, 0, 0, 0,
			       0, 1, is_in_bound, 2, n_in_bound);
	} else if (x > cwidth) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 0, 0, 
			       0, 0, 0, 0, 0, 0,
			       0, 1, 0, 0, 0, 0, 
			       0, 0, 0, 1, 0, 0,
			       0, 0, is_in_bound, 2, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 0, 1, 
			       0, 0, 0, 0, 0, 1,
			       0, 0, 0, 0, 0, 1, 
			       0, 0, 0, 0, 0, 0,
			       0, 1, is_in_bound, 4, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 0, 1, 
			       0, 0, 0, 0, 0, 0,
			       0, 1, 0, 0, 0, 1, 
			       0, 0, 0, 1, 0, 0,
			       0, 0, is_in_bound, 4, n_in_bound);
	} else {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 0, 1, 
			       0, 0, 0, 0, 0, 1,
			       0, 1, 0, 0, 0, 1, 
			       0, 0, 0, 1, 0, 0,
			       0, 1, is_in_bound, 6, n_in_bound);
	}
      } else {
	if (x == -1) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 0, 0, 
			       0, 0, 0, 0, 0, 1,
			       0, 0, 0, 0, 0, 0, 
			       0, 1, 0, 0, 0, 0,
			       0, 1, is_in_bound, 3, n_in_bound);
	} else if (x > cwidth) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 0, 0, 
			       0, 0, 0, 0, 0, 0,
			       0, 1, 0, 0, 0, 0, 
			       0, 0, 0, 1, 0, 1,
			       0, 0, is_in_bound, 3, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 0, 1, 
			       0, 0, 0, 0, 0, 1,
			       0, 0, 0, 1, 0, 1, 
			       0, 1, 0, 0, 0, 0,
			       0, 1, is_in_bound, 6, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 0, 1, 
			       0, 0, 0, 0, 0, 0,
			       0, 1, 0, 1, 0, 1, 
			       0, 0, 0, 1, 0, 1,
			       0, 0, is_in_bound, 6, n_in_bound);
	} else {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 0, 1, 
			       0, 0, 0, 0, 0, 1,
			       0, 1, 0, 1, 0, 1, 
			       0, 1, 0, 1, 0, 1,
			       0, 1, is_in_bound, 9, n_in_bound);
	}
      }
    } else if (z > cdepth) {
      if (y == -1) {
	if (x == -1) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 0, 0, 
			       0, 0, 0, 0, 0, 0,
			       0, 0, 0, 0, 0, 0, 
			       0, 0, 1, 0, 0, 0,
			       0, 0, is_in_bound, 1, n_in_bound);
	} else if (x == cwidth + 1) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 0, 0, 
			       0, 0, 0, 0, 0, 0,
			       0, 0, 0, 0, 0, 0, 
			       0, 0, 0, 0, 0, 0,
			       1, 0, is_in_bound, 1, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 0, 0, 
			       0, 0, 0, 0, 0, 0,
			       0, 0, 0, 0, 1, 0, 
			       0, 0, 1, 0, 0, 0,
			       0, 0, is_in_bound, 2, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 0, 0, 
			       0, 0, 0, 0, 0, 0,
			       0, 0, 0, 0, 1, 0, 
			       0, 0, 0, 0, 0, 0,
			       1, 0, is_in_bound, 2, n_in_bound);
	} else {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 0, 0, 
			       0, 0, 0, 0, 0, 0,
			       0, 0, 0, 0, 1, 0, 
			       0, 0, 1, 0, 0, 0,
			       1, 0, is_in_bound, 3, n_in_bound);
	}
      } else if (y > cheight) {
	if (x == -1) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 0, 0, 
			       0, 0, 0, 0, 0, 0,
			       0, 0, 0, 0, 0, 0, 
			       0, 0, 0, 0, 1, 0,
			       0, 0, is_in_bound, 1, n_in_bound);
	} else if (x > cwidth) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 0, 0, 
			       0, 0, 0, 0, 0, 0,
			       0, 0, 0, 0, 0, 0, 
			       1, 0, 0, 0, 0, 0,
			       0, 0, is_in_bound, 1, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 0, 0, 
			       0, 0, 0, 0, 0, 0,
			       0, 0, 1, 0, 0, 0, 
			       0, 0, 0, 0, 1, 0,
			       0, 0, is_in_bound, 2, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 0, 0, 
			       0, 0, 0, 0, 0, 0,
			       0, 0, 1, 0, 0, 0, 
			       1, 0, 0, 0, 0, 0,
			       0, 0, is_in_bound, 2, n_in_bound);
	} else {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 0, 0, 
			       0, 0, 0, 0, 0, 0,
			       0, 0, 1, 0, 0, 0, 
			       1, 0, 0, 0, 1, 0,
			       0, 0, is_in_bound, 3, n_in_bound);
	}
      } else if (y == 0) {
	if (x == -1) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 0, 0, 
			       0, 0, 0, 0, 0, 0,
			       1, 0, 0, 0, 0, 0, 
			       0, 0, 1, 0, 0, 0,
			       0, 0, is_in_bound, 2, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 1, 0, 
			       0, 0, 0, 0, 0, 0,
			       1, 0, 0, 0, 1, 0, 
			       0, 0, 1, 0, 0, 0,
			       0, 0, is_in_bound, 4, n_in_bound);
	} else if (x > cwidth) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 0, 0, 
			       0, 0, 0, 0, 1, 0,
			       0, 0, 0, 0, 0, 0, 
			       0, 0, 0, 0, 0, 0,
			       1, 0, is_in_bound, 2, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 1, 0, 
			       0, 0, 0, 0, 1, 0,
			       0, 0, 0, 0, 1, 0, 
			       0, 0, 0, 0, 0, 0,
			       1, 0, is_in_bound, 4, n_in_bound);
	} else {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 1, 0, 
			       0, 0, 0, 0, 1, 0,
			       1, 0, 0, 0, 1, 0, 
			       0, 0, 1, 0, 0, 0,
			       1, 0, is_in_bound, 6, n_in_bound);
	}
      } else if (y == cheight ) {
	if (x == -1) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 0, 0, 
			       0, 0, 0, 0, 0, 0,
			       1, 0, 0, 0, 0, 0, 
			       0, 0, 0, 0, 1, 0,
			       0, 0, is_in_bound, 2, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 1, 0, 
			       0, 0, 0, 0, 0, 0,
			       1, 0, 1, 0, 0, 0, 
			       0, 0, 0, 0, 1, 0,
			       0, 0, is_in_bound, 4, n_in_bound);
	} else if (x > cwidth) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 0, 0, 
			       0, 0, 0, 0, 1, 0,
			       0, 0, 0, 0, 0, 0, 
			       1, 0, 0, 0, 0, 0,
			       0, 0, is_in_bound, 2, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 1, 0, 
			       0, 0, 0, 0, 1, 0,
			       0, 0, 1, 0, 0, 0, 
			       1, 0, 0, 0, 0, 0,
			       0, 0, is_in_bound, 4, n_in_bound);
	} else {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 1, 0, 
			       0, 0, 0, 0, 1, 0,
			       1, 0, 1, 0, 0, 0, 
			       1, 0, 0, 0, 1, 0,
			       0, 0, is_in_bound, 6, n_in_bound);
	}
      } else {
	if (x == -1) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 0, 0, 
			       0, 0, 0, 0, 0, 0,
			       1, 0, 0, 0, 0, 0, 
			       0, 0, 1, 0, 1, 0,
			       0, 0, is_in_bound, 3, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 1, 0, 
			       0, 0, 0, 0, 0, 0,
			       1, 0, 1, 0, 1, 0, 
			       0, 0, 1, 0, 1, 0,
			       0, 0, is_in_bound, 6, n_in_bound);
	} else if (x > cwidth) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 0, 0, 
			       0, 0, 0, 0, 1, 0,
			       0, 0, 0, 0, 0, 0, 
			       1, 0, 0, 0, 0, 0,
			       1, 0, is_in_bound, 3, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 1, 0, 
			       0, 0, 0, 0, 1, 0,
			       0, 0, 1, 0, 1, 0, 
			       1, 0, 0, 0, 0, 0,
			       1, 0, is_in_bound, 6, n_in_bound);
	} else {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 1, 0, 
			       0, 0, 0, 0, 1, 0,
			       1, 0, 1, 0, 1, 0, 
			       1, 0, 1, 0, 1, 0,
			       1, 0, is_in_bound, 9, n_in_bound);
	}
      }
    } else if (z == 0) {
      if (y == -1) {
	if (x == -1) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 0, 0, 
			       0, 1, 0, 0, 0, 0,
			       0, 0, 0, 0, 0, 0, 
			       0, 1, 0, 0, 0, 0,
			       0, 0, is_in_bound, 2, n_in_bound);
	} else if (x == cwidth + 1) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 0, 0, 
			       0, 0, 0, 1, 0, 0,
			       0, 0, 0, 0, 0, 0, 
			       0, 0, 0, 0, 0, 1,
			       0, 0, is_in_bound, 2, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 1, 0, 0, 
			       0, 1, 0, 0, 0, 0,
			       0, 0, 0, 1, 0, 0, 
			       0, 1, 0, 0, 0, 0,
			       0, 0, is_in_bound, 4, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 1, 0, 0, 
			       0, 0, 0, 1, 0, 0,
			       0, 0, 0, 1, 0, 0, 
			       0, 0, 0, 0, 0, 1,
			       0, 0, is_in_bound, 4, n_in_bound);
	} else {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 1, 0, 0, 
			       0, 1, 0, 1, 0, 0,
			       0, 0, 0, 1, 0, 0, 
			       0, 1, 0, 0, 0, 1,
			       0, 0, is_in_bound, 6, n_in_bound);
	}
      } else if (y > cheight) {
	if (x == -1) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 0, 0, 
			       0, 0, 1, 0, 0, 0,
			       0, 0, 0, 0, 0, 0, 
			       0, 0, 0, 0, 0, 0,
			       0, 1, is_in_bound, 2, n_in_bound);
	} else if (x == cwidth + 1) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 0, 0, 
			       1, 0, 0, 0, 0, 0,
			       0, 0, 0, 0, 0, 0, 
			       0, 0, 0, 1, 0, 0,
			       0, 0, is_in_bound, 2, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR26_BOUND(0, 0, 1, 0, 0, 0, 
			       0, 0, 1, 0, 0, 0,
			       0, 0, 0, 0, 0, 1, 
			       0, 0, 0, 0, 0, 0,
			       0, 1, is_in_bound, 4, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR26_BOUND(0, 0, 1, 0, 0, 0, 
			       1, 0, 0, 0, 0, 0,
			       0, 0, 0, 0, 0, 1, 
			       0, 0, 0, 1, 0, 0,
			       0, 0, is_in_bound, 4, n_in_bound);
	} else {
	  SET_NEIGHBOR26_BOUND(0, 0, 1, 0, 0, 0, 
			       1, 0, 1, 0, 0, 0,
			       0, 0, 0, 0, 0, 1, 
			       0, 0, 0, 1, 0, 0,
			       0, 1, is_in_bound, 6, n_in_bound);
	}
      } else if (y == 0) {
	if (x == -1) {
	  SET_NEIGHBOR26_BOUND(0, 1, 0, 0, 0, 0, 
			       0, 1, 0, 0, 0, 1,
			       0, 0, 0, 0, 0, 0, 
			       0, 1, 0, 0, 0, 0,
			       0, 0, is_in_bound, 4, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR26_BOUND(0, 1, 0, 1, 0, 1, 
			       0, 1, 0, 0, 0, 1,
			       0, 0, 0, 1, 0, 0, 
			       0, 1, 0, 0, 0, 0,
			       0, 0, is_in_bound, 7, n_in_bound);
	} else if (x > cwidth) {
	  SET_NEIGHBOR26_BOUND(1, 0, 0, 0, 0, 0, 
			       0, 0, 0, 1, 0, 0,
			       0, 1, 0, 0, 0, 0, 
			       0, 0, 0, 0, 0, 1,
			       0, 0, is_in_bound, 4, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR26_BOUND(1, 0, 0, 1, 0, 1, 
			       0, 0, 0, 1, 0, 0,
			       0, 1, 0, 1, 0, 0, 
			       0, 0, 0, 0, 0, 1,
			       0, 0, is_in_bound, 7, n_in_bound);
	} else {
	  SET_NEIGHBOR26_BOUND(1, 1, 0, 1, 0, 1, 
			       0, 1, 0, 1, 0, 1,
			       0, 1, 0, 1, 0, 0, 
			       0, 1, 0, 0, 0, 1,
			       0, 0, is_in_bound, 11, n_in_bound);
	}
      } else if (y == cheight) {
	if (x == -1) {
	  SET_NEIGHBOR26_BOUND(0, 1, 0, 0, 0, 0, 
			       0, 0, 1, 0, 0, 1,
			       0, 0, 0, 0, 0, 0, 
			       0, 0, 0, 0, 0, 0,
			       0, 1, is_in_bound, 4, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR26_BOUND(0, 1, 1, 0, 0, 1, 
			       0, 0, 1, 0, 0, 1,
			       0, 0, 0, 0, 0, 1, 
			       0, 0, 0, 0, 0, 0,
			       0, 1, is_in_bound, 7, n_in_bound);
	} else if (x > cwidth) {
	  SET_NEIGHBOR26_BOUND(1, 0, 0, 0, 0, 0, 
			       1, 0, 0, 0, 0, 0,
			       0, 1, 0, 0, 0, 0, 
			       0, 0, 0, 1, 0, 0,
			       0, 0, is_in_bound, 4, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR26_BOUND(1, 0, 1, 0, 0, 1, 
			       1, 0, 0, 0, 0, 0,
			       0, 1, 0, 0, 0, 1, 
			       0, 0, 0, 1, 0, 0,
			       0, 0, is_in_bound, 7, n_in_bound);
	} else {
	  SET_NEIGHBOR26_BOUND(1, 1, 1, 0, 0, 1, 
			       1, 0, 1, 0, 0, 1,
			       0, 1, 0, 0, 0, 1, 
			       0, 0, 0, 1, 0, 0,
			       0, 1, is_in_bound, 11, n_in_bound);
	}
      } else {
	if (x == -1) {
	  SET_NEIGHBOR26_BOUND(0, 1, 0, 0, 0, 0, 
			       0, 1, 1, 0, 0, 1,
			       0, 0, 0, 0, 0, 0, 
			       0, 1, 0, 0, 0, 0,
			       0, 1, is_in_bound, 6, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR26_BOUND(0, 1, 1, 1, 0, 1, 
			       0, 1, 1, 0, 0, 1,
			       0, 0, 0, 1, 0, 1, 
			       0, 1, 0, 0, 0, 0,
			       0, 1, is_in_bound, 11, n_in_bound);
	} else if (x > cwidth) {
	  SET_NEIGHBOR26_BOUND(1, 0, 0, 0, 0, 0, 
			       1, 0, 0, 1, 0, 0,
			       0, 1, 0, 0, 0, 0, 
			       0, 0, 0, 1, 0, 1,
			       0, 0, is_in_bound, 6, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR26_BOUND(1, 0, 1, 1, 0, 1, 
			       1, 0, 0, 1, 0, 0,
			       0, 1, 0, 1, 0, 1, 
			       0, 0, 0, 1, 0, 1,
			       0, 0, is_in_bound, 11, n_in_bound);
	} else {
	  SET_NEIGHBOR26_BOUND(1, 1, 1, 1, 0, 1, 
			       1, 1, 1, 1, 0, 1,
			       0, 1, 0, 1, 0, 1, 
			       0, 1, 0, 1, 0, 1,
			       0, 1, is_in_bound, 17, n_in_bound);
	}
      }
    } else if (z == cdepth) {
      if (y == -1) {
	if (x == -1) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 0, 0, 
			       0, 1, 0, 0, 0, 0,
			       0, 0, 0, 0, 0, 0, 
			       0, 0, 1, 0, 0, 0,
			       0, 0, is_in_bound, 2, n_in_bound);
	} else if (x == cwidth + 1) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 0, 0, 
			       0, 0, 0, 1, 0, 0,
			       0, 0, 0, 0, 0, 0, 
			       0, 0, 0, 0, 0, 0,
			       1, 0, is_in_bound, 2, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 1, 0, 0, 
			       0, 1, 0, 0, 0, 0,
			       0, 0, 0, 0, 1, 0, 
			       0, 0, 1, 0, 0, 0,
			       0, 0, is_in_bound, 4, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 1, 0, 0, 
			       0, 0, 0, 1, 0, 0,
			       0, 0, 0, 0, 1, 0, 
			       0, 0, 0, 0, 0, 0,
			       1, 0, is_in_bound, 4, n_in_bound);
	} else {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 1, 0, 0, 
			       0, 1, 0, 1, 0, 0,
			       0, 0, 0, 0, 1, 0, 
			       0, 0, 1, 0, 0, 0,
			       1, 0, is_in_bound, 6, n_in_bound);
	}
      } else if (y > cheight) {
	if (x == -1) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 0, 0, 
			       0, 0, 1, 0, 0, 0,
			       0, 0, 0, 0, 0, 0, 
			       0, 0, 0, 0, 1, 0,
			       0, 0, is_in_bound, 2, n_in_bound);	
	} else if (x == cwidth + 1) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 0, 0, 
			       1, 0, 0, 0, 0, 0,
			       0, 0, 0, 0, 0, 0, 
			       1, 0, 0, 0, 0, 0,
			       0, 0, is_in_bound, 2, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR26_BOUND(0, 0, 1, 0, 0, 0, 
			       0, 0, 1, 0, 0, 0,
			       0, 0, 1, 0, 0, 0, 
			       0, 0, 0, 0, 1, 0,
			       0, 0, is_in_bound, 4, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR26_BOUND(0, 0, 1, 0, 0, 0, 
			       1, 0, 0, 0, 0, 0,
			       0, 0, 1, 0, 0, 0, 
			       1, 0, 0, 0, 0, 0,
			       0, 0, is_in_bound, 4, n_in_bound);
	} else {
	  SET_NEIGHBOR26_BOUND(0, 0, 1, 0, 0, 0, 
			       1, 0, 1, 0, 0, 0,
			       0, 0, 1, 0, 0, 0, 
			       1, 0, 0, 0, 1, 0,
			       0, 0, is_in_bound, 6, n_in_bound);
	}
      } else if (y == 0) {
	if (x == -1) {
	  SET_NEIGHBOR26_BOUND(0, 1, 0, 0, 0, 0, 
			       0, 1, 0, 0, 0, 0,
			       1, 0, 0, 0, 0, 0, 
			       0, 0, 1, 0, 0, 0,
			       0, 0, is_in_bound, 4, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR26_BOUND(0, 1, 0, 1, 1, 0, 
			       0, 1, 0, 0, 0, 0,
			       1, 0, 0, 0, 1, 0, 
			       0, 0, 1, 0, 0, 0,
			       0, 0, is_in_bound, 7, n_in_bound);
	} else if (x > cwidth) {
	  SET_NEIGHBOR26_BOUND(1, 0, 0, 0, 0, 0, 
			       0, 0, 0, 1, 1, 0,
			       0, 0, 0, 0, 0, 0, 
			       0, 0, 0, 0, 0, 0,
			       1, 0, is_in_bound, 4, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR26_BOUND(1, 0, 0, 1, 1, 0, 
			       0, 0, 0, 1, 1, 0,
			       0, 0, 0, 0, 1, 0, 
			       0, 0, 0, 0, 0, 0,
			       1, 0, is_in_bound, 7, n_in_bound);
	} else {
	  SET_NEIGHBOR26_BOUND(1, 1, 0, 1, 1, 0, 
			       0, 1, 0, 1, 1, 0,
			       1, 0, 0, 0, 1, 0, 
			       0, 0, 1, 0, 0, 0,
			       1, 0, is_in_bound, 11, n_in_bound);
	}
      } else if (y == cheight ) {
	if (x == -1) {
	  SET_NEIGHBOR26_BOUND(0, 1, 0, 0, 0, 0, 
			       0, 0, 1, 0, 0, 0,
			       1, 0, 0, 0, 0, 0, 
			       0, 0, 0, 0, 1, 0,
			       0, 0, is_in_bound, 4, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR26_BOUND(0, 1, 1, 0, 1, 0, 
			       0, 0, 1, 0, 0, 0,
			       1, 0, 1, 0, 0, 0, 
			       0, 0, 0, 0, 1, 0,
			       0, 0, is_in_bound, 7, n_in_bound);
	} else if (x > cwidth) {
	  SET_NEIGHBOR26_BOUND(1, 0, 0, 0, 0, 0, 
			       1, 0, 0, 0, 1, 0,
			       0, 0, 0, 0, 0, 0, 
			       1, 0, 0, 0, 0, 0,
			       0, 0, is_in_bound, 4, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR26_BOUND(1, 0, 1, 0, 1, 0, 
			       1, 0, 0, 0, 1, 0,
			       0, 0, 1, 0, 0, 0, 
			       1, 0, 0, 0, 0, 0,
			       0, 0, is_in_bound, 7, n_in_bound);
	} else {
	  SET_NEIGHBOR26_BOUND(1, 1, 1, 0, 1, 0, 
			       1, 0, 1, 0, 1, 0,
			       1, 0, 1, 0, 0, 0, 
			       1, 0, 0, 0, 1, 0,
			       0, 0, is_in_bound, 11, n_in_bound);
	}
      } else {
	if (x == -1) {
	  SET_NEIGHBOR26_BOUND(0, 1, 0, 0, 0, 0, 
			       0, 1, 1, 0, 0, 0,
			       1, 0, 0, 0, 0, 0, 
			       0, 0, 1, 0, 1, 0,
			       0, 0, is_in_bound, 6, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR26_BOUND(0, 1, 1, 1, 1, 0, 
			       0, 1, 1, 0, 0, 0,
			       1, 0, 1, 0, 1, 0, 
			       0, 0, 1, 0, 1, 0,
			       0, 0, is_in_bound, 11, n_in_bound);
	} else if (x > cwidth) {
	  SET_NEIGHBOR26_BOUND(1, 0, 0, 0, 0, 0, 
			       1, 0, 0, 1, 1, 0,
			       0, 0, 0, 0, 0, 0, 
			       1, 0, 0, 0, 0, 0,
			       1, 0, is_in_bound, 6, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR26_BOUND(1, 0, 1, 1, 1, 0, 
			       1, 0, 0, 1, 1, 0,
			       0, 0, 1, 0, 1, 0, 
			       1, 0, 0, 0, 0, 0,
			       1, 0, is_in_bound, 11, n_in_bound);
	} else {
	  SET_NEIGHBOR26_BOUND(1, 1, 1, 1, 1, 0, 
			       1, 1, 1, 1, 1, 0,
			       1, 0, 1, 0, 1, 0, 
			       1, 0, 1, 0, 1, 0,
			       1, 0, is_in_bound, 17, n_in_bound);
	}
      }
    } else { /* z in bound */
      if (y == -1) {
	if (x == -1) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 0, 0, 
			       0, 1, 0, 0, 0, 0,
			       0, 0, 0, 0, 0, 0, 
			       0, 1, 1, 0, 0, 0,
			       0, 0, is_in_bound, 3, n_in_bound);	
	} else if (x == cwidth + 1) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 0, 0, 
			       0, 0, 0, 1, 0, 0,
			       0, 0, 0, 0, 0, 0, 
			       0, 0, 0, 0, 0, 1,
			       1, 0, is_in_bound, 3, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 1, 0, 0, 
			       0, 1, 0, 0, 0, 0,
			       0, 0, 0, 1, 1, 0, 
			       0, 1, 1, 0, 0, 0,
			       0, 0, is_in_bound, 6, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 1, 0, 0, 
			       0, 0, 0, 1, 0, 0,
			       0, 0, 0, 1, 1, 0, 
			       0, 0, 0, 0, 0, 1,
			       1, 0, is_in_bound, 6, n_in_bound);
	} else {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 1, 0, 0, 
			       0, 1, 0, 1, 0, 0,
			       0, 0, 0, 1, 1, 0, 
			       0, 1, 1, 0, 0, 1,
			       1, 0, is_in_bound, 9, n_in_bound);
	}
      } else if (y > cheight) {
	if (x == -1) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 0, 0, 
			       0, 0, 1, 0, 0, 0,
			       0, 0, 0, 0, 0, 0, 
			       0, 0, 0, 0, 1, 0,
			       0, 1, is_in_bound, 3, n_in_bound);	
	} else if (x > cwidth) {
	  SET_NEIGHBOR26_BOUND(0, 0, 0, 0, 0, 0, 
			       1, 0, 0, 0, 0, 0,
			       0, 0, 0, 0, 0, 0, 
			       1, 0, 0, 1, 0, 0,
			       0, 0, is_in_bound, 3, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR26_BOUND(0, 0, 1, 0, 0, 0, 
			       0, 0, 1, 0, 0, 0,
			       0, 0, 1, 0, 0, 1, 
			       0, 0, 0, 0, 1, 0,
			       0, 1, is_in_bound, 6, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR26_BOUND(0, 0, 1, 0, 0, 0, 
			       1, 0, 0, 0, 0, 0,
			       0, 0, 1, 0, 0, 1, 
			       1, 0, 0, 1, 0, 0,
			       0, 0, is_in_bound, 6, n_in_bound);
	} else {
	  SET_NEIGHBOR26_BOUND(0, 0, 1, 0, 0, 0, 
			       1, 0, 1, 0, 0, 0,
			       0, 0, 1, 0, 0, 1, 
			       1, 0, 0, 1, 1, 0,
			       0, 1, is_in_bound, 9, n_in_bound);
	}
      } else if (y == 0) {
	if (x == -1) {
	  SET_NEIGHBOR26_BOUND(0, 1, 0, 0, 0, 0, 
			       0, 1, 0, 0, 0, 1,
			       1, 0, 0, 0, 0, 0, 
			       0, 1, 1, 0, 0, 0,
			       0, 0, is_in_bound, 6, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR26_BOUND(0, 1, 0, 1, 1, 1, 
			       0, 1, 0, 0, 0, 1,
			       1, 0, 0, 1, 1, 0, 
			       0, 1, 1, 0, 0, 0,
			       0, 0, is_in_bound, 11, n_in_bound);
	} else if (x > cwidth) {
	  SET_NEIGHBOR26_BOUND(1, 0, 0, 0, 0, 0, 
			       0, 0, 0, 1, 1, 0,
			       0, 1, 0, 0, 0, 0, 
			       0, 0, 0, 0, 0, 1,
			       1, 0, is_in_bound, 6, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR26_BOUND(1, 0, 0, 1, 1, 1, 
			       0, 0, 0, 1, 1, 0,
			       0, 1, 0, 1, 1, 0, 
			       0, 0, 0, 0, 0, 1,
			       1, 0, is_in_bound, 11, n_in_bound);
	} else {
	  SET_NEIGHBOR26_BOUND(1, 1, 0, 1, 1, 1, 
			       0, 1, 0, 1, 1, 1,
			       1, 1, 0, 1, 1, 0, 
			       0, 1, 1, 0, 0, 1,
			       1, 0, is_in_bound, 17, n_in_bound);
	}
      } else if (y == cheight ) {
	if (x == -1) {
	  SET_NEIGHBOR26_BOUND(0, 1, 0, 0, 0, 0, 
			       0, 0, 1, 0, 0, 1,
			       1, 0, 0, 0, 0, 0, 
			       0, 0, 0, 0, 1, 0,
			       0, 1, is_in_bound, 6, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR26_BOUND(0, 1, 1, 0, 1, 1, 
			       0, 0, 1, 0, 0, 1,
			       1, 0, 1, 0, 0, 1, 
			       0, 0, 0, 0, 1, 0,
			       0, 1, is_in_bound, 11, n_in_bound);
	} else if (x > cwidth) {
	  SET_NEIGHBOR26_BOUND(1, 0, 0, 0, 0, 0, 
			       1, 0, 0, 0, 1, 0,
			       0, 1, 0, 0, 0, 0, 
			       1, 0, 0, 1, 0, 0,
			       0, 0, is_in_bound, 6, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR26_BOUND(1, 0, 1, 0, 1, 1, 
			       1, 0, 0, 0, 1, 0,
			       0, 1, 1, 0, 0, 1, 
			       1, 0, 0, 1, 0, 0,
			       0, 0, is_in_bound, 11, n_in_bound);
	} else {
	  SET_NEIGHBOR26_BOUND(1, 1, 1, 0, 1, 1, 
			       1, 0, 1, 0, 1, 1,
			       1, 1, 1, 0, 0, 1, 
			       1, 0, 0, 1, 1, 0,
			       0, 1, is_in_bound, 17, n_in_bound);
	}
      } else { /* y in bound */
	if (x == -1) {
	  SET_NEIGHBOR26_BOUND(0, 1, 0, 0, 0, 0, 
			       0, 1, 1, 0, 0, 1,
			       1, 0, 0, 0, 0, 0, 
			       0, 1, 1, 0, 1, 0,
			       0, 1, is_in_bound, 9, n_in_bound);
	} else if (x == 0) {
	  SET_NEIGHBOR26_BOUND(0, 1, 1, 1, 1, 1, 
			       0, 1, 1, 0, 0, 1,
			       1, 0, 1, 1, 1, 1, 
			       0, 1, 1, 0, 1, 0,
			       0, 1, is_in_bound, 17, n_in_bound);
	} else if (x > cwidth) {
	  SET_NEIGHBOR26_BOUND(1, 0, 0, 0, 0, 0, 
			       1, 0, 0, 1, 1, 0,
			       0, 1, 0, 0, 0, 0, 
			       1, 0, 0, 1, 0, 1,
			       1, 0, is_in_bound, 9, n_in_bound);
	} else if (x == cwidth) {
	  SET_NEIGHBOR26_BOUND(1, 0, 1, 1, 1, 1, 
			       1, 0, 0, 1, 1, 0,
			       0, 1, 1, 1, 1, 1, 
			       1, 0, 0, 1, 0, 1,
			       1, 0, is_in_bound, 17, n_in_bound);
	}
      }
    }
    break;
  }

  return n_in_bound;
}

int Stack_Neighbor_Bound_Test_S(int n_nbr, int cwidth, int cheight, 
			       int cdepth, int x, int y, int z, 
				int *is_in_bound)
{
  int n_in_bound = n_nbr;
  
  switch (n_nbr) {
  case 4:
    if ((x > 0) && (x < cwidth) && (y > 0) && (y < cheight) && 
	(z >= 0) && (z <= cdepth)) { 
      /* all neighbors are in x bound */
      return n_nbr;
    }

    if ((x < 0) || (x > cwidth) || (y < 0) || (y > cheight) ||
	(z < 0) || (z > cdepth)) {
      /* all neighbors are out bound */
      return 0;
    } else { /* some neighbors are in bound and some are not */
      SET_NEIGHBOR4_BOUND(1, 1, 1, 1, is_in_bound, 4, n_in_bound);
      if (x == 0) {
	is_in_bound[0] = 0;
	n_in_bound--;
      }
      if (x == cwidth) {
	is_in_bound[1] = 0;
	n_in_bound--;
      }
      if (y == 0) {
	is_in_bound[2] = 0;
	n_in_bound--;
      }
      if (y == cheight) {
	is_in_bound[3] = 0;
	n_in_bound--;
      }
    }
     
    break;
  case 8:
    if ((x > 0) && (x < cwidth) && (y > 0) && (y < cheight) && 
	(z >= 0) && (z <= cdepth)) {
      /* all neighbors are in x bound */
      return n_nbr;
    }
    
    if ((x < 0) || (x > cwidth) || (y < 0) || (y > cheight) ||
	(z < 0) || (z > cdepth)) {
      /* all neighbors are out bound */
      return 0;
    } else { /* some neighbors are in bound and some are not */
      SET_NEIGHBOR8_BOUND(1, 1, 1, 1, 1, 1, 1, 1, is_in_bound, 8, n_in_bound);
      if (x == 0) {
	is_in_bound[0] = 0;
	is_in_bound[4] = 0;
	is_in_bound[7] = 0;
	n_in_bound -= 3;
      }
      if (x == cwidth) {
	is_in_bound[1] = 0;
	is_in_bound[5] = 0;
	is_in_bound[6] = 0;
	n_in_bound -= 3;	
      }
      if (y == 0) {
	is_in_bound[2] = 0;
	if (is_in_bound[4] == 1) {
	  is_in_bound[4] = 0;
	} else {
	  n_in_bound++;
	}
	is_in_bound[6] = 0;
	n_in_bound -= 3;
      }
      if (y == cheight) {
	is_in_bound[3] = 0;
	if (is_in_bound[5] == 1) {
	  is_in_bound[5] = 0;
	} else {
	  n_in_bound++;
	}
	is_in_bound[7] = 0;
	n_in_bound -= 3;
      }      
    }
    break;
  case 6:
    if ((x > 0) && (x < cwidth) && (y > 0) && (y < cheight) && 
	(z > 0) && (z < cdepth)) { 
      /* all neighbors are in x bound */
      return n_nbr;
    }
    
    if ((x < 0) || (x > cwidth) || (y < 0) || (y > cheight) ||
	(z < 0) || (z > cdepth)) {
      /* all neighbors are out bound */
      return 0;
    }

    SET_NEIGHBOR6_BOUND(1, 1, 1, 1, 1, 1, is_in_bound, 6, n_in_bound);
    
    if (x == 0) {
      is_in_bound[0] = 0;
      n_in_bound--;
    }
    
    if (x == cwidth) {
      is_in_bound[1] = 0;
      n_in_bound--;
    }
    
    if (y == 0) {
      is_in_bound[2] = 0;
      n_in_bound--;
    }

    if (y == cheight) {
      is_in_bound[3] = 0;
      n_in_bound--;
    }

    if (z == 0) {
      is_in_bound[4] = 0;
      n_in_bound--;
    }

    if (z == cdepth) {
      is_in_bound[5] = 0;
      n_in_bound--;
    }
    break;
  case 10:
    if ((x > 0) && (x < cwidth) && (y > 0) && (y < cheight) && 
	(z > 0) && (z < cdepth)) { 
      /* all neighbors are in x bound */
      return n_nbr;
    }

    if ((x < 0) || (x > cwidth) || (y < 0) || (y > cheight) ||
	(z < 0) || (z > cdepth)) {
      /* all neighbors are out bound */
      return 0;
    }

    SET_NEIGHBOR10_BOUND(1, 1, 1, 1, 1, 1,
			 1, 1, 1, 1, is_in_bound, 18, n_in_bound);

    if (x == 0) {
      is_in_bound[0] = 0;
      is_in_bound[6] = 0;
      is_in_bound[9] = 0;
      n_in_bound -= 3;
    }
    
    if (x == cwidth) {
      is_in_bound[1] = 0;
      is_in_bound[7] = 0;
      is_in_bound[8] = 0;
      n_in_bound -= 3;
    }
    
    if (y == 0) {
      is_in_bound[2] = 0;
      if (is_in_bound[6] == 1) {
	is_in_bound[6] = 0;
      } else {
	n_in_bound++;
      }
      if (is_in_bound[8] == 1) {
	is_in_bound[8] = 0;
      } else {
	n_in_bound++;
      }
      n_in_bound -= 3;
    }

    if (y == cheight) {
      is_in_bound[3] = 0;
      if (is_in_bound[7] == 1) {
	is_in_bound[7] = 0;
      } else {
	n_in_bound++;
      }
      if (is_in_bound[9] == 1) {
	is_in_bound[9] = 0;
      } else {
	n_in_bound++;
      }
      n_in_bound -= 3;
    }

    if (z == 0) {
      is_in_bound[4] = 0;
      n_in_bound -= 1;
    }

    if (z == cdepth) {
      is_in_bound[5] = 0;
      n_in_bound -= 1;
    }

    break;
  case 18:
    if ((x > 0) && (x < cwidth) && (y > 0) && (y < cheight) && 
	(z > 0) && (z < cdepth)) { 
      /* all neighbors are in x bound */
      return n_nbr;
    }

    if ((x < 0) || (x > cwidth) || (y < 0) || (y > cheight) ||
	(z < 0) || (z > cdepth)) {
      /* all neighbors are out bound */
      return 0;
    }

    SET_NEIGHBOR18_BOUND(1, 1, 1, 1, 1, 1,
			 1, 1, 1, 1, 1, 1, 
			 1, 1, 1, 1, 1, 1, is_in_bound, 18, n_in_bound);

    if (x == 0) {
      is_in_bound[0] = 0;
      is_in_bound[6] = 0;
      is_in_bound[9] = 0;
      is_in_bound[10] = 0;
      is_in_bound[13] = 0;
      n_in_bound -= 5;
    }
    
    if (x == cwidth) {
      is_in_bound[1] = 0;
      is_in_bound[7] = 0;
      is_in_bound[8] = 0;
      is_in_bound[11] = 0;
      is_in_bound[12] = 0;
      n_in_bound -= 5;
    }
    
    if (y == 0) {
      is_in_bound[2] = 0;
      if (is_in_bound[6] == 1) {
	is_in_bound[6] = 0;
      } else {
	n_in_bound++;
      }
      if (is_in_bound[8] == 1) {
	is_in_bound[8] = 0;
      } else {
	n_in_bound++;
      }
      is_in_bound[14] = 0;
      is_in_bound[17] = 0;
      n_in_bound -= 5;
    }

    if (y == cheight) {
      is_in_bound[3] = 0;
      if (is_in_bound[7] == 1) {
	is_in_bound[7] = 0;
      } else {
	n_in_bound++;
      }
      if (is_in_bound[9] == 1) {
	is_in_bound[9] = 0;
      } else {
	n_in_bound++;
      }
      is_in_bound[15] = 0;
      is_in_bound[16] = 0;
      n_in_bound -= 5;
    }

    if (z == 0) {
      is_in_bound[4] = 0;
      if (is_in_bound[10] == 1) {
	is_in_bound[10] = 0;
      } else {
	n_in_bound++;
      }
      if (is_in_bound[12] == 1) {
	is_in_bound[12] = 0;
      } else {
	n_in_bound++;
      }
      if (is_in_bound[14] == 1) {
	is_in_bound[14] = 0;
      } else {
	n_in_bound++;
      }
      if (is_in_bound[16] == 1) {
	is_in_bound[16] = 0;
      } else {
	n_in_bound++;
      }
      n_in_bound -= 5;
    }

    if (z == cdepth) {
      is_in_bound[5] = 0;
      if (is_in_bound[11] == 1) {
	is_in_bound[11] = 0;
      } else {
	n_in_bound++;
      }
      if (is_in_bound[13] == 1) {
	is_in_bound[13] = 0;
      } else {
	n_in_bound++;
      }
      if (is_in_bound[15] == 1) {
	is_in_bound[15] = 0;
      } else {
	n_in_bound++;
      }
      if (is_in_bound[17] == 1) {
	is_in_bound[17] = 0;
      } else {
	n_in_bound++;
      }
      n_in_bound -= 5;
    }

    break;

  case 26:
    if ((x > 0) && (x < cwidth) && (y > 0) && (y < cheight) && 
	(z > 0) && (z < cdepth)) { 
      /* all neighbors are in bound */
      return n_nbr;
    }

    if ((x < 0) || (x > cwidth) || (y < 0) || (y > cheight) ||
	(z < 0) || (z > cdepth)) {
      /* all neighbors are out of bound */
      return 0;
    }

    SET_NEIGHBOR26_BOUND(1, 1, 1, 1, 1, 1,
			 1, 1, 1, 1, 1, 1, 
			 1, 1, 1, 1, 1, 1, 
			 1, 1, 1, 1, 1, 1,
			 1, 1, is_in_bound, 26, n_in_bound);

    if (x == 0) {
      is_in_bound[0] = 0;
      is_in_bound[6] = 0;
      is_in_bound[9] = 0;
      is_in_bound[10] = 0;
      is_in_bound[13] = 0;
      is_in_bound[18] = 0;
      is_in_bound[21] = 0;
      is_in_bound[23] = 0;
      is_in_bound[24] = 0;
      n_in_bound -= 9;
    }
    
    if (x == cwidth) {
      is_in_bound[1] = 0;
      is_in_bound[7] = 0;
      is_in_bound[8] = 0;
      is_in_bound[11] = 0;
      is_in_bound[12] = 0;
      is_in_bound[19] = 0;
      is_in_bound[20] = 0;
      is_in_bound[22] = 0;
      is_in_bound[25] = 0;
      n_in_bound -= 9;
    }
    
    if (y == 0) {
      is_in_bound[2] = 0;
      if (is_in_bound[6] == 1) {
	is_in_bound[6] = 0;
      } else {
	n_in_bound++;
      }
      if (is_in_bound[8] == 1) {
	is_in_bound[8] = 0;
      } else {
	n_in_bound++;
      }
      is_in_bound[14] = 0;
      is_in_bound[17] = 0;
      if (is_in_bound[18] == 1) {
	is_in_bound[18] = 0;
      } else {
	n_in_bound++;
      }
      if (is_in_bound[21] == 1) {
	is_in_bound[21] = 0;
      } else {
	n_in_bound++;
      }
      if (is_in_bound[22] == 1) {
	is_in_bound[22] = 0;
      } else {
	n_in_bound++;
      }
      if (is_in_bound[25] == 1) {
	is_in_bound[25] = 0;
      } else {
	n_in_bound++;
      }
      n_in_bound -= 9;
    }

    if (y == cheight) {
      is_in_bound[3] = 0;
      if (is_in_bound[7] == 1) {
	is_in_bound[7] = 0;
      } else {
	n_in_bound++;
      }
      if (is_in_bound[9] == 1) {
	is_in_bound[9] = 0;
      } else {
	n_in_bound++;
      }
      is_in_bound[15] = 0;
      is_in_bound[16] = 0;
      if (is_in_bound[19] == 1) {
	is_in_bound[19] = 0;
      } else {
	n_in_bound++;
      }
      if (is_in_bound[20] == 1) {
	is_in_bound[20] = 0;
      } else {
	n_in_bound++;
      }
      if (is_in_bound[23] == 1) {
	is_in_bound[23] = 0;
      } else {
	n_in_bound++;
      }
      if (is_in_bound[24] == 1) {
	is_in_bound[24] = 0;
      } else {
	n_in_bound++;
      }
      n_in_bound -= 9;
    }

    if (z == 0) {
      is_in_bound[4] = 0;
      if (is_in_bound[10] == 1) {
	is_in_bound[10] = 0;
      } else {
	n_in_bound++;
      }
      if (is_in_bound[12] == 1) {
	is_in_bound[12] = 0;
      } else {
	n_in_bound++;
      }
      if (is_in_bound[14] == 1) {
	is_in_bound[14] = 0;
      } else {
	n_in_bound++;
      }
      if (is_in_bound[16] == 1) {
	is_in_bound[16] = 0;
      } else {
	n_in_bound++;
      }
      if (is_in_bound[18] == 1) {
	is_in_bound[18] = 0;
      } else {
	n_in_bound++;
      }
      if (is_in_bound[20] == 1) {
	is_in_bound[20] = 0;
      } else {
	n_in_bound++;
      }
      if (is_in_bound[22] == 1) {
	is_in_bound[22] = 0;
      } else {
	n_in_bound++;
      }
      if (is_in_bound[24] == 1) {
	is_in_bound[24] = 0;
      } else {
	n_in_bound++;
      }
      n_in_bound -= 9;
    }

    if (z == cdepth) {
      is_in_bound[5] = 0;
      if (is_in_bound[11] == 1) {
	is_in_bound[11] = 0;
      } else {
	n_in_bound++;
      }
      if (is_in_bound[13] == 1) {
	is_in_bound[13] = 0;
      } else {
	n_in_bound++;
      }
      if (is_in_bound[15] == 1) {
	is_in_bound[15] = 0;
      } else {
	n_in_bound++;
      }
      if (is_in_bound[17] == 1) {
	is_in_bound[17] = 0;
      } else {
	n_in_bound++;
      }
      if (is_in_bound[19] == 1) {
	is_in_bound[19] = 0;
      } else {
	n_in_bound++;
      }
      if (is_in_bound[21] == 1) {
	is_in_bound[21] = 0;
      } else {
	n_in_bound++;
      }
      if (is_in_bound[23] == 1) {
	is_in_bound[23] = 0;
      } else {
	n_in_bound++;
      }
      if (is_in_bound[25] == 1) {
	is_in_bound[25] = 0;
      } else {
	n_in_bound++;
      }
      n_in_bound -= 9;
    }
    break;
  }

  return n_in_bound;
}

int Stack_Neighbor_Bound_Test_I(int conn, int width, int height, int depth, 
				size_t index, int is_in_bound[])
{
  int x, y, z;
  int area = width * height;
  STACK_UTIL_COORD(index, width, area, x, y, z);
  return Stack_Neighbor_Bound_Test_S(conn, width - 1, height - 1, depth - 1,
				     x, y, z, is_in_bound);
}

/* Stack_Neighbor_Sampling(): sample the neighbors of a voxel;
 *
 * Args: stack - stack for sampling;
 *       n_nbr - neighborhood option;
 *       x, y, z - central voxel;
 *       out_bound_value - the value of the voxels out of bound;
 *       s - the array to store the result.
 *
 * Return: void.
 */
#define STACK_NEIGHBOR_SAMPLING(stack_array)		\
  if (n_in_bound == n_nbr) {				\
    for (i = 0; i < n_nbr; i++) {			\
      s[i] = stack_array[center + neighbor[i]];	\
    }							\
  } else {						\
    for (i = 0; i < n_nbr; i++) {			\
      if (is_in_bound[i] == TRUE) {			\
	s[i] = stack_array[center + neighbor[i]];	\
      } else {						\
	s[i] = out_bound_value;				\
      }							\
    }							\
  }

void Stack_Neighbor_Sampling(const Stack *stack, int n_nbr, 
			     int x, int y, int z, double out_bound_value, 
			     double s[])
{
#ifdef _MSC_VER
  int *neighbor = (int*)malloc(sizeof(int)*n_nbr);
#else
  int neighbor[n_nbr];
#endif
  Stack_Neighbor_Offset(n_nbr, stack->width, stack->height, neighbor);
  
#ifdef _MSC_VER
  int *is_in_bound = (int*)malloc(sizeof(int)*n_nbr);
#else
  int is_in_bound[n_nbr];
#endif
  int n_in_bound = Stack_Neighbor_Bound_Test(n_nbr, stack->width - 1, 
					     stack->height - 1, stack->depth - 1, 
					     x, y, z, is_in_bound);

  int i;

  if (n_in_bound == 0) {
    for (i = 0; i < n_nbr; i++) {
      s[i] = out_bound_value;
    }
  } else {
    int center = z * stack->height * stack->width + y * stack->width + x;
    Image_Array ima;
    ima.array = stack->array;
    STACK_ARRAY_OPERATION(ima, STACK_NEIGHBOR_SAMPLING);
  }

#ifdef _MSC_VER
  free(neighbor);
  free(is_in_bound);
#endif
}

#define STACK_NEIGHBOR_SAMPLING_I(stack_array)			\
  if (n_in_bound == n_nbr) {					\
    for (i = 0; i < n_nbr; i++) {				\
      s[i] = stack_array[center + neighbor[i]];		\
    }								\
  } else {							\
    for (i = 0; i < n_nbr; i++) {				\
      if (is_in_bound[i] == TRUE) {				\
	s[i] = stack_array[center + neighbor[i]];		\
      } else {							\
	s[i] = out_bound_value;					\
      }								\
    }								\
  }

void Stack_Neighbor_Sampling_I(const Stack *stack, int n_nbr, 
			       size_t index, double out_bound_value, double s[])
{
#ifdef _MSC_VER
  int *neighbor = (int*)malloc(sizeof(int)*n_nbr);
#else
  int neighbor[n_nbr];
#endif
  Stack_Neighbor_Offset(n_nbr, stack->width, stack->height, neighbor);
  
#ifdef _MSC_VER
  int *is_in_bound = (int*)malloc(sizeof(int)*n_nbr);
#else
  int is_in_bound[n_nbr];
#endif
  int n_in_bound = Stack_Neighbor_Bound_Test_I(n_nbr, stack->width, 
					       stack->height, stack->depth, 
					       index, is_in_bound);

  int i;

  if (n_in_bound == 0) {
    for (i = 0; i < n_nbr; i++) {
      s[i] = out_bound_value;
    }
  } else {
    int center = index;
    Image_Array ima;
    ima.array = stack->array;
    STACK_ARRAY_OPERATION(ima, STACK_NEIGHBOR_SAMPLING_I);
  }

#ifdef _MSC_VER
  free(neighbor);
  free(is_in_bound);
#endif
}

#define STACK_NEIGHBOR_MIN(stack_array)		\
  min_value = stack_array[center];				\
  if (n_in_bound == n_nbr) {					\
    for (i = 0; i < n_nbr; i++) {				\
      if (min_value > stack_array[center + neighbor[i]]) {	\
        min_value = stack_array[center + neighbor[i]];		\
      }								\
    }								\
  } else {							\
    for (i = 0; i < n_nbr; i++) {				\
      if (is_in_bound[i] == TRUE) {				\
        if (min_value > stack_array[center + neighbor[i]]) {	\
          min_value = stack_array[center + neighbor[i]];	\
        }							\
      }								\
    }								\
  }

double Stack_Neighbor_Min(const Stack *stack, int n_nbr, int x, int y, int z)
{
#ifdef _MSC_VER
  int *neighbor = (int*)malloc(sizeof(int)*n_nbr);
#else
  int neighbor[n_nbr];
#endif
  Stack_Neighbor_Offset(n_nbr, stack->width, stack->height, neighbor);
  
#ifdef _MSC_VER
  int *is_in_bound = (int*)malloc(sizeof(int)*n_nbr);
#else
  int is_in_bound[n_nbr];
#endif
  int n_in_bound = Stack_Neighbor_Bound_Test(n_nbr, stack->width - 1, 
					     stack->height - 1, stack->depth - 1, 
					     x, y, z, is_in_bound);

  int i;
  double min_value;

  if (n_in_bound == 0) {
    min_value = 0.0;
  } else {
    int center = z * stack->height * stack->width + y * stack->width + x;
    Image_Array ima;
    ima.array = stack->array;
    switch (stack->kind) {
    case GREY:
      STACK_NEIGHBOR_MIN(ima.array8);
      break;
    case GREY16:
      STACK_NEIGHBOR_MIN(ima.array16);
      break;
    case FLOAT32:
      STACK_NEIGHBOR_MIN(ima.array32);
      break;
    case FLOAT64:
      STACK_NEIGHBOR_MIN(ima.array64);
      break;
    default:
      TZ_ERROR(ERROR_DATA_TYPE);
      break;
    }
  }

#ifdef _MSC_VER
  free(neighbor);
  free(is_in_bound);
#endif
  
  return min_value;
}

#define STACK_NEIGHBOR_MEAN(stack_array)			\
  mean_value = stack_array[center];				\
  if (n_in_bound == n_nbr) {					\
    for (i = 0; i < n_nbr; i++) {				\
      mean_value += stack_array[center + neighbor[i]];		\
    }								\
  } else {							\
    for (i = 0; i < n_nbr; i++) {				\
      if (is_in_bound[i] == TRUE) {				\
	mean_value += stack_array[center + neighbor[i]];	\
      }								\
    }								\
  }

double Stack_Neighbor_Mean(const Stack *stack, int n_nbr, int x, int y, int z)
{
#ifdef _MSC_VER
  int *neighbor = (int*)malloc(sizeof(int)*n_nbr);
#else
  int neighbor[n_nbr];
#endif
  Stack_Neighbor_Offset(n_nbr, stack->width, stack->height, neighbor);
  
#ifdef _MSC_VER
  int *is_in_bound = (int*)malloc(sizeof(int)*n_nbr);
#else
  int is_in_bound[n_nbr];
#endif
  int n_in_bound = Stack_Neighbor_Bound_Test(n_nbr, stack->width - 1, 
					     stack->height - 1, stack->depth - 1, 
					     x, y, z, is_in_bound);

  int i;
  double mean_value;

  if (n_in_bound == 0) {
    mean_value = 0.0;
  } else {
    int center = z * stack->height * stack->width + y * stack->width + x;
    Image_Array ima;
    ima.array = stack->array;
    switch (stack->kind) {
    case GREY:
      STACK_NEIGHBOR_MEAN(ima.array8);
      break;
    case GREY16:
      STACK_NEIGHBOR_MEAN(ima.array16);
      break;
    case FLOAT32:
      STACK_NEIGHBOR_MEAN(ima.array32);
      break;
    case FLOAT64:
      STACK_NEIGHBOR_MEAN(ima.array64);
      break;
    default:
      TZ_ERROR(ERROR_DATA_TYPE);
      break;
    }
  }

#ifdef _MSC_VER
  free(neighbor);
  free(is_in_bound);
#endif

  return mean_value / (n_in_bound + 1);
}

#define STACK_NEIGHBOR_SET_I(stack_array)			\
  if (n_in_bound == n_nbr) {					\
    for (i = 0; i < n_nbr; i++) {				\
      stack_array[center + neighbor[i]] = value;		\
    }								\
  } else {							\
    for (i = 0; i < n_nbr; i++) {				\
      if (is_in_bound[i] == TRUE) {				\
	stack_array[center + neighbor[i]] = value;		\
      }							\
    }								\
  }

void Stack_Neighbor_Set_I(const Stack *stack, int n_nbr, size_t index, 
    double value)
{
  int neighbor[26];
  Stack_Neighbor_Offset(n_nbr, stack->width, stack->height, neighbor);
  
  int is_in_bound[26];
  int n_in_bound = Stack_Neighbor_Bound_Test_I(n_nbr, stack->width, 
					       stack->height, stack->depth, 
					       index, is_in_bound);

  int i;

  int center = index;
  Image_Array ima;
  ima.array = stack->array;
  STACK_ARRAY_OPERATION(ima, STACK_NEIGHBOR_SET_I);
}

/*
int Stack_Neighbor_Pattern_26(const Stack *stack, int *neighbor_offset, 
    size_t index)
{
  int is_in_bound[26] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
  };

  int i;
  int x, y, z;
  Stack_Util_Coord(index, stack->width, stack->height, &x, &y, &z);
  if (z == 0) {
    for (i = 0; i < 9; ++i) {
      is_in_bound[0] = 0;
    }
  }
  
  if (z == stack->depth - 1) {
    for (i = 17; i < 26; ++i) {
      is_in_bound[0] = 0;
    }
  }
  
  if (y == 0) {
    is_in_bound[0] = 0;
    is_in_bound[1] = 0;
    is_in_bound[2] = 0;
    is_in_bound[0] = 0;
  } 
  
  if (y == stack->height - 1) {
    for (i = 0; i < 8; ++i) {
      is_in_bound[i * 3 + 2] = 0;
    }
  }

  if (x == 0) {
    for (i = 0; i < 9; ++i) {
      is_in_bound[i * 3] = 0;
    }
  }
  
  if (x == stack->width - 1) {
    for (i = 0; i < 9; ++i) {
      is_in_bound[i * 3 + 2] = 0;
    }
  }

  int pattern = 0;
  if (n_in_bound == 26) {
    for (i = 0; i < 26; ++i) {
      if (stack->array[i]) {
        pattern += 1 << i;
      }
    }
  } else {
    for (i = 0; i < 26; ++i) {
      if (is_in_bound[i] && stack->array[i]) {
        pattern += 1 << i;
      }
    }
  }

  return pattern;
}
*/

void Stack_Label_Neighbor_26(Stack *stack, int index)
{
  TZ_ASSERT(stack->kind == GREY && stack->width == 3 && stack->height == 3
      && stack->depth == 3, "Invalid stack attributes");

  int offset = 0;
  while ((index > 0) && (offset < 27)) {
    if (index % 2 > 0) {
      stack->array[offset] = 1;
    }
    ++offset;
    index >>= 1;
  }
}
