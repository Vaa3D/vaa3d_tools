/* private/tz_image_code.c
 *
 * Initial write: Ting Zhao
 */

static int chk_width;
static int chk_height;
static int chk_depth;
static int chk_iscon1 = 0;
static int ccon_2d = 0;
static int cwidth = 0;    /* Width of current image         */
static int cheight = 0;   /* Height of current image        */
static int cdepth = 0;
static int carea = 0;
static int cnpixel = 0;

/* init_imginfo(): initialize  image information.
 *
 * Args: width - image width;
 *       height - image height;
 *       depth - image depth. It is 1 for a 2D image.
 *
 * Return: void
 */
static void init_imginfo(int width, int height, int depth)
{
  assert((width > 0) && (height > 0) && (depth > 0));

  cwidth = width;
  cheight = height;
  cdepth = depth;
  carea = width * height;
  cnpixel = carea * depth;

  chk_width = width - 1;
  chk_height = height - 1;
  chk_depth = depth - 1;
}

/* boundary_pixels_2d(): set boundary pixel flags.
 *
 * Args: p - pixel index.
 *
 * Return: an array of flags. 
 *         1 means it is inside the image and 0 means outside.
 */
static inline int *boundary_pixels_2d(int p)
{ 
  static int bound[8];
  int x, xn, xp;
  int y, yn, yp;

  if (chk_depth > 0) {
    p = p%carea;
  }

  y = p/cwidth;
  x = p%cwidth;

  yn = (y > 0);
  xn = (x > 0);
  xp = (x < chk_width);
  yp = (y < chk_height);
  if (chk_iscon1)
    { bound[0] = yn;
      bound[1] = xn;
      bound[2] = xp;
      bound[3] = yp;
    }
  else
    { bound[0] = yn && xn;
      bound[1] = yn;
      bound[2] = yn && xp;
      bound[3] = xn;
      bound[4] = xp;
      bound[5] = yp && xn;
      bound[6] = yp;
      bound[7] = yp && xp;
    }

  return (bound);
}

/* boundary_pixels_3d(): set boundary pixel flags in 3D space.
 *
 * Args: p - pixel index.
 *
 * Return: an array of flags. 
 *         1 means it is inside the image and 0 means outside.
 */
static inline int *boundary_pixels_3d(int p)
{ static int bound[26];
  int x, xn, xp;
  int y, yn, yp;
  int z, zn, zp;

  z = p/carea;
  y = p%carea;
  x = y%cwidth;
  y = y/cwidth;

  yn = (y > 0);
  xn = (x > 0);
  zn = (z > 0);
  xp = (x < chk_width);
  yp = (y < chk_height);
  zp = (z < chk_depth);
  if (chk_iscon1)
    { bound[0] = zn;
      bound[1] = yn;
      bound[2] = xn;
      bound[3] = xp;
      bound[4] = yp;
      bound[5] = zp;
    }
  else
    { int t;

      bound[ 1] = t = zn && yn;
      bound[ 0] = t && xn;
      bound[ 2] = t && xp;
      bound[ 3] = zn && xn;
      bound[ 4] = zn;
      bound[ 5] = zn && xp;
      bound[ 7] = t = zn && yp;
      bound[ 6] = t && xn;
      bound[ 8] = t && xp;

      bound[ 9] = yn && xn;
      bound[10] = yn;
      bound[11] = yn && xp;
      bound[12] = xn;
      bound[13] = xp;
      bound[14] = yp && xn;
      bound[15] = yp;
      bound[16] = yp && xp;

      bound[18] = t = zp && yn;
      bound[17] = t && xn;
      bound[19] = t && xp;
      bound[20] = zp && xn;
      bound[21] = zp;
      bound[22] = zp && xp;
      bound[24] = t = zp && yp;
      bound[23] = t && xn;
      bound[25] = t && xp;
    }

  return (bound);
}

/* boundary_pixels(): a wrapper of boundary_pixels_2d and boundary_pixels_3d
 */
static inline int *boundary_pixels(int p)
{
  if ((chk_depth > 0) && (ccon_2d == 0))
    return boundary_pixels_3d(p);
  else
    return boundary_pixels_2d(p);
}

