/* tz_stack_lib.c
 * 07-Dec-2007 Initial write: Ting Zhao
 */

#define STACK_ORIENTATION_DF 13

/* 27 types of location (8 vertices, 12 edges, 6 faces, 1 internal */
const static int scan_mask[27][STACK_ORIENTATION_DF] = {
  /* 1-2 1-3 1-5 1-4 2-3 1-6 2-5 1-7 3-5 1-8 2-7 3-6 4-5 */
  {   1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },   /* 0 */
  {   1,  1,  1,  1,  0,  1,  0,  1,  0,  1,  0,  0,  0 },   /* 1 */
  {   0,  1,  1,  0,  1,  0,  1,  1,  0,  0,  1,  0,  0 },   /* 2 */
  {   1,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0 },   /* 3 */
  {   0,  0,  1,  0,  0,  0,  1,  0,  1,  0,  0,  0,  1 },   /* 4 */
  {   1,  1,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0 },   /* 5 */
  {   0,  1,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0 },   /* 6 */
  {   1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },   /* 7 */
  {   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },   /* 8 */
  {   1,  1,  1,  1,  1,  1,  1,  1,  0,  1,  1,  0,  0 },   /* 9  (1|2) */
  {   1,  0,  1,  0,  0,  1,  1,  0,  1,  0,  0,  1,  1 },   /* 10 (3|4) */
  {   1,  1,  0,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0 },   /* 11 (5|6) */
  {   1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },   /* 12 (7|8) */
  {   1,  1,  1,  1,  0,  1,  0,  1,  1,  1,  0,  1,  0 },   /* 13 (1|3) */
  {   0,  1,  1,  0,  1,  0,  1,  1,  1,  0,  1,  0,  1 },   /* 14 (2|4) */
  {   1,  1,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0 },   /* 15 (5|7) */
  {   0,  1,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0 },   /* 16 (6|8) */
  {   1,  1,  1,  1,  0,  1,  0,  1,  0,  1,  0,  0,  0 },   /* 17 (1|5) */
  {   0,  1,  1,  0,  1,  0,  1,  1,  0,  0,  1,  0,  0 },   /* 18 (2|6) */
  {   1,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0 },   /* 19 (3|7) */
  {   0,  0,  1,  0,  0,  0,  1,  0,  1,  0,  0,  0,  1 },   /* 20 (4|8) */
  {   1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },   /* 21 (1|2|3|4) */
  {   1,  1,  0,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0 },   /* 22 (5|6|7|8) */
  {   1,  1,  1,  1,  0,  1,  0,  1,  1,  1,  0,  1,  0 },   /* 23 (1|3|5|7) */
  {   0,  1,  1,  0,  1,  0,  1,  1,  1,  0,  1,  0,  1 },   /* 24 (2|4|6|8) */
  {   1,  1,  1,  1,  1,  1,  1,  1,  0,  1,  1,  0,  0 },   /* 25 (1|2|5|6) */
  {   1,  0,  1,  0,  0,  1,  1,  0,  1,  0,  0,  1,  1 }    /* 26 (3|4|7|8) */
  /* 1-2 1-3 1-5 1-4 2-3 1-6 2-5 1-7 3-5 1-8 2-7 3-6 4-5 */
};

/* for depth 1 */
const static int scan_mask_depth[STACK_ORIENTATION_DF] =
  /* 1-2 1-3 1-5 1-4 2-3 1-6 2-5 1-7 3-5 1-8 2-7 3-6 4-5 */
  {   1,  1,  0,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0 };

/* for height 1 */
const static int scan_mask_height[STACK_ORIENTATION_DF] =
  /* 1-2 1-3 1-5 1-4 2-3 1-6 2-5 1-7 3-5 1-8 2-7 3-6 4-5 */
  {   1,  0,  1,  0,  0,  1,  1,  0,  0,  0,  0,  0,  0 };

/* for width 1 */
const static int scan_mask_width[STACK_ORIENTATION_DF] =
  /* 1-2 1-3 1-5 1-4 2-3 1-6 2-5 1-7 3-5 1-8 2-7 3-6 4-5 */
  {   0,  1,  1,  0,  0,  0,  0,  1,  1,  0,  0,  0,  0 };

static void init_scan_array(int width, int height, int *neighbor)
{
  int planeOffset = width * height;

  neighbor[0] = 1; /* x-directon */
  neighbor[1] = width; /* y-direction */
  neighbor[2] = planeOffset; /* z-direction */
  neighbor[3] = width + 1;; /* x-y diagonal */
  neighbor[4] = width - 1; /* x-y counterdiagonal */
  neighbor[5] = planeOffset + 1; /* x-z diagonal */
  neighbor[6] = planeOffset - 1; /* x-z counter diaagonal */
  neighbor[7] = planeOffset + width; /* y-z diagonal */
  neighbor[8] = planeOffset - width; /* y-z counter diaagonal */
  neighbor[9] = planeOffset + width + 1; /* x-y-z diagonal */
  neighbor[10] = planeOffset + width - 1; /* x-y-z diagonal -x*/
  neighbor[11] = planeOffset - width + 1; /* x-y-z diagonal -y*/
  neighbor[12] = planeOffset - width - 1; /* x-y-z diagonal -x -y*/
}

static const int* get_scan_array_mask(int id)
{
  return scan_mask[id];
}

static void copy_scan_array_mask(int id, int *mask)
{
  memcpy(mask, get_scan_array_mask(id), sizeof(int) * 13);
}

static void scan_array_mask_depth(int *mask)
{
  int i;
  for (i = 0; i < STACK_ORIENTATION_DF; i++) {
    mask[i] = mask[i] && scan_mask_depth[i];
  }
}

static void scan_array_mask_height(int *mask)
{
  int i;
  for (i = 0; i < STACK_ORIENTATION_DF; i++) {
    mask[i] = mask[i] && scan_mask_height[i];
  }
}

static void scan_array_mask_width(int *mask)
{
  int i;
  for (i = 0; i < STACK_ORIENTATION_DF; i++) {
    mask[i] = mask[i] && scan_mask_width[i];
  }
}

#define ARRAY_CMP(array, array_out, offset, c, nboffset, neighbor, option) \
  nboffset = offset + neighbor[c];					\
  if (array[nboffset] > 0) {						\
    if (array[offset] < array[nboffset]) {				\
      array_out[offset] = 0;						\
    } else if (array[offset] > array[nboffset]) {			\
      array_out[nboffset] = 0;						\
    } else {								\
      switch (option) {							\
      case STACK_LOCMAX_CENTER:						\
	array_out[nboffset] = 0;					\
	break;								\
      case STACK_LOCMAX_NEIGHBOR:					\
	array_out[offset] = 0;						\
	break;								\
      case STACK_LOCMAX_NONFLAT:					\
	array_out[offset] = 0;						\
	array_out[nboffset] = 0;					\
	break;								\
      case STACK_LOCMAX_ALTER1:						\
	if ((array_out[offset] == 1) &&					\
	    (array_out[nboffset] == 1)) {				\
	  array_out[offset] = 0;					\
	}								\
	break;								\
      case STACK_LOCMAX_ALTER2:						\
	if ((array_out[offset] == 1) &&					\
	    (array_out[nboffset] == 1)) {				\
	  array_out[nboffset] = 0;					\
	}								\
	break;								\
      case STACK_LOCMAX_SINGLE:						\
	if ((array_out[offset] == 1) &&					\
	    (array_out[nboffset] == 1)) {				\
	  array_out[nboffset] = 0;					\
	} else {							\
	  array_out[offset] = 0;					\
	  array_out[nboffset] = 0;					\
	}								\
	break;								\
      default:								\
	/* do nothing */						\
	break;								\
      }									\
    }									\
  } else  {								\
    array_out[nboffset] = 0;						\
  }