static int init_neighbor_2d(int neighbor[])
{
  int n_nbrs = 0;
  int x, y;
  if (chk_iscon1) {
    neighbor[0] = -cwidth;
    neighbor[1] = -1;
    neighbor[2] =  1;
    neighbor[3] =  cwidth;
    n_nbrs = 4;
  } else {
    for (y = -cwidth; y <= cwidth; y += cwidth)
      for (x = -1; x <= 1; x += 1)
	if (!(x == 0 && y == 0))
	  neighbor[n_nbrs++] = y + x;
  }

  return n_nbrs;
}

static int init_neighbor_3d(int neighbor[])
{
  int n_nbrs = 0;
  int x, y, z;
  if (chk_iscon1) {
      neighbor[0] = -carea;
      neighbor[1] = -cwidth;
      neighbor[2] = -1;
      neighbor[3] =  1;
      neighbor[4] =  cwidth;
      neighbor[5] =  carea;
      n_nbrs = 6;
  } else {
    for (z = -carea; z <= carea; z += carea)
      for (y = -cwidth; y <= cwidth; y += cwidth)
	for (x = -1; x <= 1; x += 1)
	  if (! (x==0 && y == 0 && z == 0))
	    neighbor[n_nbrs++] = z+y+x;
  }

  return n_nbrs;
}

/* init_neighbor(): initialize neighbor offsets.
 *
 * Args: neighbor - an uninitialized array;
 *
 * Return: number of neighbors.
 */
static int init_neighbor(int neighbor[])
{
  if ((chk_depth > 0) && (ccon_2d == 0)) {
    return init_neighbor_3d(neighbor);
  } else {
    return init_neighbor_2d(neighbor);
  }
}
/*
static int init_neighbor(int neighbor[])
{
  int n_nbrs = 0;
  int x, y, z;

  if (cdepth == 1) {
    if (chk_iscon1) {
      neighbor[0] = -cwidth;
      neighbor[1] = -1;
      neighbor[2] =  1;
      neighbor[3] =  cwidth;
      n_nbrs = 4;
    } else {
      for (y = -cwidth; y <= cwidth; y += cwidth)
	for (x = -1; x <= 1; x += 1)
	  if (!(x == 0 && y == 0))
	    neighbor[n_nbrs++] = y + x;
    }
  } else {
    if (chk_iscon1) {
      neighbor[0] = -carea;
      neighbor[1] = -cwidth;
      neighbor[2] = -1;
      neighbor[3] =  1;
      neighbor[4] =  cwidth;
      neighbor[5] =  carea;
      n_nbrs = 6;
    } else {
      for (z = -carea; z <= carea; z += carea)
	for (y = -cwidth; y <= cwidth; y += cwidth)
	  for (x = -1; x <= 1; x += 1)
	    if (! (x==0 && y == 0 && z == 0))
	      neighbor[n_nbrs++] = z+y+x;
    }
  }

  return n_nbrs;
}
*/

/* level_code(): build level code for an image or stack.
 *
 * Args: array - image or stack array;
 *       code - array to store results;
 *       link - array to store queues;
 *       width - image width;
 *       height - image height;
 *       seeds - starting pixels;
 *       seed - number of seeds;
 *       neighbor - array to store neighbor offset.
 *       max_level - the maximum of the level to reach. It has no effect if it
 *                   is 0.
 *
 * Return: void.
 */
static void level_code(const uint8 *array, uint16 *code, int *link, 
		       int width, int height, int depth, 
		       const int *seed, int nseed, int neighbor[],
		       int max_level)
{
  int c = -1;
  int prev = -1;
  int i;
  int n_nbrs = 0;
  int nb = -1;
  int *b = NULL;

  n_nbrs = init_neighbor(neighbor);

  for (i = 0; i < cnpixel; i++) {
    code[i] = 0;
    link[i] = -1;
  }

  int prev_seed =  -1;

  /*
  for (i = 0; i < nseed; ++i) {
    if (array[seed[i]] > 0) {
      prev_seed = seed[i];
      code[prev_seed] = 1;
      link[prev_seed] = -1;
      break;
    }
  }
*/
  for (i = 0; i < nseed; i++) {
    if (array[seed[i]] > 0) {
      code[seed[i]] = 1;
      link[seed[i]] = prev_seed;
      prev_seed = seed[i];
    }
  }

  int entrance = prev_seed;

  /* generate level field  */
  while (entrance > 0) {
    c = entrance;
    prev = -1;

    do {
      if ((max_level == 0) || (code[c] < max_level)) {
	b = boundary_pixels(c);      
	for (i = 0; i < n_nbrs; i++) {
	  if (b[i]) {
	    nb = c + neighbor[i];
	    if (array[nb] && !code[nb]) {
	      code[nb] = code[c] + 1;
	      link[nb] = prev;
	      prev = nb;
	    }
	  }
	}
      }
      c = link[c];
    } while (c >= 0);
    entrance = prev;
  }  
}