static int boundary_offset(int width, int height, int depth, int id,
			   int *offset)
{
  int i, j;
  int n = 0;
  int area = width * height;
  int volume = area * depth;

  int cwidth = width - 1;
  int carea = area - width;
  int cvolume = volume - area;

  int start = 0;

  switch (id) {
  case 1:
    offset[0] = 0;
    n = 1;
    break;
  case 2:
    offset[0] = cwidth;
    n = 1;
    break;
  case 3:
    offset[0] = carea;
    n = 1;
    break;
  case 4:
    offset[0] = carea + cwidth;
    n = 1;
    break;
  case 5:
    offset[0] = cvolume;
    n = 1;
    break;
  case 6:
    offset[0] = cvolume + cwidth;
    n = 1;
    break;
  case 7:
    offset[0] = cvolume + carea;
    n = 1;
    break;
  case 8:
    offset[0] = volume - 1;
    n = 1;
    break;
  case 9:
    for (i = 1; i < cwidth; i++) {
      offset[n] = i;
      n++;
    }
    break;
  case 10:
    start = carea;
    for (i = 1; i < cwidth; i++) {
      offset[n] = start + i;
      n++;
    }
    break;
  case 11:
    start = cvolume;
    for (i = 1; i < cwidth; i++) {
      offset[n] = start + i;
      n++;
    }
    break;
  case 12:
    start = cvolume + carea;
    for (i = 1; i < cwidth; i++) {
      offset[n] = start + i;
      n++;
    }
    break;
  case 13:
    for (i = width; i < carea; i += width) {
      offset[n] = i;
      n++;
    }
    break;
  case 14:
    start = cwidth;
    for (i = width; i < carea; i += width) {
      offset[n] = start + i;
      n++;
    }
    break;
  case 15:
    start = cvolume;
    for (i = width; i < carea; i += width) {
      offset[n] = start + i;
      n++;
    }
    break;
  case 16:
    start = cvolume + cwidth;
    for (i = width; i < carea; i += width) {
      offset[n] = start + i;
      n++;
    }
    break;
  case 17:
    for (i = area; i < cvolume; i += area) {
      offset[n] = i;
      n++;
    }
    break;
  case 18:
    start = cwidth;
    for (i = area; i < cvolume; i += area) {
      offset[n] = start + i;
      n++;
    }
    break;
  case 19:					
    start = carea;
    for (i = area; i < cvolume; i += area) {
      offset[n] = start + i;
      n++;
    }
    break;
  case 20:					
    start = carea + cwidth;
    for (i = area; i < cvolume; i += area) {
      offset[n] = start + i;
      n++;
    }
    break;
  case 21:
    for (j = width; j < carea; j += width) {
      for (i = 1; i < cwidth; i++) {
	offset[n] = i + j;
	n++;
      }
    }
    break;
  case 22:
    start = cvolume;
    for (j = width; j < carea; j += width) {
      for (i = 1; i < cwidth; i++) {
	offset[n] = start + i + j;
	n++;
      }
    }
    break;
  case 23:
    for (j = area; j < cvolume; j += area) {
      for (i = width; i < carea; i += width) {
	offset[n] = i + j;
	n++;
      }
    }
    break;
  case 24:
    start = cwidth;
    for (j = area; j < cvolume; j += area) {
      for (i = width; i < carea; i += width) {
	offset[n] = start + i + j;
	n++;
      }
    }
    break;
  case 25:
    for (j = area; j < cvolume; j += area) {
      for (i = 1; i < cwidth; i ++) {
	offset[n] = i + j;
	n++;
      }
    }
    break;
  case 26:
    start = carea;
    for (j = area; j < cvolume; j += area) {
      for (i = 1; i < cwidth; i ++) {
	offset[n] = start + i + j;
	n++;
      }
    }
    break;
  default:
    /* do nothing */
    break;
  }

  return n;
}