/* branch_code(): build branch code for an image or stack.
 * 
 * Args - array - image or stack array;
 *       code - array to store results;
 *       link - array to store queues;
 *       edge - edge information. The ith element is the father of the ith branch. It ends with 0;
 *       edge - array to store edges;
 *       width - image width;
 *       height - image height;
 *       seed - starting pixel;
 *       neighbor - array to store neighbor offset.
 *
 * Return: void.
 */
static void branch_code(const uint8 *array, uint16 *code, int *link, uint16 *edge,
		       int width, int height, int depth, 
		       int seed, int neighbor[])
{
  int current_branch = 0;
  int neighbor_entrance = -1; /* entrance of the neighbor queue */
  int iter = -1;
  int branch_tail = seed; /* tail of the branches*/
  int new_branch_head = -1;
  int new_branch_tail = -1;
  int branch_number = 1;
  int found_new_branch = FALSE;
  int n_nbrs = 0;
  int nb = -1;
  int entrance = seed;
  int c = -1;
  int prev = -1;
  int *b = NULL;
  int i;

  n_nbrs = init_neighbor(neighbor);

  for (i = 0; i < cnpixel; i++) {
    code[i] = 0;
    link[i] = -1;
  }
  code[seed] = 1;
  edge[1] = 0;

  while (entrance >= 0) { /* main loop */
    c = entrance;
    neighbor_entrance = -1;
    prev = -1;
    current_branch = code[c];

    do {  /* label neighbors of a branch and build a queue */
      b = boundary_pixels(c);      
      for (i = 0; i < n_nbrs; i++) {
	if (b[i]) {
	  nb = c + neighbor[i];
	  if (array[nb] && !code[nb]) {
	    code[nb] = NEIGHBOR_LABEL;
	    if (neighbor_entrance < 0) {
	      neighbor_entrance = nb;
	      prev = neighbor_entrance;
	      link[prev] = -1;
	    } else {
	      link[prev] = nb;
	      prev = nb;
	    }
	  }
	}
      }
      entrance = c;
      c = link[c];
    } while ((c >= 0) && (code[c] == current_branch));

    /* any neighbor found from the current branch? */
    if (neighbor_entrance < 0) { 
      entrance = link[entrance];
      continue;
    }
    
    c = neighbor_entrance;
    new_branch_head = c;
    new_branch_tail = c;
    code[c] = BRANCH_LABEL;

    found_new_branch = FALSE;

    do { /* grow branches from neighbors */
      if (code[c] != BRANCH_LABEL) { /* found new branch */
	edge[++branch_number] = current_branch;
	for (iter = new_branch_head; code[iter] == BRANCH_LABEL; 
	     iter = link[iter])
	  code[iter] = branch_number;
	code[c] = BRANCH_LABEL;
	link[branch_tail] = new_branch_head;
	new_branch_head = c;
	branch_tail = new_branch_tail;
	new_branch_tail = c;
	found_new_branch = TRUE;
      }
      
      b = boundary_pixels(c);

      for (i = 0; i < n_nbrs; i++) {
	if (b[i]) {
	  nb = c + neighbor[i];
	  if (code[nb] == NEIGHBOR_LABEL) {
	    if (link[new_branch_tail] != nb) { /* resort the queue */
	      iter = new_branch_tail;
	      while((link[iter] != -1) && (link[iter] != nb)) {
		iter = link[iter];
	      }
	      link[iter] = link[nb];
	      link[nb] = link[new_branch_tail];
	      link[new_branch_tail] = nb;
	    }
	    code[nb] = BRANCH_LABEL;
	    new_branch_tail = nb;
	  }
	}
      }
      c = link[c];
    } while (c >= 0); /* grow branches from neighbors */

    if (found_new_branch == TRUE) {
      edge[++branch_number] = current_branch;
      current_branch = branch_number;
    }

    for (iter = new_branch_head; code[iter] == BRANCH_LABEL; 
	 iter = link[iter])
      code[iter] = current_branch;

    link[branch_tail] = new_branch_head;
    branch_tail = new_branch_tail;

    entrance = link[entrance]; /* go to next branch */
  }
  /*************************/

  edge[branch_number+1] = 0;
}
